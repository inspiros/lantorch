#pragma once

#include <QSvgWidget>
#include "SvgRenderer"

class SvgWidget : public QSvgWidget {
Q_OBJECT
    SvgRenderer *renderer_;

public:
    explicit SvgWidget(QWidget *parent = nullptr);

    explicit SvgWidget(const QString &file, QWidget *parent = nullptr);

    ~SvgWidget() override;

    void setSharedRenderer(SvgRenderer *renderer);

    [[nodiscard]] SvgRenderer *renderer() const;

    [[nodiscard]] QSize sizeHint() const override;

public slots:

    void load(const QString &file);

    void load(const QByteArray &contents);

protected:
    void paintEvent(QPaintEvent *event) override;
};
