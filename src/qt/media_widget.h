#pragma once

#include <QWidget>

class MediaWidget : public QWidget {
Q_OBJECT
    QImage frame_;
    QSize frame_size_;
    bool force_aspect_ratio_ = false;
    QRectF frame_rect_ = {0, 0, -1, -1};
    QRect render_rect_;
    QSizeF render_scale_ = {1, 1};

public:
    explicit MediaWidget(QWidget *parent = nullptr);

    [[nodiscard]] bool force_aspect_ratio() const;

    [[nodiscard]] QRectF frame_rect() const;

    [[nodiscard]] inline QPointF frame_offset() const {
        return frame_rect().topLeft();
    }

    [[nodiscard]] QSize frame_size() const;

    [[nodiscard]] inline int frame_width() const {
        return frame_size().width();
    }

    [[nodiscard]] inline int frame_height() const {
        return frame_size().height();
    }

    [[nodiscard]] QRect render_rect() const;

    [[nodiscard]] inline QPoint render_offset() const {
        return render_rect().topLeft();
    }

    [[nodiscard]] inline QSize render_size() const {
        return render_rect().size();
    }

    [[nodiscard]] inline qreal render_width() const {
        return render_size().width();
    }

    [[nodiscard]] inline qreal render_height() const {
        return render_size().height();
    }

    [[nodiscard]] QSizeF render_scale() const;

signals:

    void frame_changed(const QImage &frame);

    void force_aspect_ratio_changed(bool force_aspect_ratio);

    void frame_size_changed(QSize frame_size);

    void frame_rect_changed(QRectF frame_rect);

    void render_rect_changed(QRect render_rect);

    void render_scale_changed(QSizeF render_scale);

    void media_geometry_changed(QRectF frame_rect, QRect render_rect, QSizeF render_scale);

public slots:

    void set_frame(const QImage& frame);

    void set_force_aspect_ratio(bool force_aspect_ratio);

    virtual void on_frame_size_changed(QSize frame_size);

    virtual void on_media_geometry_changed(QRectF frame_rect, QRect render_rect, QSizeF render_scale);

private slots:

    virtual void update_media_geometry(QSize widget_size = {});

protected:
    void resizeEvent(QResizeEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
};
