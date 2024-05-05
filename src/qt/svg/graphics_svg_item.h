#pragma once

#include <QGraphicsSvgItem>
#include "SvgRenderer"

class GraphicsSvgItem : virtual public QGraphicsSvgItem {
    SvgRenderer *renderer_ = nullptr;
    QRectF render_rect_;

public:
    explicit GraphicsSvgItem(QGraphicsItem *parentItem = nullptr);

    explicit GraphicsSvgItem(const QString &filename, QGraphicsItem *parentItem = nullptr);

    void setSharedRenderer(SvgRenderer *renderer);

    [[nodiscard]] SvgRenderer *renderer() const;

    [[nodiscard]] QRectF renderRect() const;

    void setRenderRect(const QRectF &rect);

    [[nodiscard]] QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
};
