#include "overlay_container_widget.h"

#include <QResizeEvent>

OverlayContainerWidget::OverlayContainerWidget(QWidget *parent) : QWidget(parent) {}

void OverlayContainerWidget::propagateSize(QObject *obj) {
    if (obj->isWidgetType())
        dynamic_cast<QWidget *>(obj)->setGeometry(rect());
}

bool OverlayContainerWidget::event(QEvent *ev) {
    if (ev->type() == QEvent::ChildAdded)
        propagateSize(dynamic_cast<QChildEvent *>(ev)->child());
    return QWidget::event(ev);
}

void OverlayContainerWidget::resizeEvent(QResizeEvent *) {
    for (auto obj: children()) propagateSize(obj);
}
