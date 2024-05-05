#include "svg_group_widget.h"

#include <QPainter>

SvgGroupWidget::SvgGroupWidget(QWidget *parent) : SvgWidget(parent) {
    renderers_ = new SvgRendererGroup(this);
    QObject::connect(renderers_, &SvgRendererGroup::repaintNeeded, this, [this]() {
        update();
    });
}

SvgGroupWidget::SvgGroupWidget(std::initializer_list<QString> filenames, QWidget *parent)
        : SvgGroupWidget(parent) {
    for (const auto &filename: filenames) {
        renderers_->insertNew(renderers_->renderers().constFind(1), 0, filename);
    }
}

SvgGroupWidget::SvgGroupWidget(
        std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs,
        QWidget *parent)
        : SvgWidget(parent) {
    renderers_ = new SvgRendererGroup(contents_pairs, this);
    QObject::connect(renderers_, &SvgRendererGroup::repaintNeeded, this, [this]() {
        update();
    });
}

SvgGroupWidget::~SvgGroupWidget() {
    renderers_->deleteLater();
}

void SvgGroupWidget::setSharedRendererGroup(SvgRendererGroup *group) {
    renderers_->disconnect(this);
    renderers_ = group;
    QObject::connect(renderers_, &SvgRendererGroup::repaintNeeded, this, [this]() {
        update();
    }, Qt::DirectConnection);
    update();
}

SvgRendererGroup *SvgGroupWidget::rendererGroup() const {
    return renderers_;
}

void SvgGroupWidget::load(
        std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs) {
    renderers_->load(contents_pairs);
}

void SvgGroupWidget::load(std::initializer_list<std::pair<int, QString>> filenames) {
    renderers_->load(filenames);
}

void SvgGroupWidget::load(const QMap<int, QString> &filenames_map) {
    renderers_->load(filenames_map);
}

void SvgGroupWidget::load(std::initializer_list<std::pair<int, QByteArray>> contents_pairs) {
    renderers_->load(contents_pairs);
}

void SvgGroupWidget::load(const QMap<int, QByteArray> &contents_map) {
    renderers_->load(contents_map);
}

void SvgGroupWidget::paintEvent(QPaintEvent *event) {
    auto painter = QPainter(this);
    auto svg_size = renderers_->defaultSize();
    renderers_->render(&painter, {0, 0,
                                  static_cast<qreal>(svg_size.width()),
                                  static_cast<qreal>(svg_size.height())});
    painter.end();
}
