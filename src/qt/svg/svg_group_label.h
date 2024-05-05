#pragma once

#include "SvgGroupWidget"

class SvgGroupLabel : public SvgGroupWidget {
Q_OBJECT
public:
    explicit SvgGroupLabel(QWidget *parent = nullptr);

    SvgGroupLabel(std::initializer_list<QString> filenames, QWidget *parent = nullptr);

    SvgGroupLabel(std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs, QWidget *parent = nullptr);

    [[nodiscard]] Qt::AspectRatioMode aspectRatioMode() const;

    void setAspectRatioMode(Qt::AspectRatioMode mode);

    void paintEvent(QPaintEvent *event) override;
};
