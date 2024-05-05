#include "graphics_view_overlayed_gst_video_widget.h"

#include <QResizeEvent>

GraphicsViewOverlayedGstVideoWidget::GraphicsViewOverlayedGstVideoWidget(
        QWidget *parent)
        : GstVideoWidget(parent) {
    initOverlay(this);
    initGraphicsItemFactory(view(), this);
}

GraphicsViewOverlayedGstVideoWidget::GraphicsViewOverlayedGstVideoWidget(
        QGraphicsScene *scene, QWidget *parent)
        : GstVideoWidget(parent) {
    initOverlay(scene, this);
    initGraphicsItemFactory(view(), this);
}

void GraphicsViewOverlayedGstVideoWidget::on_media_geometry_changed(
        QRectF frame_rect, QRect render_rect, QSizeF render_scale) {
    view()->resetTransform();
    if (!frame_size().isEmpty()) {
        view()->setSceneRect(-frame_rect.x(), -frame_rect.y(),
                             width() / render_scale.width(),
                             height() / render_scale.height());
        view()->scale(render_scale.width(), render_scale.height());
    } else {
        view()->setSceneRect({0, 0,
                              static_cast<qreal>(width()),
                              static_cast<qreal>(height())});
    }
}
