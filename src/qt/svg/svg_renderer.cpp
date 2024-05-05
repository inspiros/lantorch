#include "svg_renderer.h"

#include <QFile>
#include <QTransform>

SvgRenderer::SvgRenderer(QObject *parent) : QObject(parent), base_renderer_(new QSvgRenderer) {
    QObject::connect(base_renderer_, &QSvgRenderer::repaintNeeded, this, [this]() {
        emit repaintNeeded();
    }, Qt::DirectConnection);
}

SvgRenderer::SvgRenderer(QSvgRenderer *base_renderer, QObject *parent)
        : QObject(parent), base_renderer_(base_renderer) {
    QObject::connect(base_renderer_, &QSvgRenderer::repaintNeeded, this, [this]() {
        emit repaintNeeded();
    }, Qt::DirectConnection);
}

SvgRenderer::SvgRenderer(const QString &filename, QObject *parent)
        : SvgRenderer(parent) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    load(file.readAll());
}

SvgRenderer::SvgRenderer(const QByteArray &contents, QObject *parent)
        : SvgRenderer(parent) {
    load(contents);
}

SvgRenderer::SvgRenderer(QXmlStreamReader *contents, QObject *parent)
        : SvgRenderer(parent) {
    load(contents);
}

QSvgRenderer *SvgRenderer::baseRenderer() const {
    return base_renderer_;
}

QByteArray SvgRenderer::contents() const {
    return svg_contents_;
}

bool SvgRenderer::isVisible() const {
    return visible_;
}

void SvgRenderer::setVisible(bool visible) {
    bool should_repaint = visible_ != visible;
    visible_ = visible;
    if (should_repaint) {
        emit repaintNeeded();
    }
}

void SvgRenderer::show() {
    bool should_repaint = !visible_;
    visible_ = true;
    if (should_repaint) {
        emit repaintNeeded();
    }
}

void SvgRenderer::hide() {
    bool should_repaint = visible_;
    visible_ = false;
    if (should_repaint) {
        emit repaintNeeded();
    }
}

bool SvgRenderer::isValid() const {
    return base_renderer_->isValid();
}

QSize SvgRenderer::defaultSize() const {
    if (!base_renderer_)
        return {0, 0};
    return base_renderer_->defaultSize();
}

QRect SvgRenderer::viewBox() const {
    return base_renderer_->viewBox();
}

QRectF SvgRenderer::viewBoxF() const {
    return base_renderer_->viewBoxF();
}

void SvgRenderer::setViewBox(const QRect &viewbox) {
    base_renderer_->setViewBox(viewbox);
}

void SvgRenderer::setViewBox(const QRectF &viewbox) {
    base_renderer_->setViewBox(viewbox);
}

Qt::AspectRatioMode SvgRenderer::aspectRatioMode() const {
    return base_renderer_->aspectRatioMode();
}

void SvgRenderer::setAspectRatioMode(Qt::AspectRatioMode mode) {
    base_renderer_->setAspectRatioMode(mode);
}

bool SvgRenderer::animated() const {
    return base_renderer_->animated();
}

int SvgRenderer::framesPerSecond() const {
    return base_renderer_->framesPerSecond();
}

void SvgRenderer::setFramesPerSecond(int num) {
    base_renderer_->setFramesPerSecond(num);
}

int SvgRenderer::currentFrame() const {
    return base_renderer_->currentFrame();
}

void SvgRenderer::setCurrentFrame(int frame) {
    base_renderer_->setCurrentFrame(frame);
}

int SvgRenderer::animationDuration() const {
    return base_renderer_->animationDuration();
}

QRectF SvgRenderer::boundsOnElement(const QString &id) const {
    return base_renderer_->boundsOnElement(id);
}

bool SvgRenderer::elementExists(const QString &id) const {
    return base_renderer_->elementExists(id);
}

QTransform SvgRenderer::transformForElement(const QString &id) const {
    return base_renderer_->transformForElement(id);
}

void SvgRenderer::setSharedRenderer(QSvgRenderer *base_renderer) {
    base_renderer_ = base_renderer;
}

SvgRenderer *SvgRenderer::clone() const {
    auto *copy = new SvgRenderer(contents());
    copy->setAspectRatioMode(aspectRatioMode());
    copy->setVisible(visible_);
    return copy;
}

SvgRenderer *SvgRenderer::cloneShared() const {
    auto *copy = new SvgRenderer(base_renderer_);
    copy->setContents(svg_contents_);
    copy->setAspectRatioMode(aspectRatioMode());
    copy->setVisible(visible_);
    return copy;
}

SvgRenderer *SvgRenderer::newShared() const {
    auto *copy = new SvgRenderer(base_renderer_);
    copy->setContents(svg_contents_);
    return copy;
}

bool SvgRenderer::load(const QString &filename) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDomDocument svg_doc(file.readAll());
    svg_contents_ = svg_doc.toByteArray();
    bool res = base_renderer_->load(svg_contents_);
    emit repaintNeeded();
    return res;
}

bool SvgRenderer::load(const QByteArray &contents) {
    svg_contents_ = contents;
    bool res = base_renderer_->load(svg_contents_);
    emit repaintNeeded();
    return res;
}

bool SvgRenderer::load(QXmlStreamReader *contents) {
    QDomDocument svg_doc;
    svg_doc.setContent(contents, true);
    svg_contents_ = svg_doc.toByteArray();
    bool res = base_renderer_->load(svg_contents_);
    emit repaintNeeded();
    return res;
}

void SvgRenderer::render(QPainter *p) {
    if (isVisible())
        base_renderer_->render(p);
}

void SvgRenderer::render(QPainter *p, const QRectF &bounds) {
    if (isVisible())
        base_renderer_->render(p, bounds);
}

void SvgRenderer::render(QPainter *p, const QString &elementId, const QRectF &bounds) {
    if (isVisible())
        base_renderer_->render(p, elementId, bounds);
}

void SvgRenderer::setContents(const QByteArray &contents) {
    svg_contents_ = contents;
}
