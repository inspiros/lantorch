#pragma once

#include "MediaWidget"
#include "gst_types_registration.h"

#include <array>

#include <gst/gst.h>

class GstVideoWidget : public MediaWidget {
Q_OBJECT
    GstElement *sink_;
    std::array<gulong, 2> probes_ids_ = {0, 0};

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

    void frame_pts_changed(GstClockTime pts);

private:
    void add_probes();

    void remove_probes();

    void update_force_aspect_ratio();
};
