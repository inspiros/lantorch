#pragma once

#include "OverlayGraphicsItem"
#include "../MediaWidget"

class MediaOverlayingGraphicsTrait : public OverlayGraphicsTrait {
public:
    explicit MediaOverlayingGraphicsTrait() = default;

    explicit MediaOverlayingGraphicsTrait(OverlayGraphicsView *view);

    [[nodiscard]] MediaWidget *overlayed_media_widget() const;

protected:
    double pop_scale(QTransform &transform) const;
};

class MediaOverlayingGraphicsItem : public QGraphicsItem, public MediaOverlayingGraphicsTrait {
public:
    explicit MediaOverlayingGraphicsItem(QGraphicsItem *parent = nullptr);

    explicit MediaOverlayingGraphicsItem(OverlayGraphicsView *view, QGraphicsItem *parent = nullptr);
};
