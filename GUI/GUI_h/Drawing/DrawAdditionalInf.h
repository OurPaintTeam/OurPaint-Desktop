#ifndef OURPAINT_DRAWADDITIONALINF_H
#define OURPAINT_DRAWADDITIONALINF_H
#include <QPainter>
#include <QPainterPath>

#include "ID.h"
#include "Scaling.h"
#include "Colors.h"

// Class for drawing additional information (outlines, numbers)

class DrawAdditionalInf {

public:
    explicit DrawAdditionalInf() = default;

    // The function of drawing values on axes
    static void drawCoordinateLabels(QPainter &painter,
                                     const std::vector<QPointF> &pointXR,
                                     const std::vector<QPointF> &pointXL,
                                     const std::vector<QPointF> &pointYU,
                                     const std::vector<QPointF> &pointYD);

    // Drawing the coordinate values on the axes
    [[maybe_unused]] static void drawAxes(QPainter &painter);

    // Cursor coordinates rendering function
    static void drawCursor(QPainter &painter) ;

    /************* ID ********************/


    static void drawPointID(QPainter &painter,QPointF &point,const ID& pointID);
    static void drawSectionID(QPainter &painter, QPointF &start, QPointF &end,const ID& sectionID) ;
    static void drawCircleID(QPainter& painter, QPointF &center, double r,const ID& circleID);
    static void drawArcID(QPainter &painter,const ID &arcID,const QPointF &center,double startAngleDeg,double endAngleDeg,double radius);


    /************* Glow ********************/

    static void drawPointGlow(QPainter &painter, QPointF &point,const Color &color) ;
    static void drawSectionGlow(QPainter &painter, QPointF &start,QPointF &end,const Color &color) ;
    static void drawCircleGlow(QPainter& painter, QPointF &center,double Radius,const Color &color);
    static void drawArcGlow(QPainter& painter, QRectF &rect, int qtStart, int qtSpan,const Color &color);


    /****************** Colors *********************/
    static void setPointGradientColor(QRadialGradient &radialGradient,const Color &color);
    static void setSectionGradientColor(QLinearGradient &radialGradient,const Color &color);
    static void setCircleGradientColor(QRadialGradient &radialGradient,qreal stopTransparent1,qreal stopTransparent2,qreal stopCyan,const Color &color);


    /****************** Length *********************/

    // Drawing the line length
    static void drawCoordinateLine(QPainter &painter, QPointF& start,QPointF &end);
};

#endif //OURPAINT_DRAWADDITIONALINF_H

