#pragma once

#include "SvgWidget"

class SvgLabel : public SvgWidget {
Q_OBJECT
public:
    explicit SvgLabel(QWidget *parent = nullptr);

    explicit SvgLabel(const QString &file, QWidget *parent = nullptr);

    [[nodiscard]] Qt::AspectRatioMode aspectRatioMode() const;

    void setAspectRatioMode(Qt::AspectRatioMode mode);

protected:
    void paintEvent(QPaintEvent *event) override;
};
