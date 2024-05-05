#include "svg_widget.h"

#include <QPainter>

SvgWidget::SvgWidget(QWidget *parent) : QSvgWidget(parent), renderer_(new SvgRenderer) {
    QObject::connect(renderer_, &SvgRenderer::repaintNeeded, this, [this]() {
        update();
    });
}

SvgWidget::SvgWidget(const QString &file, QWidget *parent) : QSvgWidget(parent) {
    renderer_ = new SvgRenderer(file, this);
    QObject::connect(renderer_, &SvgRenderer::repaintNeeded, this, [this]() {
        update();
    });
}

SvgWidget::~SvgWidget() {
    renderer_->deleteLater();
}

void SvgWidget::setSharedRenderer(SvgRenderer *renderer) {
    renderer_->disconnect(this);
    renderer_ = renderer;
    QObject::connect(renderer_, &SvgRenderer::repaintNeeded, this, [this]() {
        update();
    }, Qt::DirectConnection);
    update();
}

SvgRenderer *SvgWidget::renderer() const {
    return renderer_;
}

QSize SvgWidget::sizeHint() const {
    auto default_size = renderer_->defaultSize();
    if (default_size.isEmpty())
        return QSvgWidget::sizeHint();
    return default_size;
}

void SvgWidget::load(const QString &file) {
    renderer_->load(file);
}

void SvgWidget::load(const QByteArray &contents) {
    renderer_->load(contents);
}

void SvgWidget::paintEvent(QPaintEvent *event) {
    auto painter = QPainter(this);
    auto svg_size = renderer_->defaultSize();
    renderer_->render(&painter, {0, 0,
                                 static_cast<qreal>(svg_size.width()),
                                 static_cast<qreal>(svg_size.height())});
    painter.end();
}
