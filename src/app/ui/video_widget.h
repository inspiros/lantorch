#pragma once

#include "dnn/return_types.h"

#include "qt/ColorPalette"
#include "qt/OverlayGraphicsScene"
#include "qt/GraphicsViewOverlayedGstVideoWidget"
#include "qt/overlay_graphics_items/DetectionBoundingBox"
#include "qt/overlay_graphics_items/OverlayGraphicsItemPool"

class VideoWidget : public GraphicsViewOverlayedGstVideoWidget {
Q_OBJECT
    OverlayGraphicsScene *scene_;
    OverlayGraphicsItemPool<DetectionBoundingBox> *bbox_pool_;

public:
    DetectionBoundingBoxOptions bbox_options;
    ColorPalette bbox_color_palette;

    explicit VideoWidget(QWidget *parent = nullptr);

    [[nodiscard]] auto *bbox_pool() const noexcept;

public slots:
    QSharedPointer<DetectionBoundingBox> add_bbox(const Detection &det);

    QList<QSharedPointer<DetectionBoundingBox>> add_bboxes(const std::vector<Detection> &dets);

    QList<QSharedPointer<DetectionBoundingBox>> request_bboxes_from_pool(const std::vector<Detection> &dets);
};
