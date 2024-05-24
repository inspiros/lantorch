#pragma once

#include <QComboBox>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QScopedPointer>

#include "../gst/gst_pipeline_manager.h"
#include "../dnn/yolo_inference_worker.h"

#include "video_widget.h"

class MainWindow : public QMainWindow {
Q_OBJECT
    QPushButton *toggle_ai_btn;
    QLabel *score_thresh_indicator;
    QLabel *nms_thresh_indicator;
    QSlider *score_thresh_slider;
    QSlider *nms_thresh_slider;
    QLabel *dtype_label;
    QLabel *device_label;
    QComboBox *dtype_cbb;
    QComboBox *device_cbb;

    VideoWidget *video_widget;
    QLabel *time_indicator;

    QScopedPointer<GstPipelineManager> pipeline;
    QScopedPointer<GstInferenceQThread> yolo_infer_thread;

    ColorPalette bbox_color_palette;

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

public slots:
    void reset();

    void resetUI();

    void resetPipeline();

    void setPipelineState(GstState state);

    void pauseInferenceThreads(bool mode = true) const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void showEvent(QShowEvent *event) override;
};
