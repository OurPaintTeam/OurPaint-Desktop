#ifndef OURPAINT_HEADERS_PAINTERS_QTPAINTER_H_
#define OURPAINT_HEADERS_PAINTERS_QTPAINTER_H_

#include <QFrame>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>
#include <vector>

#include "DrawMode.h"
#include "objects/GeometricObjects.h"
#include "Paint.h"
#include "Scaling.h"
#include "mainwindow.h"

class QTPainter : public QFrame, public Painter {
Q_OBJECT

private:
    Ui::MainWindow *ui;
    std::vector<Point> points;
    std::vector<Circle> circles;
    std::vector<Section> sections;
    std::vector<Point> ReqieredPoints;
    std::vector<Circle> ReqieredCircles;
    std::vector<Section> ReqieredSections;
    std::vector<ID> vec_id;
    Scaling scaling; // Масштабирование

    ID id; // Айди фигуры
    const int CellSize;    // Изначальный размер клетки при отрисовке
    const int maxCellSize; // Максимальный размер клетки при масштабировании
    const int minCellSize; // Минимальный размер клетки при масштабировании
    int currentCellSize;   // Текущий размер клетки
    int cursorX; // Координаты курсора
    int currentCursorX;
    int currentCursorY;
    int cursorY;
    int tab;
    bool CellView;         // Флаг отрисовки сетки
    bool editor; // Обычный режим
    bool Circle_;
    bool Section_;
    bool Point_;
    bool Drawing; // Режим отрисовки за курсором после одного клика
    bool leftClick;
    bool leftDoubleClick;
    bool Shift;
    bool Moving; // Режим перемещения
    bool ReleaseLeftClick; // Отпускание левой кноки
    QPoint centerPoint; // Координаты центра круга
    QPoint perimeterPoint; // Для круга
    QPoint sectionStartPoint;
    QPoint sectionEndPoint;
    QPointF closestPoint; // Крайние точки
    QPointF closestPointNext;



public:
    void drawExp(QPainter &painter);
    QTPainter(Ui::MainWindow *ui, QWidget *parent);

    QPointF MouseCoordinate();

    ID getIdFigures() { return id; }

    std::vector<ID> getVecID() { return vec_id; }

    void setCircle(bool T) { Circle_ = T; }

    void setSection(bool T) { Section_ = T; }

    void setPoint(bool T) { Point_ = T; }

    void setMoving(bool T) { Moving = T; }

    void setMode(DrawMode mode);

    void setEditor(bool T) { editor = T; }

    void setZoomPlus();

    void setZoomMinus();

    void setZoomZero();

    bool getDoubleClick() { return leftDoubleClick; }

    void getUsers(bool var) { scaling.getUsers(var); }

    bool moving(double x, double y); // Проверяем координаты фигуры с координатами курсора
    bool moving(double x0, double y0, double r);

    bool moving(double x1, double y1, double x2, double y2);

    double distancePointToSection(double px, double py, double x0, double y0, double x1,
                                  double y1); // Вспомогательная функция

    // Функция включения сетки
    void setCell(bool On_Off);

    void draw();

    void clear(); // Очистка

    void drawBackground(QPainter &painter); // Отрисовка фона
    void drawPoints(QPainter &painter); // Отрисовка фигур
    void drawCircles(QPainter &painter); // Отрисовка фигур
    void drawSections(QPainter &painter); // Отрисовка фигур
    void drawHints(QPainter &painter);
    void drawMouse(QPainter &painter); // Отрисовка мышью
    void drawCoordinateLine(QPainter &painter, double BegX, double BegY, double EndX, double EndY);

    std::vector<double> FindMaxMin(); // Поиск крайних точек

    QPointF findPoint(); // Ближайшие точки к курсору


protected:

    void setIdFigures(ID Id); // Присвоение айди выделенной фигуры

    void resizeEvent(QResizeEvent *event)
    override;

    void drawPoint(Point pt, bool isWhite = false)
    override;

    void drawCircle(Circle c, bool isWhite = false)
    override;

    void drawSection(Section sec, bool isWhite = false)
    override;

    void paintEvent(QPaintEvent *event)
    override; // Отрисовка

    void changeSize(const BoundBox2D &allObjects)
    override;

    unsigned long long getWeight()
    override;

    unsigned long long getHeight()
    override;

    void mousePressEvent(QMouseEvent *event)
    override;

    void mouseMoveEvent(QMouseEvent *event)
    override;

    void mouseReleaseEvent(QMouseEvent *event)
    override;

    void mouseDoubleClickEvent(QMouseEvent *event)
    override;

    void keyPressEvent(QKeyEvent *event)
    override;

    void keyReleaseEvent(QKeyEvent *event)
    override;
    
signals:

    void RightPress(); // Нажатие правой кнопки мыши

    // Сигналы для отрисовки мышкой
    void SigPoint(QPoint Position);

    void SigCircle(QPoint centerPoint, int radius);

    void SigSection(double startX, double startY, double endX, double endY);

    // Сигналы для перемещения
    void Move(Element F, double x, double y);

    void Move(Element F, double x, double y, double r);

    void Move(Element F, double x, double y, double x1, double y1);

    void MovingFigures();

private slots:

    void onWorkWindowResized();


};


#endif // OURPAINT_HEADERS_PAINTERS_QTPAINTER_H_