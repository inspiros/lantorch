#include "overlay_graphics_scene.h"

#include <QOpenGLFramebufferObject>

#include <QPaintEngine>

OverlayGraphicsScene::OverlayGraphicsScene(QObject *parent) : QGraphicsScene(parent) {}

void OverlayGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect) {
    QGraphicsScene::drawBackground(painter, rect);
    if (painter->paintEngine()->type() == QPaintEngine::OpenGL ||
        painter->paintEngine()->type() == QPaintEngine::OpenGL2)
        glClear(GL_COLOR_BUFFER_BIT);
}
