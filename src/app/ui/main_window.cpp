#include "main_window.h"

#include <fmt/chrono.h>

#undef slots

#include <torch/cuda.h>

#define slots Q_SLOTS

#include <QApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTimer>

#include "../app_config.h"
#include "../macros.h"

#include <QDebug>

static const at::Device fallback_device = torch::cuda::is_available() ? at::kCUDA : at::kCPU;
static const at::ScalarType fallback_dtype = at::kFloat;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto configs = AppConfig::instance();
    auto window_configs = configs["app"]["ui"]["main_window"];

    /* UI */
    if (window_configs["window_name"].IsDefined())
        setWindowTitle(window_configs["window_name"].as<QString>());
    if (window_configs["window_logo_filepath"].IsDefined())
        setWindowIcon(QIcon(QString::fromStdString(
                window_configs["window_logo_filepath"].as<AppConfig::crel_path>().string())));

    toggle_ai_btn = new QPushButton("Enable AI", this);
    toggle_ai_btn->setCheckable(true);
    toggle_ai_btn->setChecked(true);

    // options
    score_thresh_slider = new QSlider(Qt::Horizontal, this);
    score_thresh_slider->setRange(0, 100);
    score_thresh_indicator = new QLabel(this);
    score_thresh_indicator->setProperty("template", "score_threshold: %1");

    nms_thresh_slider = new QSlider(Qt::Horizontal, this);
    nms_thresh_slider->setRange(0, 100);
    nms_thresh_indicator = new QLabel(this);
    nms_thresh_indicator->setProperty("template", "nms_threshold: %1");

    dtype_label = new QLabel("dtype:", this);
    dtype_cbb = new QComboBox(this);
    dtype_cbb->addItem(YAML::convert<at::ScalarType>::encode(at::kDouble).Scalar().c_str());
    dtype_cbb->addItem(YAML::convert<at::ScalarType>::encode(at::kFloat).Scalar().c_str());
    dtype_cbb->addItem(YAML::convert<at::ScalarType>::encode(at::kHalf).Scalar().c_str());
    dtype_cbb->addItem(YAML::convert<at::ScalarType>::encode(at::kBFloat16).Scalar().c_str());

    device_label = new QLabel("device:", this);
    device_cbb = new QComboBox(this);
    device_cbb->addItem(YAML::convert<at::Device>::encode(at::kCPU).Scalar().c_str());
    for (int8_t device_id = 0; device_id < torch::cuda::device_count(); device_id++)
        device_cbb->addItem(
                YAML::convert<at::Device>::encode(at::Device(at::DeviceType::CUDA, device_id)).Scalar().c_str());
    // TODO: remove these lines if not using YOLOv8
    device_cbb->setEnabled(false);
    device_cbb->setToolTip("Yolov8 exported model is device-fixated");

    // video
    video_widget = new VideoWidget(this);
    if (window_configs["video_widget"]["use_opengl_paint_engine"].as<bool>(false))
        video_widget->initOpenGLViewport();
    video_widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    video_widget->setMinimumSize(100, 100);

    time_indicator = new QLabel(this);
    time_indicator->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    time_indicator->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    auto *main_widget = this->centralWidget() ?: new QWidget(this);
    auto *main_layout = new QVBoxLayout(main_widget);
    main_widget->setLayout(main_layout);

    auto *options_layout = new QGridLayout;
    options_layout->addWidget(score_thresh_indicator, 0, 0, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
    options_layout->addWidget(score_thresh_slider, 0, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    options_layout->addWidget(nms_thresh_indicator, 1, 0, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
    options_layout->addWidget(nms_thresh_slider, 1, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    options_layout->addWidget(dtype_label, 0, 2, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
    options_layout->addWidget(dtype_cbb, 0, 3, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    options_layout->addWidget(device_label, 1, 2, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
    options_layout->addWidget(device_cbb, 1, 3, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    options_layout->setRowStretch(0, 0);
    options_layout->setRowStretch(1, 0);
    options_layout->setColumnStretch(0, 1);
    options_layout->setColumnStretch(1, 1);
    options_layout->setColumnStretch(2, 1);
    options_layout->setColumnStretch(3, 1);

    auto *control_layout = new QVBoxLayout;
    control_layout->addWidget(toggle_ai_btn);
    control_layout->addLayout(options_layout);

    main_layout->addLayout(control_layout);
    main_layout->addWidget(video_widget);
    main_layout->addWidget(time_indicator);
    setCentralWidget(main_widget);

    /* Pipeline */
    auto yolo_infer_config = configs["app"]["dnn"]["yolo_infer"];
    yolo_infer_thread.reset(new GstInferenceQThread(this));
    auto yolo_infer_worker = yolo_infer_thread->init_worker<YoloInferenceWorker>(
            nullptr,
            yolo_infer_config["model_filepath"].as<AppConfig::crel_path>().string(),
            yolo_infer_config["classes_filepath"].IsDefined()
            ? yolo_infer_config["classes_filepath"].as<AppConfig::crel_path>().string() : "",
            ultralytics::YoloOptions()
                    .input_shape(yolo_infer_config["yolo_options"]["input_shape"].as<cv::Size>(
                            DEFAULT_PARAM(ultralytics::YoloOptions, input_shape)))
                    .confidence_threshold(yolo_infer_config["yolo_options"]["confidence_threshold"].as<float>(
                            DEFAULT_PARAM(ultralytics::YoloOptions, confidence_threshold)))
                    .score_threshold(yolo_infer_config["yolo_options"]["score_threshold"].as<float>(
                            DEFAULT_PARAM(ultralytics::YoloOptions, score_threshold)))
                    .nms_threshold(yolo_infer_config["yolo_options"]["nms_threshold"].as<float>(
                            DEFAULT_PARAM(ultralytics::YoloOptions, nms_threshold)))
                    .align_center(yolo_infer_config["yolo_options"]["align_center"].as<bool>(
                            DEFAULT_PARAM(ultralytics::YoloOptions, align_center))),
            yolo_infer_config["device"].as<at::Device>(fallback_device),
            yolo_infer_config["dtype"].as<at::ScalarType>(fallback_dtype),
            yolo_infer_config["verbose"].as<bool>(false));
    yolo_infer_thread->start(yolo_infer_config["priority"].as<QThread::Priority>(QThread::NormalPriority));

    /* Signals */
    // options
    QObject::connect(score_thresh_slider, &QSlider::valueChanged, this, [this](int conf) {
        auto new_conf_thresh = (float) conf / 100.0f;
        score_thresh_indicator->setText(
                score_thresh_indicator->property("template").toString().arg(new_conf_thresh));
        auto worker = yolo_infer_thread->worker<YoloInferenceWorker>();
        worker->update_options_later({}, {}, worker->options().score_threshold(new_conf_thresh));
    });
    QObject::connect(nms_thresh_slider, &QSlider::valueChanged, this, [this](int nms) {
        auto new_nms_thresh = (float) nms / 100.0f;
        nms_thresh_indicator->setText(
                nms_thresh_indicator->property("template").toString().arg(new_nms_thresh));

        auto worker = yolo_infer_thread->worker<YoloInferenceWorker>();
        worker->update_options_later({}, {}, worker->options().nms_threshold(new_nms_thresh));
    });
    QObject::connect(dtype_cbb, &QComboBox::currentTextChanged, this, [this](const QString &dtype_string) {
        // create a node with dtype string and decode
        YAML::Node node;
        call_private::Assign(node, dtype_string.toStdString().c_str());
        at::ScalarType new_dtype;
        YAML::convert<at::ScalarType>::decode(node, new_dtype);

        auto worker = yolo_infer_thread->worker<YoloInferenceWorker>();
        worker->update_options_later({}, new_dtype, {});
    });
    QObject::connect(device_cbb, &QComboBox::currentTextChanged, this, [this](const QString &device_string) {
        at::Device new_device(device_string.toStdString());
        auto worker = yolo_infer_thread->worker<YoloInferenceWorker>();
        worker->update_options_later(new_device, {}, {});
    });

    QObject::connect(video_widget, &VideoWidget::frame_pts_changed, this, [this](GstClockTime pts) {
        auto rounded_pts = std::chrono::floor<std::chrono::milliseconds>(std::chrono::nanoseconds(pts));
        time_indicator->setText(QString::fromStdString(fmt::format("{:%H:%M:%S}", rounded_pts)));
    });

    QObject::connect(  // yolo detector
            yolo_infer_worker.data(),
            &YoloInferenceWorker::new_result,
            this,
            [this](unsigned long frame_id, const std::vector<Detection> &dets) {
                video_widget->request_bboxes_from_pool(dets);
            }, Qt::BlockingQueuedConnection);

    QObject::connect(toggle_ai_btn, &QPushButton::clicked, this, [this](bool checked = false) {
        yolo_infer_thread->pause(!checked);
        QTimer::singleShot(200, this, [this]() {
            video_widget->request_bboxes_from_pool({});
        });
    });

    /* Set UI values */
    score_thresh_slider->setValue((int) std::round(yolo_infer_config["yolo_options"]["score_threshold"].as<float>(
            DEFAULT_PARAM(ultralytics::YoloOptions, score_threshold)) * 100));
    nms_thresh_slider->setValue((int) std::round(yolo_infer_config["yolo_options"]["nms_threshold"].as<float>(
            DEFAULT_PARAM(ultralytics::YoloOptions, nms_threshold)) * 100));
    dtype_cbb->setCurrentText(
            YAML::convert<at::ScalarType>::encode(
                    yolo_infer_config["dtype"].as<at::ScalarType>(fallback_dtype)).Scalar().c_str());
    device_cbb->setCurrentText(
            YAML::convert<at::Device>::encode(
                    yolo_infer_config["device"].as<at::Device>(fallback_device)).Scalar().c_str());
}

MainWindow::~MainWindow() {
    auto yolo_infer_worker = yolo_infer_thread->worker<YoloInferenceWorker>();
    if (!yolo_infer_worker.isNull())
        yolo_infer_worker.data()->disconnect(this);
}

void MainWindow::reset() {
    resetUI();
    resetPipeline();
}

void MainWindow::resetUI() {
    resize(500, 400);
}

void MainWindow::resetPipeline() {
    pauseInferenceThreads(true);

    pipeline.reset(new GstPipelineManager);
    video_widget->set_qwidget5videosink(
            pipeline->get_element("display_sink") ?:
            pipeline->get_element_by_factory_name("qwidget5videosink"));
    pipeline->add_inference_bin("yolo_infer");

    if (!yolo_infer_thread.isNull())
        yolo_infer_thread->worker<YoloInferenceWorker>()->set_app_sink(
                pipeline->get_element("yolo_infer_sink"));

    setPipelineState(GST_STATE_PLAYING);
    pauseInferenceThreads(false);
}

void MainWindow::setPipelineState(GstState state) {
    if (!pipeline.isNull() && !pipeline->set_state(state)) {
        QMessageBox::critical(this, "Error", "Unable to play, please check main_pipeline description.");
        QTimer::singleShot(0, this, SLOT(close()));
    }
}

void MainWindow::pauseInferenceThreads(bool mode) const {
    if (!yolo_infer_thread.isNull())
        yolo_infer_thread->pause(mode ? mode : !toggle_ai_btn->isChecked());
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Q:
            if (event->modifiers() != Qt::ControlModifier)
                break;
        case Qt::Key_Escape:
            QTimer::singleShot(0, this, SLOT(close()));
            break;
        case Qt::Key_F11:
            isFullScreen() ? showNormal() : showFullScreen();
            break;
    }
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    reset();
}
