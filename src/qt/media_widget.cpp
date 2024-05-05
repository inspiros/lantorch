#include "media_widget.h"

#include <cmath>

#include <QPainter>
#include <QResizeEvent>
#include <QTimer>

MediaWidget::MediaWidget(QWidget *parent)
        : QWidget(parent) {
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowStaysOnBottomHint);
    setAttribute(Qt::WA_NativeWindow);
    QObject::connect(this, &MediaWidget::frame_size_changed,
                     this, &MediaWidget::on_frame_size_changed);
    QObject::connect(this, &MediaWidget::media_geometry_changed,
                     this, &MediaWidget::on_media_geometry_changed);
    QObject::connect(this, &MediaWidget::media_geometry_changed,
                     this, [this]() {
                update();
            }, Qt::DirectConnection);
}

QSize MediaWidget::frame_size() const {
    return frame_size_;
}

void MediaWidget::set_force_aspect_ratio(bool force_aspect_ratio) {
    force_aspect_ratio_ = force_aspect_ratio;
    emit force_aspect_ratio_changed(force_aspect_ratio_);
}

bool MediaWidget::force_aspect_ratio() const {
    return force_aspect_ratio_;
}

QRectF MediaWidget::frame_rect() const {
    return frame_rect_;
}

QRect MediaWidget::render_rect() const {
    return render_rect_;
}

QSizeF MediaWidget::render_scale() const {
    return render_scale_;
}

void MediaWidget::set_frame(const QImage &frame) {
    frame_ = frame;
    emit frame_changed(frame_);
    if (frame_size_ != frame_.size())
            emit frame_size_changed(frame_.size());
    update();
}

void MediaWidget::on_frame_size_changed(QSize frame_size) {
    frame_size_ = frame_size;
    update_media_geometry();
}

void MediaWidget::on_media_geometry_changed(QRectF frame_rect, QRect render_rect, QSizeF render_scale) {
}

void MediaWidget::update_media_geometry(QSize widget_size) {
    if (frame_size_.isEmpty() || !force_aspect_ratio_) {
        render_rect_ = widget_size.isEmpty() ? rect() : QRect(0, 0, widget_size.width(), widget_size.height());
        emit render_rect_changed(render_rect_);
    } else {
        if (widget_size.isEmpty())
            widget_size = size();
        // render_scale
        auto scale_x = static_cast<double>(widget_size.width()) / static_cast<double>(frame_size_.width());
        auto scale_y = static_cast<double>(widget_size.height()) / static_cast<double>(frame_size_.height());
        auto resize_scale = std::min(scale_x, scale_y);
        render_scale_ = !force_aspect_ratio_ ? QSizeF{scale_x, scale_y} : QSizeF{resize_scale, resize_scale};
        // render_rect
        auto new_width = (int) (frame_size_.width() * resize_scale);
        auto new_height = (int) (frame_size_.height() * resize_scale);
        render_rect_ = {(widget_size.width() - new_width) / 2,
                        (widget_size.height() - new_height) / 2,
                        new_width, new_height};
        // frame_rect
        auto reverse_scale_x = static_cast<double>(frame_size_.width()) / new_width;
        auto reverse_scale_y = static_cast<double>(frame_size_.height()) / new_height;
        frame_rect_ = {render_rect_.x() * reverse_scale_x,
                       render_rect_.y() * reverse_scale_y,
                       static_cast<qreal>(frame_size_.width()),
                       static_cast<qreal>(frame_size_.height())};
        emit render_scale_changed(render_scale_);
        emit render_rect_changed(render_rect_);
        emit frame_rect_changed(frame_rect_);
    }
    emit media_geometry_changed(frame_rect_, render_rect_, render_scale_);
}

void MediaWidget::resizeEvent(QResizeEvent *event) {
    QTimer::singleShot(0, this, SLOT(update_media_geometry(event->size())));
    QWidget::resizeEvent(event);
}

void MediaWidget::paintEvent(QPaintEvent *event) {
    if (!frame_.isNull()) {
        auto painter = QPainter(this);
        painter.drawImage(!render_rect_.isNull() ? render_rect_ : rect(), frame_);
        painter.end();
    }
}
