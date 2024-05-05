#include "overlayed_gst_video_widget.h"

OverlayedGstVideoWidget::OverlayedGstVideoWidget(QWidget *parent)
        : GstVideoWidget(parent) {
    overlay_ = new OverlayWidget(this);
}

OverlayWidget *OverlayedGstVideoWidget::overlay() const {
    return overlay_;
}
