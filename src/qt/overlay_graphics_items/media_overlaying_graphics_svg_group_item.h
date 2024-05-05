#pragma once

#include "MediaOverlayingGraphicsItem"
#include "qt/svg/GraphicsSvgGroupItem"

class MediaOverlayingGraphicsSvgGroupItem : public GraphicsSvgGroupItem, public MediaOverlayingGraphicsTrait {
public:
    explicit MediaOverlayingGraphicsSvgGroupItem(QGraphicsItem *parentItem = nullptr);
    explicit MediaOverlayingGraphicsSvgGroupItem(OverlayGraphicsView *view, QGraphicsItem *parentItem = nullptr);
    MediaOverlayingGraphicsSvgGroupItem(
            std::initializer_list<QString> filenames, QGraphicsItem *parentItem = nullptr);
    MediaOverlayingGraphicsSvgGroupItem(
            OverlayGraphicsView *view, std::initializer_list<QString> filenames, QGraphicsItem *parentItem = nullptr);
};
