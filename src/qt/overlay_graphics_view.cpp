#include "overlay_graphics_view.h"

#include <QOpenGLWidget>
#include <QEvent>
#include <QResizeEvent>

OverlayGraphicsView::OverlayGraphicsView(QWidget *parent)
        : QGraphicsView(parent) {
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    QGraphicsView::setStyleSheet("background: transparent");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    newParent();
}

OverlayGraphicsView::OverlayGraphicsView(QGraphicsScene *scene, QWidget *parent)
        : QGraphicsView(scene, parent) {
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    QGraphicsView::setStyleSheet("background: transparent");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    newParent();
}

QObject *OverlayGraphicsView::overlayed_widget() const {
    return parent();
}

void OverlayGraphicsView::update() {
    QGraphicsView::update();
    if (qobject_cast<QOpenGLWidget *>(viewport()))
        viewport()->update();
}

void OverlayGraphicsView::newParent() {
    if (!parent()) return;
    parent()->installEventFilter(this);
    raise();
}

void OverlayGraphicsView::wheelEvent(QWheelEvent *event) {
    event->accept();
}

bool OverlayGraphicsView::eventFilter(QObject *obj, QEvent *event) {
    if (obj == parent()) {
        if (event->type() == QEvent::Resize)
            resize(dynamic_cast<QResizeEvent *>(event)->size());
        else if (event->type() == QEvent::ChildAdded)
            raise();
    }
    return QGraphicsView::eventFilter(obj, event);
}

bool OverlayGraphicsView::event(QEvent *event) {
    if (event->type() == QEvent::ParentAboutToChange && parent())
        parent()->removeEventFilter(this);
    else if (event->type() == QEvent::ParentChange)
        newParent();
    return QGraphicsView::event(event);
}
