#ifndef QTPAINTER_H
#define QTPAINTER_H

#include <QPainter>
#include <QWidget>
#include <QPaintEvent>
#include <QFrame>
#include <vector>
#include "paint.h"
#include "mainwindow.h"
#include "scaling.h"
#include "DrawMode.h"

class QTPainter : public QFrame, public Painter {
Q_OBJECT

private:
    Ui::MainWindow *ui;
    std::vector<point> points;
    std::vector<circle> circles;
    std::vector<section> sections;
    Scaling scaling; // Масштабирование

    unsigned long long id; // Айди фигуры
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
    bool Circle;
    bool Section;
    bool Point;
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
    QPoint closestPoint; // Крайние точки
    QPoint closestPointNext;



public:
    QTPainter(Ui::MainWindow *ui, QWidget *parent);

    QPoint MouseCoordinate(){return QPoint(cursorX-currentCursorX,-cursorY+currentCursorY);}
    void setIdFigures(unsigned long long ID){if(id==0)id=ID;}
    unsigned long long getIdFigures(){  return id;}
    void setCircle(bool T){Circle=T;}
    void setSection(bool T){Section=T;}
    void setPoint(bool T){Point=T;}
    void setMoving(bool T){Moving=T;}
    void setMode(DrawMode mode);
    void setEditor(bool T){editor=T;}
    void setZoomPlus();
    void setZoomMinus();
    void setZoomZero();
    bool getDoubleClick(){return leftDoubleClick;}
    void getUsers(bool var) { scaling.getUsers(var); }

    bool moving(double x,double y); // Проверяем координаты фигуры с координатами курсора
    bool moving(double x0, double y0, double r);
    bool moving(double x1, double y1, double x2,double y2);

    // Функция включения сетки
    void setCell(bool On_Off);

    void draw();

    void clear(); // Очистка

    void drawBackground(QPainter &painter); // Отрисовка фона
    void drawFigures(QPainter &painter); // Отрисовка фигур
    void drawMouse(QPainter &painter); // Отрисовка мышью

    std::vector<double> FindMaxMin(); // Поиск крайних точек

    QPoint findPoint(); // Ближайшие точки к курсору

protected:
    void resizeEvent(QResizeEvent *event) override;

    void drawPoint(point pt, bool isWhite = false) override;

    void drawCircle(circle c, bool isWhite = false) override;

    void drawSection(section sec, bool isWhite = false) override;

    void paintEvent(QPaintEvent *event) override; // Отрисовка

    void changeSize(const rectangle &allObjects) override;

    unsigned long long getWeight() override;
    unsigned long long getHeight() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    bool focusNextPrevChild(bool next) override;

signals:

    void RightPress(); // Нажатие правой кнопки мыши

    // Сигналы для отрисовки мышкой
    void SigPoint(QPoint Position);
    void SigCircle(QPoint centerPoint, int radius);
    void SigSection(int startX, int startY, int endX, int endY);

    // Сигналы для перемещения
    void Move(Element F,double x,double y);
    void Move(Element F,double x,double y,double r);
    void Move(Element F,double x,double y,double x1,double y1);
    void MovingFigures();

private slots:

    void onWorkWindowResized();


};


#endif // QTPAINTER_H