#include "media_overlaying_graphics_item_factory.h"

MediaOverlayingGraphicsItemFactory::MediaOverlayingGraphicsItemFactory(OverlayGraphicsView *view, QObject *parent)
        : QObject(parent), view_(view) {}

MediaOverlayingGraphicsItemFactory::~MediaOverlayingGraphicsItemFactory() {
    clear();
}

void MediaOverlayingGraphicsItemFactory::setView(OverlayGraphicsView *view) {
    view_ = view;
}

OverlayGraphicsView *MediaOverlayingGraphicsItemFactory::view() const {
    return view_;
}

MediaWidget *MediaOverlayingGraphicsItemFactory::overlayed_media_widget() const {
    return qobject_cast<MediaWidget *>(view_->parent());
}

void MediaOverlayingGraphicsItemFactory::addItem(const QSharedPointer<MediaOverlayingGraphicsItem> &item) {
    items_.insert(item.data(), item);
}

void MediaOverlayingGraphicsItemFactory::addItem(MediaOverlayingGraphicsItem *item) {
    items_.insert(item, QSharedPointer<MediaOverlayingGraphicsItem>(item));
}

int MediaOverlayingGraphicsItemFactory::removeItem(const QSharedPointer<MediaOverlayingGraphicsItem> &item) {
    return items_.remove(item.data());
}

int MediaOverlayingGraphicsItemFactory::removeItem(MediaOverlayingGraphicsItem *item) {
    return items_.remove(item);
}

void MediaOverlayingGraphicsItemFactory::clear() {
    items_.clear();
}
