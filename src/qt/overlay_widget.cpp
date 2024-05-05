#include "overlay_widget.h"

#include <QEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QRandomGenerator>

OverlayWidget::OverlayWidget(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    newParent();
}

void OverlayWidget::newParent() {
    if (!parent()) return;
    parent()->installEventFilter(this);
    raise();
}

QObject *OverlayWidget::overlayed_widget() const {
    return parent();
}

bool OverlayWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == parent()) {
        if (event->type() == QEvent::Resize)
            resize(dynamic_cast<QResizeEvent *>(event)->size());
        else if (event->type() == QEvent::ChildAdded)
            raise();
    }
    return QWidget::eventFilter(obj, event);
}

bool OverlayWidget::event(QEvent *event) {
    if (event->type() == QEvent::ParentAboutToChange && parent())
        parent()->removeEventFilter(this);
    else if (event->type() == QEvent::ParentChange)
        newParent();
    return QWidget::event(event);
}
