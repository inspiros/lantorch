#include "overlay_graphics_item.h"

#include <QGraphicsScene>

OverlayGraphicsTrait::OverlayGraphicsTrait(OverlayGraphicsView *view) : view_(view) {}

void OverlayGraphicsTrait::setView(OverlayGraphicsView *view) {
    view_ = view;
}

OverlayGraphicsView *OverlayGraphicsTrait::view() const {
    return view_;
}

QObject *OverlayGraphicsTrait::overlayed_object() const {
    return view_->parent();
}

OverlayGraphicsItem::OverlayGraphicsItem(QGraphicsItem *parent)
        : QGraphicsItem(parent), OverlayGraphicsTrait() {}

OverlayGraphicsItem::OverlayGraphicsItem(OverlayGraphicsView *view, QGraphicsItem *parent)
        : QGraphicsItem(parent), OverlayGraphicsTrait(view) {}
