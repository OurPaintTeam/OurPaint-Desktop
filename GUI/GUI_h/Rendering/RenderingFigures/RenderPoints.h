#ifndef OURPAINT_DRAWINGPOINT_H
#define OURPAINT_DRAWINGPOINT_H
#include <QPainter>
#include <QPointF>

#include "RenderBaseFigures.h"
#include "RenderStyle.h"
#include "Scaling.h"
#include "Colors.h"
#include "GeometricObjects.h"

namespace render::util {
    QPointF scaleLogicPoint(const Point* pt);
    QPointF scaleLogicPoint(const QPointF& pt);

    void drawID(QPainter& painter, const ID& pointID, const QPointF& point, const IDLabelStyle& idStyle);

    void drawGlow(QPainter& painter, const QPointF& point, const GlowStyle& glowStyle);

    inline void setGradientColor(QRadialGradient& radialGradient, const Color& color);

    struct IDTextData;

    IDTextData prepareIDText(const QFontMetrics& metrics, const ID& pointID, const QPointF& point, const IDLabelStyle& idStyle);

    void setupPenAndBrush(QPainter& painter, const PointStyle& style);
}

namespace render {

    struct pointShell {
        Point* object;
        PointStyle style;
    };

    void drawFigures(QPainter& painter, const std::unordered_map<ID, pointShell>& points);

     void drawFigure(QPainter& painter, const QPointF& point, const PointStyle& style);
};

#endif //OURPAINT_DRAWINGPOINT_H
