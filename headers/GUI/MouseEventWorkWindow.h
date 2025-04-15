#ifndef OURPAINT_MOUSEEVENTWORKWINDOW_H
#define OURPAINT_MOUSEEVENTWORKWINDOW_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QTimer>

class MouseWorkWindow : public QObject {
private:
    QWidget *m_parent;
    QTimer m_mouseIdleTimer;
    QPoint m_lastMousePos;

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    void mouseDoubleClickEvent(QMouseEvent *event);

    void setMouseTrackingRecursively(QWidget *widget, bool enable);
public:
    explicit MouseWorkWindow(QWidget *parent);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;


};

#endif // OURPAINT_MOUSEEVENTWORKWINDOW_H
