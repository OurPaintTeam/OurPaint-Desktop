#ifndef OURPAINT_MOUSEEVENTWORKWINDOW_H
#define OURPAINT_MOUSEEVENTWORKWINDOW_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QTimer>

class Modes;
class Scaling;

class MouseWorkWindow final : public QObject {
private:
    QWidget* m_parent;
    QTimer m_mouseIdleTimer;
    QPoint m_lastMousePos;

    void mousePressEvent(const QMouseEvent* event) const;
    void mouseMoveEvent(const QMouseEvent* event);
    void mouseReleaseEvent(const QMouseEvent* event) const;
    void mouseDoubleClickEvent(const QMouseEvent* event) const;

    static void setMouseTrackingRecursively(QWidget* widget, bool enable);

public:
    explicit MouseWorkWindow(QWidget* parent);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

};

#endif // OURPAINT_MOUSEEVENTWORKWINDOW_H
