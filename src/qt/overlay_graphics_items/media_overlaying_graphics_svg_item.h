#pragma once

#include "MediaOverlayingGraphicsItem"
#include "qt/svg/GraphicsSvgItem"

class MediaOverlayingGraphicsSvgItem : public GraphicsSvgItem, public MediaOverlayingGraphicsTrait {
public:
    explicit MediaOverlayingGraphicsSvgItem(QGraphicsItem *parentItem = nullptr);
    explicit MediaOverlayingGraphicsSvgItem(OverlayGraphicsView *view, QGraphicsItem *parentItem = nullptr);
    explicit MediaOverlayingGraphicsSvgItem(const QString &filename, QGraphicsItem *parentItem = nullptr);
    explicit MediaOverlayingGraphicsSvgItem(
            OverlayGraphicsView *view, const QString &filename, QGraphicsItem *parentItem = nullptr);
};
