#pragma once

#include <QGraphicsView>

class OverlayGraphicsView : public QGraphicsView {
Q_OBJECT
public:
    explicit OverlayGraphicsView(QWidget *parent = nullptr);

    explicit OverlayGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);

    [[nodiscard]] QObject *overlayed_widget() const;

    void update();

private:
    void newParent();

protected:
    void wheelEvent(QWheelEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

    bool event(QEvent *event) override;
};
