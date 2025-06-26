#ifndef OURPAINT_DRAWINGLINE_H
#define OURPAINT_DRAWINGLINE_H

#include <QPainter>
#include <QPointF>

#include "Scaling.h"
#include "RenderBaseFigures.h"
#include "Colors.h"
#include "ID.h"
#include "GeometricObjects.h"
#include "RenderStyle.h"

// --- Sections --- //
namespace render::util{
    inline void setGradientColor(QLinearGradient& gradient, const Color& color);

    void drawGlow(QPainter& painter, const QPointF& start, const QPointF& end,const GlowStyle& style);

    void drawID(QPainter& painter, const ID& sectionID, const QPointF& start, const QPointF& end,const IDLabelStyle& style);

    void drawCoordinateLine(QPainter& painter, const QPointF& start, const QPointF& end,const LineLengthLabel& style);

    void setupPen(QPainter& painter, const LineStyle& style);
}

namespace render {

    struct lineShell {
        Section* object;
        LineStyle style;
    };

     void drawFigures(QPainter& painter, const std::unordered_map<ID, lineShell>& sections);

     void drawFigure(QPainter &painter,const QPointF beg,const QPointF end,const LineStyle& style);

    void drawArrow(QPainter& painter, const QPointF& from, const QPointF& to, qreal arrowSize = 10.0, qreal angleDeg = 15.0);
    void lineWithRightArrow(QPainter& painter, const QPointF& start, const QPointF& end);
    void lineWithLeftArrow(QPainter& painter, const QPointF& start, const QPointF& end);
    void lineWithLeftRightArrow(QPainter& painter, const QPointF& start, const QPointF& end);
}

#endif //OURPAINT_DRAWINGLINE_H
