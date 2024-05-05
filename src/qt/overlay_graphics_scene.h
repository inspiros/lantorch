#pragma once

#include <QGraphicsScene>

class OverlayGraphicsScene : public QGraphicsScene {
public:
    explicit OverlayGraphicsScene(QObject *parent = nullptr);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};
