#pragma once

#include "SvgWidget"
#include "SvgRendererGroup"

class SvgGroupWidget : public SvgWidget {
Q_OBJECT
    SvgRendererGroup *renderers_;

public:
    explicit SvgGroupWidget(QWidget *parent = nullptr);

    SvgGroupWidget(std::initializer_list<QString> filenames, QWidget *parent = nullptr);

    SvgGroupWidget(
            std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs,
            QWidget *parent = nullptr);

    ~SvgGroupWidget() override;

    void setSharedRendererGroup(SvgRendererGroup *group);

    [[nodiscard]] SvgRendererGroup *rendererGroup() const;

public slots:

    void load(
            std::initializer_list<std::pair<int, std::variant<QString, QByteArray, QXmlStreamReader *>>> contents_pairs);

    void load(std::initializer_list<std::pair<int, QString>> filenames);

    void load(const QMap<int, QString> &filenames_map);

    void load(std::initializer_list<std::pair<int, QByteArray>> contents_pairs);

    void load(const QMap<int, QByteArray> &contents_map);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    SvgRenderer *renderer() const;
};
