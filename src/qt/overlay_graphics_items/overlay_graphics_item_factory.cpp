#include "overlay_graphics_item_factory.h"

OverlayGraphicsItemFactory::OverlayGraphicsItemFactory(OverlayGraphicsView *view, QObject *parent)
        : QObject(parent), view_(view) {}

OverlayGraphicsItemFactory::~OverlayGraphicsItemFactory() {
    clear();
}

void OverlayGraphicsItemFactory::setView(OverlayGraphicsView *view) {
    view_ = view;
}

OverlayGraphicsView *OverlayGraphicsItemFactory::view() const {
    return view_;
}

QObject *OverlayGraphicsItemFactory::overlayed_object() const {
    return view_->parent();
}

void OverlayGraphicsItemFactory::addItem(const QSharedPointer<OverlayGraphicsItem> &item) {
    items_.insert(item.data(), item);
}

void OverlayGraphicsItemFactory::addItem(OverlayGraphicsItem *item) {
    items_.insert(item, QSharedPointer<OverlayGraphicsItem>(item));
}

int OverlayGraphicsItemFactory::removeItem(const QSharedPointer<OverlayGraphicsItem> &item) {
    return items_.remove(item.data());
}

int OverlayGraphicsItemFactory::removeItem(OverlayGraphicsItem *item) {
    return items_.remove(item);
}

void OverlayGraphicsItemFactory::clear() {
    items_.clear();
}
