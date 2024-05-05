#include "graphics_view_overlayed_widget.h"

#include <QOpenGLWidget>
#include <QScrollBar>

QGraphicsScene *GraphicsViewOverlayedTrait::scene() const {
    return graphics_view_->scene();
}

QPointer<OverlayGraphicsView> GraphicsViewOverlayedTrait::overlay() const {
    return graphics_view_;
}

void GraphicsViewOverlayedTrait::initOverlay(QWidget *self) {
    graphics_view_ = new OverlayGraphicsView(self);
}

void GraphicsViewOverlayedTrait::initOverlay(QGraphicsScene *scene, QWidget *self) {
    graphics_view_ = new OverlayGraphicsView(scene, self);
}

void GraphicsViewOverlayedTrait::initOpenGLViewport() {
    if (!graphics_view_.isNull() && !qobject_cast<QOpenGLWidget *>(graphics_view_->viewport())) {
        auto *viewport = new QOpenGLWidget(graphics_view_);
        viewport->setAttribute(Qt::WA_AlwaysStackOnTop);
        graphics_view_->setViewport(viewport);
    }
}

GraphicsViewOverlayedTrait::~GraphicsViewOverlayedTrait() {
    if (!graphics_view_.isNull())
        delete graphics_view_;
}

GraphicsViewOverlayedWidget::GraphicsViewOverlayedWidget(QWidget *parent)
        : QWidget(parent) {
    initOverlay(this);
}

GraphicsViewOverlayedWidget::GraphicsViewOverlayedWidget(QGraphicsScene *scene, QWidget *parent)
        : QWidget(parent) {
    initOverlay(scene, this);
}
