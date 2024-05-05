#pragma once

#include "GstVideoWidget"
#include "GraphicsViewOverlayedMediaWidget"

#include "overlay_graphics_items/MediaOverlayingGraphicsItemFactory"

class GraphicsViewOverlayedGstVideoWidget : public GstVideoWidget, virtual public GraphicsViewOverlayedMediaTrait {
Q_OBJECT
public:
    explicit GraphicsViewOverlayedGstVideoWidget(QWidget *parent = nullptr);

    explicit GraphicsViewOverlayedGstVideoWidget(QGraphicsScene *scene, QWidget *parent = nullptr);

public slots:

    void on_media_geometry_changed(QRectF frame_rect, QRect render_rect, QSizeF render_scale) override;
};
