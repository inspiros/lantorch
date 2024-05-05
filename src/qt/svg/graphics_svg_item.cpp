#include "graphics_svg_item.h"

#include <QPainter>

GraphicsSvgItem::GraphicsSvgItem(QGraphicsItem *parentItem)
    : QGraphicsSvgItem(parentItem), renderer_(new SvgRenderer) {
    QObject::connect(renderer_, &SvgRenderer::repaintNeeded, this, [this]() {
        update();
    }, Qt::DirectConnection);
}

GraphicsSvgItem::GraphicsSvgItem(const QString &filename, QGraphicsItem *parentItem)
        : QGraphicsSvgItem(parentItem) {
    renderer_ = new SvgRenderer(filename);
    QObject::connect(renderer_, &SvgRenderer::repaintNeeded, this, [this]() {
        update();
    }, Qt::DirectConnection);
}

void GraphicsSvgItem::setSharedRenderer(SvgRenderer *renderer) {
    renderer_->disconnect(this);
    renderer_ = renderer;
    QObject::connect(renderer_, &SvgRenderer::repaintNeeded, this, [this]() {
        update();
    }, Qt::DirectConnection);
    update();
}

SvgRenderer *GraphicsSvgItem::renderer() const {
    return renderer_;
}

QRectF GraphicsSvgItem::renderRect() const {
    return render_rect_;
}

void GraphicsSvgItem::setRenderRect(const QRectF &rect) {
    render_rect_ = rect;
    update();
}

QRectF GraphicsSvgItem::boundingRect() const {
    if (!render_rect_.isNull())
        return render_rect_;
    if (!renderer_)
        return {0, 0, 0, 0};
    auto default_size = renderer_->defaultSize();
    return {0, 0,
            static_cast<qreal>(default_size.width()),
            static_cast<qreal>(default_size.height())};
}

void GraphicsSvgItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    renderer_->render(painter, boundingRect());
}
