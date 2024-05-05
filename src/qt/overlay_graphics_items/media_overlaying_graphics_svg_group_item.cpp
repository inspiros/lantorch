#include "media_overlaying_graphics_svg_group_item.h"

MediaOverlayingGraphicsSvgGroupItem::MediaOverlayingGraphicsSvgGroupItem(QGraphicsItem *parentItem)
        : GraphicsSvgGroupItem(parentItem), MediaOverlayingGraphicsTrait() {}

MediaOverlayingGraphicsSvgGroupItem::MediaOverlayingGraphicsSvgGroupItem(OverlayGraphicsView *view,
                                                                         QGraphicsItem *parentItem)
        : GraphicsSvgGroupItem(parentItem), MediaOverlayingGraphicsTrait(view) {}

MediaOverlayingGraphicsSvgGroupItem::MediaOverlayingGraphicsSvgGroupItem(std::initializer_list<QString> filenames,
                                                                         QGraphicsItem *parentItem)
        : GraphicsSvgGroupItem(filenames, parentItem), MediaOverlayingGraphicsTrait() {}

MediaOverlayingGraphicsSvgGroupItem::MediaOverlayingGraphicsSvgGroupItem(
        OverlayGraphicsView *view, std::initializer_list<QString> filenames, QGraphicsItem *parentItem)
        : GraphicsSvgGroupItem(filenames, parentItem), MediaOverlayingGraphicsTrait(view) {}
