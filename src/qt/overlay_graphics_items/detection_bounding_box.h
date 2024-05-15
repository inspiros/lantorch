#pragma once

#include "MediaOverlayingGraphicsItem"

class DetectionBoundingBoxOptions {
public:
    enum Shape {
        Rect, RoundedRect, Ellipse
    };

    enum ShowFlag {
        Always, OnFocus, Never
    };

    DetectionBoundingBoxOptions()
            : shape_(Rect),
              rounded_rect_radius_ratio_(0.05),
              line_width_(2),
              focus_line_width_(3),
              fill_(false),
              fill_alpha_(30),
              focus_fill_alpha_(30),
              show_description_(Always),
              show_confidence_(OnFocus),
              boxed_description_(false),
              font_family_(""),
              font_point_size_(11),
              focus_font_point_size_(12),
              enable_focus_(true),
              cosmetic_(true),
              antialiasing_(false) {}

    // setters
    [[nodiscard]] inline DetectionBoundingBoxOptions shape(Shape shape) const noexcept {
        auto r = *this;
        r.shape_ = shape;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions rounded_rect_radius_ratio(qreal rounded_rect_radius_ratio) const noexcept {
        auto r = *this;
        r.rounded_rect_radius_ratio_ = rounded_rect_radius_ratio;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions line_width(qreal line_width) const noexcept {
        auto r = *this;
        r.line_width_ = line_width;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions focus_line_width(qreal focus_line_width) const noexcept {
        auto r = *this;
        r.focus_line_width_ = focus_line_width;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions fill(bool fill) const noexcept {
        auto r = *this;
        r.fill_ = fill;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions fill_alpha(int fill_alpha) const noexcept {
        auto r = *this;
        r.fill_alpha_ = fill_alpha;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions focus_fill_alpha(int focus_fill_alpha) const noexcept {
        auto r = *this;
        r.focus_fill_alpha_ = focus_fill_alpha;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions show_description(ShowFlag show_description) const noexcept {
        auto r = *this;
        r.show_description_ = show_description;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions show_confidence(ShowFlag show_confidence) const noexcept {
        auto r = *this;
        r.show_confidence_ = show_confidence;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions boxed_description(bool boxed_description) const noexcept {
        auto r = *this;
        r.boxed_description_ = boxed_description;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions font_family(const QString &font_family) const noexcept {
        auto r = *this;
        r.font_family_ = font_family;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions font_point_size(int font_point_size) const noexcept {
        auto r = *this;
        r.font_point_size_ = font_point_size;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions focus_font_point_size(int focus_font_point_size) const noexcept {
        auto r = *this;
        r.focus_font_point_size_ = focus_font_point_size;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions enable_focus(bool enable_focus) const noexcept {
        auto r = *this;
        r.enable_focus_ = enable_focus;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions cosmetic(bool cosmetic) const noexcept {
        auto r = *this;
        r.cosmetic_ = cosmetic;
        return r;
    }

    [[nodiscard]] inline DetectionBoundingBoxOptions antialiasing(bool antialiasing) const noexcept {
        auto r = *this;
        r.antialiasing_ = antialiasing;
        return r;
    }

    // getters
    [[nodiscard]] inline Shape shape() const noexcept {
        return shape_;
    }

    [[nodiscard]] inline qreal rounded_rect_radius_ratio() const noexcept {
        return rounded_rect_radius_ratio_;
    }

    [[nodiscard]] inline qreal line_width() const noexcept {
        return line_width_;
    }

    [[nodiscard]] inline qreal focus_line_width() const noexcept {
        return focus_line_width_;
    }

    [[nodiscard]] inline bool fill() const noexcept {
        return fill_;
    }

    [[nodiscard]] inline int fill_alpha() const noexcept {
        return fill_alpha_;
    }

    [[nodiscard]] inline int focus_fill_alpha() const noexcept {
        return focus_fill_alpha_;
    }

    [[nodiscard]] inline ShowFlag show_description() const noexcept {
        return show_description_;
    }

    [[nodiscard]] inline ShowFlag show_confidence() const noexcept {
        return show_confidence_;
    }

    [[nodiscard]] inline bool boxed_description() const noexcept {
        return boxed_description_;
    }

    [[nodiscard]] inline QString font_family() const noexcept {
        return font_family_;
    }

    [[nodiscard]] inline int font_point_size() const noexcept {
        return font_point_size_;
    }

    [[nodiscard]] inline int focus_font_point_size() const noexcept {
        return focus_font_point_size_;
    }

    [[nodiscard]] inline bool enable_focus() const noexcept {
        return enable_focus_;
    }

    [[nodiscard]] inline bool cosmetic() const noexcept {
        return cosmetic_;
    }

    [[nodiscard]] inline bool antialiasing() const noexcept {
        return antialiasing_;
    }

private:
    Shape shape_;
    qreal rounded_rect_radius_ratio_;
    qreal line_width_;
    qreal focus_line_width_;

    bool fill_;
    int fill_alpha_;
    int focus_fill_alpha_;

    ShowFlag show_description_;
    ShowFlag show_confidence_;
    bool boxed_description_;
    QString font_family_;
    int font_point_size_;
    int focus_font_point_size_;

    bool enable_focus_;
    bool cosmetic_;
    bool antialiasing_;
};

class DetectionBoundingBox : public MediaOverlayingGraphicsItem {
    int label_id_ = -1;
    QString label_;
    double confidence_ = 0;
    QRectF bbox_;

    QColor color_;
    bool on_focus_ = false;
    DetectionBoundingBoxOptions options_;

public:
    explicit DetectionBoundingBox(DetectionBoundingBoxOptions options = {},
                                  QGraphicsItem *parent = nullptr);

    explicit DetectionBoundingBox(OverlayGraphicsView *view,
                                  DetectionBoundingBoxOptions options = {},
                                  QGraphicsItem *parent = nullptr);

    explicit DetectionBoundingBox(OverlayGraphicsView *view,
                                  int label_id,
                                  const QRectF &bbox,
                                  DetectionBoundingBoxOptions options = {},
                                  QGraphicsItem *parent = nullptr);

    explicit DetectionBoundingBox(OverlayGraphicsView *view,
                                  int label_id,
                                  const QString &label,
                                  const QRectF &bbox,
                                  DetectionBoundingBoxOptions options = {},
                                  QGraphicsItem *parent = nullptr);

    explicit DetectionBoundingBox(OverlayGraphicsView *view,
                                  int label_id,
                                  const QRectF &bbox,
                                  double confidence,
                                  DetectionBoundingBoxOptions options = {},
                                  QGraphicsItem *parent = nullptr);

    explicit DetectionBoundingBox(OverlayGraphicsView *view,
                                  int label_id,
                                  const QString &label,
                                  const QRectF &bbox,
                                  double confidence,
                                  DetectionBoundingBoxOptions options = {},
                                  QGraphicsItem *parent = nullptr);

    [[nodiscard]] DetectionBoundingBoxOptions options() const;

    [[nodiscard]] int labelId() const noexcept;

    [[nodiscard]] QString label() const noexcept;

    [[nodiscard]] double confidence() const noexcept;

    [[nodiscard]] QRectF bbox() const noexcept;

    [[nodiscard]] QColor color() const noexcept;

    void setLabelId(int label_id);

    void setLabel(const QString &label);

    void setConfidence(double confidence);

    void setBBox(const QRectF &bbox);

    void setColor(const QColor &color);

    [[nodiscard]] inline QSizeF size() const {
        return bbox_.size();
    }

    [[nodiscard]] inline qreal width() const {
        return bbox_.width();
    }

    [[nodiscard]] inline qreal height() const {
        return bbox_.height();
    }

    [[nodiscard]] inline bool isOnFocus() const {
        return on_focus_;
    }

    [[nodiscard]] QRectF boundingRect() const override;

    [[nodiscard]] QPainterPath shape() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};
