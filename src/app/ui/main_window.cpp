#include "main_window.h"

#include <fmt/core.h>

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    auto configs = AppConfig::instance();
    auto window_configs = configs["app"]["ui"]["main_window"];

    /* UI */
    if (window_configs["window_name"].IsDefined())
        setWindowTitle(window_configs["window_name"].as<QString>());
    if (window_configs["window_logo_filepath"].IsDefined())
        setWindowIcon(QIcon(QString::fromStdString(
                window_configs["window_logo_filepath"].as<AppConfig::crel_path>().string())));

    btn_enable_ai = new QPushButton("Enable AI", this);
    btn_enable_ai->setCheckable(true);
    btn_enable_ai->setChecked(true);

    video_widget = new VideoWidget(this);
    if (window_configs["video_widget"]["use_opengl_paint_engine"].as<bool>(false))
        video_widget->initOpenGLViewport();
    video_widget->setMinimumSize(100, 100);

    auto *main_widget = this->centralWidget() ?: new QWidget(this);
    auto *main_layout = new QVBoxLayout(main_widget);
    main_widget->setLayout(main_layout);
    main_layout->addWidget(btn_enable_ai);
    main_layout->addWidget(video_widget);
    setCentralWidget(main_widget);

    /* Pipeline */
    at::Device fallback_device = torch::cuda::is_available() ? at::kCUDA : at::kCPU;

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
            yolo_infer_config["dtype"].as<at::ScalarType>(at::kFloat),
            yolo_infer_config["verbose"].as<bool>(false));
    yolo_infer_thread->start(yolo_infer_config["priority"].as<QThread::Priority>(QThread::NormalPriority));

    /* Signals */
    QObject::connect(  // yolo detector
            yolo_infer_worker.data(),
            &YoloInferenceWorker::new_result,
            this,
            [this](unsigned int frame_id, const std::vector<Detection> &dets) {
                video_widget->request_bboxes_from_pool(dets);
            }, Qt::BlockingQueuedConnection);

    QObject::connect(btn_enable_ai, &QPushButton::clicked, this, [this](bool checked = false) {
        yolo_infer_thread->pause(!checked);
        QTimer::singleShot(200, this, [this]() {
            video_widget->request_bboxes_from_pool({});
        });
    });
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
        yolo_infer_thread->pause(mode ? mode : !btn_enable_ai->isChecked());
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Q:
            if (event->modifiers() == Qt::ControlModifier)
                QTimer::singleShot(0, this, SLOT(close()));
            break;
        case Qt::Key_F11:
            isFullScreen() ? showNormal() : showFullScreen();
            break;
        case Qt::Key_Escape:
            if (isFullScreen())
                showNormal();
            break;
    }
}
