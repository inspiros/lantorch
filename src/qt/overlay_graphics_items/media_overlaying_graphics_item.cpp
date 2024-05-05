#include "media_overlaying_graphics_item.h"

#include <QGraphicsScene>

MediaOverlayingGraphicsTrait::MediaOverlayingGraphicsTrait(OverlayGraphicsView *view)
        : OverlayGraphicsTrait(view) {}

MediaWidget *MediaOverlayingGraphicsTrait::overlayed_media_widget() const {
    return qobject_cast<MediaWidget *>(overlayed_object());
}

double MediaOverlayingGraphicsTrait::pop_scale(QTransform &transform) const {
    auto dx = transform.m13(), dy = transform.m23();
    transform.translate(-dx, -dy);
    auto render_scale = overlayed_media_widget()->render_scale();
    auto s = 1 / std::min(render_scale.width(), render_scale.height());
    transform.scale(s, s);
    transform.translate(dx, dy);
    return s;
}

MediaOverlayingGraphicsItem::MediaOverlayingGraphicsItem(QGraphicsItem *parent)
        : QGraphicsItem(parent), MediaOverlayingGraphicsTrait() {}

MediaOverlayingGraphicsItem::MediaOverlayingGraphicsItem(OverlayGraphicsView *view, QGraphicsItem *parent)
        : QGraphicsItem(parent), MediaOverlayingGraphicsTrait(view) {}
