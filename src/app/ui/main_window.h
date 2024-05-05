#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QScopedPointer>

#include "../gst/gst_pipeline_manager.h"
#include "../dnn/yolo_inference_worker.h"

#include "video_widget.h"

class MainWindow : public QMainWindow {
Q_OBJECT
private:
    QPushButton *btn_enable_ai;
    VideoWidget *video_widget;

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
};
