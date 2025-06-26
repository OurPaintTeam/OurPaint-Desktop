#ifndef OURPAINT_DRAWINFCIRCLE_H
#define OURPAINT_DRAWINFCIRCLE_H

#include "RenderBaseFigures.h"
#include "RenderStyle.h"
#include "ID.h"
#include "GeometricObjects.h"
#include "Scaling.h"

// --- Circles --- //
namespace render::util{
    void setGradientColor(QRadialGradient& radialGradient, qreal stopTransparent1,
                          qreal stopTransparent2, qreal stopCyan, const Color& color);

    void drawGlow(QPainter& painter, const QPointF& center, qreal radius, const GlowStyle& style);

    void drawID(QPainter& painter, const ID& circleID, const QPointF& center, const qreal radius,const IDLabelStyle& style);

    void setupPen(QPainter& painter, const CircleStyle& style);
}

namespace render {

    struct circleShell {
        Circle* object;
        CircleStyle style;
    };

     void drawFigures(QPainter& painter, const std::unordered_map<ID, circleShell>& circles);

     void drawFigure(QPainter &painter,const QPointF center,const qreal radius,const CircleStyle& style);

}

#endif //OURPAINT_DRAWINFCIRCLE_H
