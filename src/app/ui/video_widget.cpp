#include "video_widget.h"

#include "../app_config.h"
#include "../macros.h"

#include <QDebug>

VideoWidget::VideoWidget(QWidget *parent)
        : GraphicsViewOverlayedGstVideoWidget(parent) {
    auto configs = AppConfig::instance()["app"]["ui"]["main_window"]["video_widget"];

    scene_ = new OverlayGraphicsScene(this);
    scene_->setSceneRect(-10000, -10000, 20000, 20000);
    scene_->setItemIndexMethod(QGraphicsScene::NoIndex);
    view()->setScene(scene_);

    auto bbox_config = configs["detection_bounding_box_options"];
    bbox_options = bbox_options
            .shape(bbox_config["shape"].as<DetectionBoundingBoxOptions::Shape>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(shape)))
            .rounded_rect_radius_ratio(bbox_config["rounded_rect_radius"].as<qreal>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(rounded_rect_radius_ratio)))
            .line_width(bbox_config["line_width"].as<qreal>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(line_width)))
            .focus_line_width(bbox_config["focus_line_width"].as<qreal>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(focus_line_width)))
            .fill(bbox_config["fill"].as<bool>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(fill)))
            .fill_alpha(bbox_config["fill_alpha"].as<int>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(fill_alpha)))
            .focus_fill_alpha(bbox_config["focus_fill_alpha"].as<int>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(focus_fill_alpha)))
            .show_description(bbox_config["show_description"].as<DetectionBoundingBoxOptions::ShowFlag>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(show_description)))
            .show_confidence(bbox_config["show_confidence"].as<DetectionBoundingBoxOptions::ShowFlag>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(show_confidence)))
            .boxed_description(bbox_config["boxed_description"].as<bool>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(boxed_description)))
            .font_family(bbox_config["font_family"].as<QString>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(font_family)))
            .font_point_size(bbox_config["font_point_size"].as<int>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(font_point_size)))
            .focus_font_point_size(bbox_config["focus_font_point_size"].as<int>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(focus_font_point_size)))
            .enable_focus(bbox_config["enable_focus"].as<bool>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(enable_focus)))
            .cosmetic(bbox_config["cosmetic"].as<bool>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(cosmetic)))
            .antialiasing(bbox_config["antialiasing"].as<bool>(
                    DetectionBoundingBoxOptions::DEFAULT_OPTION(antialiasing)));
    bbox_color_palette = ColorPalette(configs["bbox_color_palette"].as<ColorPalette>(ColorPalette()));

    bbox_pool_ = new OverlayGraphicsItemPool<DetectionBoundingBox>(view());
    bbox_pool_->factory_resize(100, graphics_item_factory(), bbox_options, nullptr);
    bbox_pool_->addToScene(scene_);
}

auto *VideoWidget::bbox_pool() const noexcept {
    return bbox_pool_;
}

QSharedPointer<DetectionBoundingBox> VideoWidget::add_bbox(const Detection &det) {
    auto bbox = graphics_item_factory()->createItem<DetectionBoundingBox>(
            det.label_id,
            QString::fromStdString(det.label),
            QRectF(det.bbox.x, det.bbox.y, det.bbox.width, det.bbox.height),
            det.confidence,
            bbox_options);
    bbox->setColor(bbox_color_palette.at(det.label_id));
    scene()->addItem(bbox.data());
    return bbox;
}

QList<QSharedPointer<DetectionBoundingBox>> VideoWidget::add_bboxes(const std::vector<Detection> &dets) {
    QList<QSharedPointer<DetectionBoundingBox>> bboxes;
    bboxes.reserve((int) dets.size());
    for (const auto &det: dets) {
        auto bbox = graphics_item_factory()->createItem<DetectionBoundingBox>(
                det.label_id,
                QString::fromStdString(det.label),
                QRectF(det.bbox.x, det.bbox.y, det.bbox.width, det.bbox.height),
                det.confidence,
                bbox_options);
        bbox->setColor(bbox_color_palette.at(det.label_id));
        scene()->addItem(bbox.data());
        bboxes.push_back(bbox);
    }
    return bboxes;
}

QList<QSharedPointer<DetectionBoundingBox>> VideoWidget::request_bboxes_from_pool(
        const std::vector<Detection> &dets) {
    auto active_bboxes = bbox_pool_->request(std::min((int) dets.size(), bbox_pool_->size()));
    for (auto i = 0; i < dets.size(); i++) {
        auto item = active_bboxes[i];
        auto det = dets[i];
        item->setLabelId(det.label_id);
        item->setLabel(QString::fromStdString(det.label));
        item->setConfidence(det.confidence);
        item->setBBox(QRectF(det.bbox.x, det.bbox.y, det.bbox.width, det.bbox.height));
        item->setColor(bbox_color_palette.at(det.label_id));
    }
    update();
    return active_bboxes;
}
