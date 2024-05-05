#pragma once

#include <QSvgRenderer>
#include <QDomDocument>

class SvgRenderer : public QObject {
Q_OBJECT
    QSvgRenderer *base_renderer_ = nullptr;
    QByteArray svg_contents_;
    bool visible_ = true;

public:
    explicit SvgRenderer(QObject *parent = nullptr);

    explicit SvgRenderer(QSvgRenderer *base_renderer, QObject *parent = nullptr);

    explicit SvgRenderer(const QString &filename, QObject *parent = nullptr);

    explicit SvgRenderer(const QByteArray &contents, QObject *parent = nullptr);

    explicit SvgRenderer(QXmlStreamReader *contents, QObject *parent = nullptr);

    [[nodiscard]] QSvgRenderer *baseRenderer() const;

    [[nodiscard]] QByteArray contents() const;

    [[nodiscard]] bool isVisible() const;

    void setVisible(bool visible);

    void show();

    void hide();

    [[nodiscard]] bool isValid() const;

    [[nodiscard]] QSize defaultSize() const;

    [[nodiscard]] QRect viewBox() const;

    [[nodiscard]] QRectF viewBoxF() const;

    void setViewBox(const QRect &viewbox);

    void setViewBox(const QRectF &viewbox);

    [[nodiscard]] Qt::AspectRatioMode aspectRatioMode() const;

    void setAspectRatioMode(Qt::AspectRatioMode mode);

    [[nodiscard]] bool animated() const;

    [[nodiscard]] int framesPerSecond() const;

    void setFramesPerSecond(int num);

    [[nodiscard]] int currentFrame() const;

    void setCurrentFrame(int frame);

    [[nodiscard]] int animationDuration() const;//in seconds

    [[nodiscard]] QRectF boundsOnElement(const QString &id) const;

    [[nodiscard]] bool elementExists(const QString &id) const;

    [[nodiscard]] QTransform transformForElement(const QString &id) const;

    [[nodiscard]] SvgRenderer *clone() const;

    [[nodiscard]] SvgRenderer *cloneShared() const;

    [[nodiscard]] SvgRenderer *newShared() const;

public slots:

    void setSharedRenderer(QSvgRenderer *base_renderer);

    bool load(const QString &filename);

    bool load(const QByteArray &contents);

    bool load(QXmlStreamReader *contents);

    void render(QPainter *p);

    void render(QPainter *p, const QRectF &bounds);

    void render(QPainter *p, const QString &elementId, const QRectF &bounds = QRectF());

signals:
    void repaintNeeded();

private:
    void setContents(const QByteArray &contents);
};
