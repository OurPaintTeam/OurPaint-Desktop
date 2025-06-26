#ifndef OURPAINT_DRAWADDITIONALINF_H
#define OURPAINT_DRAWADDITIONALINF_H

#include <QPainter>
#include <QPainterPath>

#include "ID.h"
#include "Scaling.h"
#include "Colors.h"
#include "RenderStyle.h"

// Class for drawing additional information (outlines, numbers)

class DrawAdditionalInf {

public:
    explicit DrawAdditionalInf() = default;

    // The function of drawing values on axes
    static void drawCoordinateLabels(QPainter& painter,
                                     const QVector<QPointF>& pointXR,
                                     const QVector<QPointF>& pointXL,
                                     const QVector<QPointF>& pointYU,
                                     const QVector<QPointF>& pointYD);

    // Drawing the coordinate values on the axes
    [[maybe_unused]] static void drawAxes(QPainter& painter);

    // Cursor coordinates rendering function
    static void drawCursor(QPainter& painter);

    /************* ID ********************/

    static inline bool isValidID(const ID& id);

    static void drawPointID(QPainter& painter, const ID& pointID, const QPointF& point);
    static void drawSectionID(QPainter& painter, const ID& sectionID, const QPointF& start, const QPointF& end);
    static void drawCircleID(QPainter& painter, const ID& circleID, const QPointF& center, qreal radius);
    static void
    drawArcID(QPainter& painter, const ID& arcID, const QPointF& center, qreal startAngleDeg, qreal endAngleDeg,
              qreal radius);


    /************* Glow ********************/
    static void drawPointGlow(QPainter& painter, const QPointF& point, const Color& color);
    static void drawPointGlow(QPainter& painter, const QPointF& point, const PointStyle& style);
    static void drawSectionGlow(QPainter& painter, const QPointF& start, const QPointF& end, const Color& color);
    static void drawCircleGlow(QPainter& painter, const QPointF& center, const qreal radius, const Color& color);
    static void drawArcGlow(QPainter& painter, const QRectF& rect, const int qtStart, int qtSpan, const Color& color);


    /****************** Colors *********************/
    static void setPointGradientColor(QRadialGradient& radialGradient, const Color& color);
    static void setSectionGradientColor(QLinearGradient& radialGradient, const Color& color);
    static void setCircleGradientColor(QRadialGradient& radialGradient,const qreal stopTransparent1,
                                       const qreal stopTransparent2,const qreal stopCyan, const Color& color);


    /****************** Length *********************/

    // Drawing the line length
    static void drawCoordinateLine(QPainter& painter, const QPointF& start, const QPointF& end);
};

#endif //OURPAINT_DRAWADDITIONALINF_H

