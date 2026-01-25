#ifndef OURPAINT_MAINWINDOWCONTROLLER_H
#define OURPAINT_MAINWINDOWCONTROLLER_H

#include <QObject>
#include <QMainWindow>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QRect>
#include <QPoint>
#include <functional>

class MainWindow;
class MoveDialog;
class Scaling;

class MainWindowController : public QObject {
    Q_OBJECT

public:
    explicit MainWindowController(MainWindow* window);

    void mousePress(QMouseEvent* event);
    void mouseMove(QMouseEvent* event);
    void mouseRelease(QMouseEvent* event);
    void mouseDoubleClick(QMouseEvent* event);

    void wheel(QWheelEvent* event);
    void keyPress(QKeyEvent* event);

    bool handleEvent(QEvent* event);
    bool handleEventFilter(QObject* obj, QEvent* event);
    void handleCloseEvent(QCloseEvent *event);

private:
    void updateShapeCursor(const  QPoint &pos);

private:
    MainWindow* mainWindow = nullptr;

    static constexpr int edgeMargin = 8;

    bool resizing = false;
    bool moving   = false;

    QPoint dragStartPos;
    QRect  originalGeometry;

    enum ResizeRegion {
        None,
        Top, Bottom, Left, Right,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    ResizeRegion currentRegion = None;
};

#endif // OURPAINT_MAINWINDOWCONTROLLER_H
