#include "svg_group_label.h"

#include <QPainter>

SvgGroupLabel::SvgGroupLabel(QWidget *parent) : SvgGroupWidget(parent) {}

SvgGroupLabel::SvgGroupLabel(std::initializer_list<QString> filenames, QWidget *parent)
        : SvgGroupWidget(filenames, parent) {}

SvgGroupLabel::SvgGroupLabel(
        std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs,
        QWidget *parent)
        : SvgGroupWidget(contents_pairs, parent) {}

Qt::AspectRatioMode SvgGroupLabel::aspectRatioMode() const {
    return rendererGroup()->aspectRatioMode();
}

void SvgGroupLabel::setAspectRatioMode(Qt::AspectRatioMode mode) {
    rendererGroup()->setAspectRatioMode(mode);
}

void SvgGroupLabel::paintEvent(QPaintEvent *event) {
    auto painter = QPainter(this);
    rendererGroup()->render(&painter, rect());
    painter.end();
}
