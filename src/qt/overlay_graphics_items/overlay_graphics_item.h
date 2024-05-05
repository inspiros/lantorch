#pragma once

#include <QGraphicsItem>

#include "../OverlayGraphicsView"

class OverlayGraphicsTrait {
protected:
    OverlayGraphicsView *view_ = nullptr;

public:
    explicit OverlayGraphicsTrait() = default;

    explicit OverlayGraphicsTrait(OverlayGraphicsView *view);

    virtual ~OverlayGraphicsTrait() = default;

    void setView(OverlayGraphicsView *view);

    [[nodiscard]] OverlayGraphicsView *view() const;

    [[nodiscard]] QObject *overlayed_object() const;
};

class OverlayGraphicsItem : public QGraphicsItem, public OverlayGraphicsTrait {
public:
    explicit OverlayGraphicsItem(QGraphicsItem *parent = nullptr);

    explicit OverlayGraphicsItem(OverlayGraphicsView *view, QGraphicsItem *parent = nullptr);
};
