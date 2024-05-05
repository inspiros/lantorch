#pragma once

#include <QWidget>

class OverlayContainerWidget : public QWidget {
Q_OBJECT
public:
    explicit OverlayContainerWidget(QWidget *parent = nullptr);

    void propagateSize(QObject *obj);

protected:
    bool event(QEvent *ev) override;

    void resizeEvent(QResizeEvent *) override;
};
