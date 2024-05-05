#include "detection_bounding_box.h"

#include <fmt/core.h>

#include <QPainter>

#include "../utils/color_utils.h"

DetectionBoundingBox::DetectionBoundingBox(
        DetectionBoundingBoxOptions options, QGraphicsItem *parent)
        : MediaOverlayingGraphicsItem(parent), bbox_{}, options_(std::move(options)) {
    if (options_.enable_focus())
        setAcceptHoverEvents(true);
}

DetectionBoundingBox::DetectionBoundingBox(
        OverlayGraphicsView *view,
        DetectionBoundingBoxOptions options,
        QGraphicsItem *parent)
        : MediaOverlayingGraphicsItem(view, parent), bbox_{}, options_(std::move(options)) {
    if (options_.enable_focus())
        setAcceptHoverEvents(true);
}

DetectionBoundingBox::DetectionBoundingBox(
        OverlayGraphicsView *view,
        int label_id,
        const QRectF &bbox,
        DetectionBoundingBoxOptions options,
        QGraphicsItem *parent)
        : DetectionBoundingBox(view, std::move(options), parent) {
    setLabelId(label_id);
    setBBox(bbox);
}

DetectionBoundingBox::DetectionBoundingBox(
        OverlayGraphicsView *view,
        int label_id,
        const QString &label,
        const QRectF &bbox,
        DetectionBoundingBoxOptions options,
        QGraphicsItem *parent)
        : DetectionBoundingBox(view, label_id, bbox, std::move(options), parent) {
    setLabel(label);
}

DetectionBoundingBox::DetectionBoundingBox(
        OverlayGraphicsView *view,
        int label_id,
        const QRectF &bbox,
        double confidence,
        DetectionBoundingBoxOptions options,
        QGraphicsItem *parent)
        : DetectionBoundingBox(view, label_id, bbox, std::move(options), parent) {
    setConfidence(confidence);
}

DetectionBoundingBox::DetectionBoundingBox(
        OverlayGraphicsView *view,
        int label_id,
        const QString &label,
        const QRectF &bbox,
        double confidence,
        DetectionBoundingBoxOptions options,
        QGraphicsItem *parent)
        : DetectionBoundingBox(view, label_id, bbox, confidence, std::move(options), parent) {
    setLabel(label);
}

DetectionBoundingBoxOptions DetectionBoundingBox::options() const {
    return options_;
}

int DetectionBoundingBox::labelId() const noexcept {
    return label_id_;
}

QString DetectionBoundingBox::label() const noexcept {
    return label_;
}

double DetectionBoundingBox::confidence() const noexcept {
    return confidence_;
}

QRectF DetectionBoundingBox::bbox() const noexcept {
    return bbox_;
}

QColor DetectionBoundingBox::color() const noexcept {
    return color_;
}

void DetectionBoundingBox::setLabelId(int label_id) {
    label_id_ = label_id;
}

void DetectionBoundingBox::setLabel(const QString &label) {
    label_ = label;
}

void DetectionBoundingBox::setConfidence(double confidence) {
    confidence_ = confidence;
}

void DetectionBoundingBox::setBBox(const QRectF &bbox) {
    bbox_ = bbox;
    setPos(bbox.topLeft());
}

void DetectionBoundingBox::setColor(const QColor &color) {
    color_ = color;
}

QRectF DetectionBoundingBox::boundingRect() const {
    return {0, 0, bbox_.width(), bbox_.height()};
}

QPainterPath DetectionBoundingBox::shape() const {
    auto rel_bbox = boundingRect();

    QPainterPath path;
    if (options_.shape() == DetectionBoundingBoxOptions::Rect) {
        path.addRect(rel_bbox);
    } else if (options_.shape() == DetectionBoundingBoxOptions::RoundedRect) {
        path.addRoundedRect(rel_bbox,
                            options_.rounded_rect_radius_ratio() * rel_bbox.width(),
                            options_.rounded_rect_radius_ratio() * rel_bbox.height());
    } else if (options_.shape() == DetectionBoundingBoxOptions::Ellipse) {
        path.addEllipse(rel_bbox);
    }
    return path;
}

void DetectionBoundingBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget) {
    auto rel_bbox = boundingRect();

    bool is_on_focus = isOnFocus();
    auto pen = QPen(color_, is_on_focus ? options_.focus_line_width() : options_.line_width());
    pen.setCosmetic(options_.cosmetic());
    auto brush = QBrush(QColor(color_.red(), color_.green(), color_.blue(),
                               is_on_focus ? options_.focus_fill_alpha() : options_.fill_alpha()));

    if (options_.antialiasing())
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    painter->setPen(pen);
    if (options_.fill())
        painter->setBrush(brush);

    if (options_.shape() == DetectionBoundingBoxOptions::Rect) {
        painter->drawRect(rel_bbox);
    } else if (options_.shape() == DetectionBoundingBoxOptions::RoundedRect) {
        QPainterPath path;
        path.addRoundedRect(rel_bbox,
                            options_.rounded_rect_radius_ratio() * rel_bbox.width(),
                            options_.rounded_rect_radius_ratio() * rel_bbox.height());
        if (options_.fill())
            painter->fillPath(path, brush);
        painter->drawPath(path);
    } else if (options_.shape() == DetectionBoundingBoxOptions::Ellipse) {
        painter->drawEllipse(rel_bbox);
    }

    if (options_.show_description() == DetectionBoundingBoxOptions::Always ||
        (is_on_focus && options_.show_description() == DetectionBoundingBoxOptions::OnFocus)) {
        auto font = QFont(options_.font_family(),
                          is_on_focus ? options_.focus_font_point_size() : options_.font_point_size());

        painter->save();
        auto trans = painter->transform();
        trans.translate(rel_bbox.x() + rel_bbox.width() / 2, rel_bbox.y());
        auto scale = pop_scale(trans);
        painter->setTransform(trans);
        {
            QString desc = label_.isEmpty() ? QString::fromStdString(fmt::format("[{}]", label_id_)) : label_;
            if (options_.show_confidence() == DetectionBoundingBoxOptions::Always ||
                (is_on_focus && options_.show_confidence() == DetectionBoundingBoxOptions::OnFocus))
                desc.append(QString::fromStdString(fmt::format(" ({:.02f}%)", confidence_ * 100)));

            auto fm = QFontMetricsF(font, view());
            auto text_width = fm.width(desc);
            auto text_height = fm.height();

            bool out_of_frame = y() - (text_height + pen.widthF() / 2) * scale < 0;
            auto text_y = out_of_frame ? pen.widthF() / 2 : -text_height - pen.widthF() / 2;
            auto text_rect = QRectF{-(text_width + font.pointSizeF()) / 2, text_y,
                                    text_width + font.pointSizeF(), text_height};
            if (options_.boxed_description()) {
                painter->fillRect(text_rect, QBrush(color_));
            }

            painter->setPen(options_.boxed_description() ? QPen(invertColor(color_, true)) : pen);
            painter->setFont(font);
            painter->drawText(text_rect, desc, Qt::AlignCenter | Qt::AlignTop);
        }
        painter->restore();
    }
}

void DetectionBoundingBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    on_focus_ = true;
    view()->update();
}

void DetectionBoundingBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!isSelected()) {
        on_focus_ = false;
        view()->update();
    }
}
