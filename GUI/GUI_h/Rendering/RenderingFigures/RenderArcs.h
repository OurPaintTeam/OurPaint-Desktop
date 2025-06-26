#ifndef OURPAINT_DRAWINGARC_H
#define OURPAINT_DRAWINGARC_H

#include "RenderBaseFigures.h"

#include <QPainter>
#include <QPointF>
#include <QPainterPath>

#include "ID.h"
#include "RenderStyle.h"
#include "GeometricObjects.h"
#include "Colors.h"
#include "Scaling.h"

// --- Arcs --- //
namespace render::util{
    inline void setGradientColor(QRadialGradient& radialGradient,const qreal stopTransparent1,
                                 const qreal stopTransparent2,const qreal stopCyan, const Color& color);

    void drawID(QPainter& painter,
                const ID& arcID,
                const QPointF& center,
                const qreal startAngleDeg,
                const qreal endAngleDeg,
                const qreal radius,
                const IDLabelStyle& style);

    void drawGlow(QPainter& painter, const QRectF& rect, const qint32 qtStart, const qint32 qtSpan, const GlowStyle& color);


    qreal angleBetween(const QPointF& center, const QPointF& point);

    void setupPen(QPainter& painter, const ArcStyle& style);
}

namespace render {

    struct arcShell {
        Arc* object;
        ArcStyle style;
    };


    void drawFigures(QPainter& painter, const std::unordered_map<ID, arcShell>& arcs);

    void drawFigure(QPainter& painter, const QPointF startPoint, const QPointF endPoint, const QPointF centerPoint,
                    const ArcStyle& style);
}

#endif //OURPAINT_DRAWINGARC_H
