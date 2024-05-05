#pragma once

#include <variant>

#include <QMultiMap>
#include "SvgRenderer"

class SvgRendererGroup : public QObject {
Q_OBJECT
    using renderers_iterator = QMultiMap<int, SvgRenderer *>::iterator;
    using renderers_const_iterator = QMultiMap<int, SvgRenderer *>::const_iterator;

    Qt::AspectRatioMode aspect_ratio_mode_ = Qt::IgnoreAspectRatio;
    bool visible_ = true;
    QMultiMap<int, SvgRenderer *> renderers_;
    QMap<SvgRenderer *, renderers_iterator> iterators_;

public:
    explicit SvgRendererGroup(QObject *parent = nullptr);

    SvgRendererGroup(
            std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs,
            QObject *parent = nullptr);

    SvgRendererGroup(std::initializer_list<std::pair<int, QString>> filenames, QObject *parent = nullptr);

    explicit SvgRendererGroup(const QMap<int, QString> &filenames_map, QObject *parent = nullptr);

    SvgRendererGroup(std::initializer_list<std::pair<int, QByteArray>> contents_pairs, QObject *parent = nullptr);

    explicit SvgRendererGroup(const QMap<int, QByteArray> &contents_map, QObject *parent = nullptr);

    SvgRendererGroup(std::initializer_list<std::pair<int, QXmlStreamReader *>> contents_pairs,
                     QObject *parent = nullptr);

    explicit SvgRendererGroup(const QMap<int, QXmlStreamReader *> &contents_map, QObject *parent = nullptr);

    ~SvgRendererGroup() override;

    renderers_iterator insert(int z, SvgRenderer *renderer);

    renderers_iterator insert(renderers_const_iterator pos, int z, SvgRenderer *renderer);

    renderers_iterator insertNew(int z, const QString &filename);

    renderers_iterator insertNew(renderers_const_iterator pos, int z, const QString &filename);

    renderers_iterator insertNew(int z, const QByteArray &contents);

    renderers_iterator insertNew(renderers_const_iterator pos, int z, const QByteArray &contents);

    renderers_iterator insertNew(int z, QXmlStreamReader *contents);

    renderers_iterator insertNew(renderers_const_iterator pos, int z, QXmlStreamReader *contents);

    SvgRendererGroup::renderers_iterator remove(SvgRenderer *renderer);

    [[nodiscard]] QMultiMap<int, SvgRenderer *> renderers() const;

    [[nodiscard]] QMap<SvgRenderer *, renderers_iterator> renderer_iterators() const;

    [[nodiscard]] Qt::AspectRatioMode aspectRatioMode() const;

    void setAspectRatioMode(Qt::AspectRatioMode mode);

    [[nodiscard]] bool isVisible() const;

    void setVisible(bool visible);

    void show();

    void hide();

    [[nodiscard]] QSize defaultSize() const;

    [[nodiscard]] SvgRendererGroup *clone() const;

    [[nodiscard]] SvgRendererGroup *cloneShared() const;

    [[nodiscard]] SvgRendererGroup *newShared() const;

public slots:

    void load(
            std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs);

    void load(std::initializer_list<std::pair<int, QString>> filenames);

    void load(const QMap<int, QString> &filenames_map);

    void load(std::initializer_list<std::pair<int, QByteArray>> contents_pairs);

    void load(const QMap<int, QByteArray> &contents_map);

    void render(QPainter *p);

    void render(QPainter *p, const QRectF &bounds);

    void render(QPainter *p, const QString &elementId, const QRectF &bounds = QRectF());

signals:

    void repaintNeeded();
};
