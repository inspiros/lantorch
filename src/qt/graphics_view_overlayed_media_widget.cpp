#include "graphics_view_overlayed_media_widget.h"

#include <QResizeEvent>

void GraphicsViewOverlayedMediaTrait::initGraphicsItemFactory(OverlayGraphicsView *view, QObject *parent) {
    graphics_item_factory_ = new MediaOverlayingGraphicsItemFactory(view, parent);
}

GraphicsViewOverlayedMediaTrait::~GraphicsViewOverlayedMediaTrait() {
    if (!graphics_item_factory_.isNull())
        delete graphics_item_factory_;
}

QPointer<MediaOverlayingGraphicsItemFactory> GraphicsViewOverlayedMediaTrait::graphics_item_factory() const {
    return graphics_item_factory_;
}

GraphicsViewOverlayedMediaWidget::GraphicsViewOverlayedMediaWidget(QWidget *parent)
        : MediaWidget(parent) {
    initOverlay(this);
    initGraphicsItemFactory(view(), this);
}

GraphicsViewOverlayedMediaWidget::GraphicsViewOverlayedMediaWidget(
        QGraphicsScene *scene, QWidget *parent)
        : MediaWidget(parent) {
    initOverlay(scene, this);
    initGraphicsItemFactory(view(), this);
}

void GraphicsViewOverlayedMediaWidget::on_media_geometry_changed(
        QRectF frame_rect, QRect render_rect, QSizeF render_scale) {
    view()->resetTransform();
    if (!frame_size().isEmpty()) {
        view()->setSceneRect(-frame_rect.x(), -frame_rect.y(),
                             width() / render_scale.width(),
                             height() / render_scale.height());
        view()->scale(render_scale.width(), render_scale.height());
    } else {
        view()->setSceneRect({0, 0,
                              static_cast<qreal>(width()),
                              static_cast<qreal>(height())});
    }
}
