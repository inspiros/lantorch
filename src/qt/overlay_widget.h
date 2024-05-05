#pragma once

#include <QPointer>
#include <QWidget>
#include <QPainter>

/**
 * Reference: https://stackoverflow.com/questions/19383427/blur-effect-over-a-qwidget-in-app/19386886#19386886
 */
class OverlayWidget : public QWidget {
Q_OBJECT
public:
    explicit OverlayWidget(QWidget *parent = nullptr);

    [[nodiscard]] QObject *overlayed_widget() const;

private:
    void newParent();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

    bool event(QEvent *event) override;
};
