#pragma once

#include "GraphicsSvgItem"
#include "SvgRendererGroup"

class GraphicsSvgGroupItem : public GraphicsSvgItem {
    SvgRendererGroup *renderer_group_ = nullptr;

public:
    GraphicsSvgGroupItem(QGraphicsItem *parentItem = nullptr);

    GraphicsSvgGroupItem(std::initializer_list<QString> filenames, QGraphicsItem *parentItem = nullptr);

    GraphicsSvgGroupItem(
            std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs,
            QGraphicsItem *parentItem = nullptr);

    [[nodiscard]] SvgRendererGroup *rendererGroup() const;

    void setSharedRendererGroup(SvgRendererGroup *group);

    [[nodiscard]] QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    SvgRenderer *renderer() const;

    void setSharedRenderer(SvgRenderer *renderer);
};
