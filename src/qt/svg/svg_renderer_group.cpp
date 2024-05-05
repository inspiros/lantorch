#include "svg_renderer_group.h"

SvgRendererGroup::SvgRendererGroup(QObject *parent) : QObject(parent) {}

SvgRendererGroup::SvgRendererGroup(
        std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs,
        QObject *parent)
        : QObject(parent) {
    load(contents_pairs);
}

SvgRendererGroup::SvgRendererGroup(std::initializer_list<std::pair<int, QString>> filenames, QObject *parent)
        : QObject(parent) {
    load(filenames);
}

SvgRendererGroup::SvgRendererGroup(const QMap<int, QString> &filenames_map, QObject *parent)
        : QObject(parent) {
    load(filenames_map);
}

SvgRendererGroup::SvgRendererGroup(std::initializer_list<std::pair<int, QByteArray>> contents_pairs, QObject *parent)
        : QObject(parent) {
    load(contents_pairs);
}

SvgRendererGroup::SvgRendererGroup(const QMap<int, QByteArray> &contents_map, QObject *parent)
        : QObject(parent) {
    load(contents_map);
}

SvgRendererGroup::SvgRendererGroup(std::initializer_list<std::pair<int, QXmlStreamReader *>> contents_pairs,
                                   QObject *parent)
        : QObject(parent) {
    for (const auto &[z, contents]: contents_pairs)
        insertNew(z, contents);
}

SvgRendererGroup::SvgRendererGroup(const QMap<int, QXmlStreamReader *> &contents_map, QObject *parent)
        : QObject(parent) {
    for (auto it = contents_map.keyValueBegin(); it != contents_map.keyValueEnd(); ++it)
        insertNew((*it).first, (*it).second);
}

SvgRendererGroup::~SvgRendererGroup() {
    for (const auto renderer: renderers_)
        renderer->deleteLater();
    iterators_.clear();
    renderers_.clear();
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::insert(
        int z, SvgRenderer *renderer) {
    auto itit = iterators_.find(renderer);
    bool already_inserted = itit != iterators_.end();
    if (already_inserted) {
        auto it = *itit;
        if (renderers_.key(renderer) == z)
            return it;
        renderers_.erase(it);
        iterators_.remove(renderer);
    }
    auto it = renderers_.insert(z, renderer);
    iterators_.insert(renderer, it);
    if (!already_inserted) {
        QObject::connect(*it, &SvgRenderer::repaintNeeded, this, [this]() {
            emit repaintNeeded();
        });
    }
    return it;
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::insert(
        SvgRendererGroup::renderers_const_iterator pos, int z, SvgRenderer *renderer) {
    auto itit = iterators_.find(renderer);
    bool already_inserted = itit != iterators_.end();
    if (already_inserted) {
        auto it = *itit;
        renderers_.erase(it);
        iterators_.remove(renderer);
    }
    auto it = renderers_.insert(pos, z, renderer);
    iterators_.insert(renderer, it);
    if (!already_inserted) {
        QObject::connect(*it, &SvgRenderer::repaintNeeded, this, [this]() {
            emit repaintNeeded();
        });
    }
    return it;
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::insertNew(
        int z, const QString &filename) {
    auto *renderer = new SvgRenderer(filename);
    renderer->setAspectRatioMode(aspect_ratio_mode_);
    auto it = renderers_.insert(z, renderer);
    iterators_.insert(renderer, it);
    QObject::connect(*it, &SvgRenderer::repaintNeeded, this, [this]() {
        emit repaintNeeded();
    });
    emit repaintNeeded();
    return it;
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::insertNew(
        renderers_const_iterator pos, int z, const QString &filename) {
    auto *renderer = new SvgRenderer(filename);
    renderer->setAspectRatioMode(aspect_ratio_mode_);
    auto it = renderers_.insert(pos, z, renderer);
    iterators_.insert(renderer, it);
    QObject::connect(*it, &SvgRenderer::repaintNeeded, this, [this]() {
        emit repaintNeeded();
    });
    emit repaintNeeded();
    return it;
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::insertNew(
        int z, const QByteArray &contents) {
    auto *renderer = new SvgRenderer(contents);
    renderer->setAspectRatioMode(aspect_ratio_mode_);
    auto it = renderers_.insert(z, renderer);
    iterators_.insert(renderer, it);
    QObject::connect(*it, &SvgRenderer::repaintNeeded, this, [this]() {
        emit repaintNeeded();
    });
    emit repaintNeeded();
    return it;
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::insertNew(
        renderers_const_iterator pos, int z, const QByteArray &contents) {
    auto *renderer = new SvgRenderer(contents);
    renderer->setAspectRatioMode(aspect_ratio_mode_);
    auto it = renderers_.insert(pos, z, renderer);
    iterators_.insert(renderer, it);
    QObject::connect(*it, &SvgRenderer::repaintNeeded, this, [this]() {
        emit repaintNeeded();
    });
    emit repaintNeeded();
    return it;
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::insertNew(
        int z, QXmlStreamReader *contents) {
    auto *renderer = new SvgRenderer(contents);
    renderer->setAspectRatioMode(aspect_ratio_mode_);
    auto it = renderers_.insert(z, renderer);
    iterators_.insert(renderer, it);
    QObject::connect(*it, &SvgRenderer::repaintNeeded, this, [this]() {
        emit repaintNeeded();
    });
    emit repaintNeeded();
    return it;
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::insertNew(
        renderers_const_iterator pos, int z, QXmlStreamReader *contents) {
    auto *renderer = new SvgRenderer(contents);
    renderer->setAspectRatioMode(aspect_ratio_mode_);
    auto it = renderers_.insert(pos, z, renderer);
    iterators_.insert(renderer, it);
    QObject::connect(*it, &SvgRenderer::repaintNeeded, this, [this]() {
        emit repaintNeeded();
    });
    emit repaintNeeded();
    return it;
}

SvgRendererGroup::renderers_iterator SvgRendererGroup::remove(SvgRenderer *renderer) {
    auto itit = iterators_.find(renderer);
    if (itit != iterators_.end()) {
        auto it = *itit;
        (*it)->disconnect(this);
        it = renderers_.erase(it);
        emit repaintNeeded();
        return it;
    }
    return renderers_.end();
}

QMultiMap<int, SvgRenderer *> SvgRendererGroup::renderers() const {
    return renderers_;
}

QMap<SvgRenderer *, SvgRendererGroup::renderers_iterator> SvgRendererGroup::renderer_iterators() const {
    return iterators_;
}

Qt::AspectRatioMode SvgRendererGroup::aspectRatioMode() const {
    return aspect_ratio_mode_;
}

void SvgRendererGroup::setAspectRatioMode(Qt::AspectRatioMode mode) {
    aspect_ratio_mode_ = mode;
    for (const auto &renderer: renderers_) {
        renderer->setAspectRatioMode(mode);
    }
    emit repaintNeeded();
}

bool SvgRendererGroup::isVisible() const {
    return visible_;
}

void SvgRendererGroup::setVisible(bool visible) {
    visible_ = visible;
    emit repaintNeeded();
}

void SvgRendererGroup::show() {
    visible_ = true;
    emit repaintNeeded();
}

void SvgRendererGroup::hide() {
    visible_ = false;
    emit repaintNeeded();
}

QSize SvgRendererGroup::defaultSize() const {
    int width = 0, height = 0;
    for (const auto &renderer: renderers_) {
        width = std::max(width, renderer->defaultSize().width());
        height = std::max(height, renderer->defaultSize().height());
    }
    return {width, height};
}

SvgRendererGroup *SvgRendererGroup::clone() const {
    auto copy = new SvgRendererGroup;
    copy->setVisible(visible_);
    copy->setAspectRatioMode(aspect_ratio_mode_);
    for (auto it = renderers_.keyValueBegin(); it != renderers_.keyValueEnd(); it++) {
        int z = it->first;
        auto *renderer = it->second;
        copy->insert(z, renderer->clone());
    }
    return copy;
}

SvgRendererGroup *SvgRendererGroup::cloneShared() const {
    auto copy = new SvgRendererGroup;
    copy->setVisible(visible_);
    copy->setAspectRatioMode(aspect_ratio_mode_);
    for (auto it = renderers_.keyValueBegin(); it != renderers_.keyValueEnd(); it++) {
        int z = it->first;
        auto *renderer = it->second;
        copy->insert(z, renderer->cloneShared());
    }
    return copy;
}

SvgRendererGroup *SvgRendererGroup::newShared() const {
    auto copy = new SvgRendererGroup;
    for (auto it = renderers_.keyValueBegin(); it != renderers_.keyValueEnd(); it++) {
        int z = it->first;
        auto *renderer = it->second;
        copy->insert(z, renderer->newShared());
    }
    return copy;
}

void SvgRendererGroup::load(
        std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs) {
    for (const auto &p: contents_pairs) {
        int z = p.first;
        std::visit([this, z](auto &&arg) {
            insertNew(z, arg);
        }, p.second);
    }
}

void SvgRendererGroup::load(std::initializer_list<std::pair<int, QString>> filenames) {
    for (const auto &[z, filename]: filenames)
        insertNew(z, filename);
}

void SvgRendererGroup::load(const QMap<int, QString> &filenames_map) {
    for (auto it = filenames_map.keyValueBegin(); it != filenames_map.keyValueEnd(); ++it)
        insertNew((*it).first, (*it).second);
}

void SvgRendererGroup::load(std::initializer_list<std::pair<int, QByteArray>> contents_pairs) {
    for (const auto &[z, contents]: contents_pairs)
        insertNew(z, contents);
}

void SvgRendererGroup::load(const QMap<int, QByteArray> &contents_map) {
    for (auto it = contents_map.keyValueBegin(); it != contents_map.keyValueEnd(); ++it)
        insertNew((*it).first, (*it).second);
}

void SvgRendererGroup::render(QPainter *p) {
    if (!isVisible())
        return;
    for (const auto renderer: renderers_) {
        renderer->render(p);
    }
}

void SvgRendererGroup::render(QPainter *p, const QRectF &bounds) {
    if (!isVisible())
        return;
    for (const auto renderer: renderers_) {
        renderer->render(p, bounds);
    }
}

void SvgRendererGroup::render(QPainter *p, const QString &elementId, const QRectF &bounds) {
    if (!isVisible())
        return;
    for (const auto renderer: renderers_) {
        renderer->render(p, elementId, bounds);
    }
}
