#ifndef OURPAINT_DRAWADDITIONALINF_H
#define OURPAINT_DRAWADDITIONALINF_H
#include <QPainter>
#include <QPainterPath>

#include "ID.h"
#include "Scaling.h"
#include "Colors.h"

// Класс для отрисовки доп информации (обводки, цифр)

class DrawAdditionalInf {

public:
    explicit DrawAdditionalInf() = default;

    // Функция отрисовки значений на осях
    static void drawCoordinateLabels(QPainter &painter,
                                     const std::vector<QPointF> &pointXR,
                                     const std::vector<QPointF> &pointXL,
                                     const std::vector<QPointF> &pointYU,
                                     const std::vector<QPointF> &pointYD);

    // Рисуем значения координат на осях
    [[maybe_unused]] static void drawAxes(QPainter &painter);

    // Функция отрисовки координат курсора
    static void drawCursor(QPainter &painter) ;

    /************* ID ********************/

    // Отрисовка айди для точки
    static void drawPointID(QPainter &painter,QPointF &point,const ID& pointID);

    // Отрисовка айди для линии
    static void drawSectionID(QPainter &painter, QPointF &start, QPointF &end,const ID& sectionID) ;

    // Отрисовка айди для круга
    static void drawCircleID(QPainter& painter, QPointF &center, double r,const ID& circleID);

    // Отрисовка айди для арки
    static void drawArcID(QPainter &painter,const ID &arcID,const QPointF &center,double startAngleDeg,double endAngleDeg,double radius);


    /************* Свечение ********************/

    // Отрисовка свечения для точки
    static void drawPointGlow(QPainter &painter, QPointF &point,const Color &color) ;

    // Отрисовка свечения для линии
    static void drawSectionGlow(QPainter &painter, QPointF &start,QPointF &end,const Color &color) ;

    // Отрисовка свечения для круга
    static void drawCircleGlow(QPainter& painter, QPointF &center,double Radius,const Color &color);

    // Отрисовка свечения для арки
    static void drawArcGlow(QPainter& painter, QRectF &rect, int qtStart, int qtSpan,const Color &color);


    /****************** Цвета *********************/
    static void setPointGradientColor(QRadialGradient &radialGradient,const Color &color);
    static void setSectionGradientColor(QLinearGradient &radialGradient,const Color &color);
    static void setCircleGradientColor(QRadialGradient &radialGradient,qreal stopTransparent1,qreal stopTransparent2,qreal stopCyan,const Color &color);


    /****************** Длина *********************/

    // Отрисовка длины линии
    static void drawCoordinateLine(QPainter &painter, QPointF& start,QPointF &end);
};

#endif //OURPAINT_DRAWADDITIONALINF_H

