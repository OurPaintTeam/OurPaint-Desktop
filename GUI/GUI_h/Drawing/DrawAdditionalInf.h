#ifndef OURPAINT_DRAWADDITIONALINF_H
#define OURPAINT_DRAWADDITIONALINF_H
#include <QPainter>
#include "Scaling.h"

// Класс для отрисовки доп информации (обводки, цифр)

class DrawAdditionalInf {
    static unsigned long long id; // айди для вывода
    static unsigned long long LeftMenuID;
public:
    explicit DrawAdditionalInf() = default;

    // Для отрисовки айди
    static void setLeftMenuID(unsigned long long ID);
    static void setID(unsigned long long ID);


    // Функция отрисовки значений на осях
    static void drawCoordinateLabels(QPainter &painter,
                                     const std::vector<QPointF> &pointXR,
                                     const std::vector<QPointF> &pointXL,
                                     const std::vector<QPointF> &pointYU,
                                     const std::vector<QPointF> &pointYD);

    // Рисуем значения координат на осях
    static void drawAxes(QPainter &painter);

    // Функция отрисовки координат курсора
    static void drawCursor(QPainter &painter) ;

    /************* ID ********************/

    // Отрисовка айди для точки
    static void drawPointID(QPainter &painter,QPointF &point,long long int ID);

    // Отрисовка айди для линии
    static void drawSectionID(QPainter &painter, QPointF &start, QPointF &end,long long int ID) ;

    // Отрисовка айди для круга
    static void drawCircleID(QPainter& painter, QPointF &center, double r,long long int ID) ;

    // Отрисовка айди для круга
    static void drawCircleLeftID(QPainter& painter,QPointF &center,double r);

    // Отрисовка айди для линии
    static void drawSectionLeftID(QPainter &painter, QPointF &start,QPointF &end) ;

    // Отрисовка айди для точки
    static void drawPointLeftID(QPainter &painter, QPointF &point) ;

    /************* Свечение ********************/

    // Отрисовка свечения для точки
    static void drawPointGlow(QPainter &painter, QPointF &point) ;

    // Отрисовка свечения для линии
    static void drawSectionGlow(QPainter &painter, QPointF &start,QPointF &end) ;

    // Отрисовка свечения для круга
    static void drawCircleGlow(QPainter& painter, QPointF &center,double Radius);

    /****************** длина *********************/

    // Отрисовка длины линии
    static void drawCoordinateLine(QPainter &painter, QPointF& start,QPointF &end);
};

#endif //OURPAINT_DRAWADDITIONALINF_H

