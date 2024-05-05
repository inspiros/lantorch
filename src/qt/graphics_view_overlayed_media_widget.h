#pragma once

#include <QPointer>

#include "MediaWidget"
#include "GraphicsViewOverlayedWidget"

#include "overlay_graphics_items/MediaOverlayingGraphicsItemFactory"

class GraphicsViewOverlayedMediaTrait : public virtual GraphicsViewOverlayedTrait {
protected:
    QPointer<MediaOverlayingGraphicsItemFactory> graphics_item_factory_ = nullptr;

    void initGraphicsItemFactory(OverlayGraphicsView *view, QObject *parent = nullptr);

public:
    ~GraphicsViewOverlayedMediaTrait() override;

    [[nodiscard]] QPointer<MediaOverlayingGraphicsItemFactory> graphics_item_factory() const;
};

class GraphicsViewOverlayedMediaWidget : public MediaWidget, virtual public GraphicsViewOverlayedMediaTrait {
Q_OBJECT
public:
    explicit GraphicsViewOverlayedMediaWidget(QWidget *parent = nullptr);

    explicit GraphicsViewOverlayedMediaWidget(QGraphicsScene *scene, QWidget *parent = nullptr);

public slots:

    void on_media_geometry_changed(QRectF frame_rect, QRect render_rect, QSizeF render_scale) override;
};
