#include "media_overlaying_graphics_svg_item.h"

MediaOverlayingGraphicsSvgItem::MediaOverlayingGraphicsSvgItem(QGraphicsItem *parentItem)
    : GraphicsSvgItem(parentItem), MediaOverlayingGraphicsTrait() {}

MediaOverlayingGraphicsSvgItem::MediaOverlayingGraphicsSvgItem(OverlayGraphicsView *view, QGraphicsItem *parentItem)
        : GraphicsSvgItem(parentItem), MediaOverlayingGraphicsTrait(view) {}

MediaOverlayingGraphicsSvgItem::MediaOverlayingGraphicsSvgItem(const QString &filename, QGraphicsItem *parentItem)
        : GraphicsSvgItem(filename, parentItem), MediaOverlayingGraphicsTrait() {}

MediaOverlayingGraphicsSvgItem::MediaOverlayingGraphicsSvgItem(
        OverlayGraphicsView *view, const QString &filename, QGraphicsItem *parentItem)
       : GraphicsSvgItem(filename, parentItem), MediaOverlayingGraphicsTrait(view) {}
