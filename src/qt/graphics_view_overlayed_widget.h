#pragma once

#include <QPointer>

#include "OverlayGraphicsView"

class GraphicsViewOverlayedTrait {
protected:
    QPointer<OverlayGraphicsView> graphics_view_ = nullptr;

    void initOverlay(QWidget *self = nullptr);

    void initOverlay(QGraphicsScene *scene, QWidget *self = nullptr);

public:
    GraphicsViewOverlayedTrait() = default;

    virtual ~GraphicsViewOverlayedTrait();

    void initOpenGLViewport();

    [[nodiscard]] QGraphicsScene *scene() const;

    [[nodiscard]] QPointer<OverlayGraphicsView> overlay() const;

    [[nodiscard]] inline auto view() const {
        return overlay();
    }
};

class GraphicsViewOverlayedWidget : public QWidget, virtual public GraphicsViewOverlayedTrait {
Q_OBJECT
public:
    explicit GraphicsViewOverlayedWidget(QWidget *parent = nullptr);

    explicit GraphicsViewOverlayedWidget(QGraphicsScene *scene, QWidget *parent = nullptr);
};
