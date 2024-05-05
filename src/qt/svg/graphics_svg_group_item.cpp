#include "graphics_svg_group_item.h"

GraphicsSvgGroupItem::GraphicsSvgGroupItem(QGraphicsItem *parentItem)
        : GraphicsSvgItem(parentItem), renderer_group_(new SvgRendererGroup) {
    QObject::connect(renderer_group_, &SvgRendererGroup::repaintNeeded, this, [this]() {
        update();
    }, Qt::DirectConnection);
}

GraphicsSvgGroupItem::GraphicsSvgGroupItem(std::initializer_list<QString> filenames, QGraphicsItem *parentItem)
        : GraphicsSvgGroupItem(parentItem) {
    for (const auto &filename: filenames) {
        renderer_group_->insertNew(0, filename);
        QObject::connect(renderer_group_, &SvgRendererGroup::repaintNeeded, this, [this]() {
            update();
        }, Qt::DirectConnection);
    }
}

GraphicsSvgGroupItem::GraphicsSvgGroupItem(
        std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs,
        QGraphicsItem *parentItem) : GraphicsSvgItem(parentItem) {
    renderer_group_ = new SvgRendererGroup(contents_pairs);
    QObject::connect(renderer_group_, &SvgRendererGroup::repaintNeeded, this, [this]() {
        update();
    }, Qt::DirectConnection);
}

SvgRendererGroup *GraphicsSvgGroupItem::rendererGroup() const {
    return renderer_group_;
}

void GraphicsSvgGroupItem::setSharedRendererGroup(SvgRendererGroup *group) {
    renderer_group_->disconnect(this);
    renderer_group_ = group;
    QObject::connect(renderer_group_, &SvgRendererGroup::repaintNeeded, this, [this]() {
        update();
    }, Qt::DirectConnection);
    update();
}

QRectF GraphicsSvgGroupItem::boundingRect() const {
    if (!renderRect().isNull())
        return renderRect();
    if (!renderer_group_)
        return {0, 0, 0, 0};
    auto default_size = renderer_group_->defaultSize();
    return {0, 0,
            static_cast<qreal>(default_size.width()),
            static_cast<qreal>(default_size.height())};
}

void GraphicsSvgGroupItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    renderer_group_->render(painter, boundingRect());
}
