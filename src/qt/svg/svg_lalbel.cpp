#include "svg_label.h"

#include <QPainter>
#include <QPainterPath>

SvgLabel::SvgLabel(QWidget *parent) : SvgWidget(parent) {}

SvgLabel::SvgLabel(const QString &file, QWidget *parent) : SvgWidget(file, parent) {}

Qt::AspectRatioMode SvgLabel::aspectRatioMode() const {
    return renderer()->aspectRatioMode();
}

void SvgLabel::setAspectRatioMode(Qt::AspectRatioMode mode) {
    renderer()->setAspectRatioMode(mode);
}

void SvgLabel::paintEvent(QPaintEvent *event) {
    auto painter = QPainter(this);
    renderer()->render(&painter, rect());
    painter.end();
}
