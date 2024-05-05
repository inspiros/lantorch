#pragma once

#include "MediaWidget"

#include <gst/gst.h>

class GstVideoWidget : public MediaWidget {
Q_OBJECT
    GstElement *sink_;
    gulong frame_size_probe_id_ = 0;

public:
    explicit GstVideoWidget(QWidget *parent = nullptr);

    ~GstVideoWidget() override;

    void set_qwidget5videosink(GstElement *sink);

    void set_overlay_videosink(GstElement *sink);

    void remove_sink();

    [[nodiscard]] GstElement *sink() const;

    bool has_sink() const;

signals:
    void sink_changed(GstElement *sink);

private:
    void add_frame_size_probe();

    void remove_frame_size_probe();

    void update_force_aspect_ratio();
};
