#include "RenderArcs.h"

namespace render::util {
    qreal angleBetween(const QPointF& center, const QPointF& point) {
        return QLineF(center, point).angle();
    }

    void drawGlow(QPainter& painter, const QRectF& rect, const qint32 qtStart, const qint32 qtSpan,
                  const GlowStyle& style) {
        constexpr quint16 glowDistance = 20;
        const qreal radius = rect.width() / 2.0;
        const QPointF center = rect.center();
        const qreal gradientRadius = radius + glowDistance;

        // Color stop positions
        qreal stopTransparent1 = (radius - glowDistance) / gradientRadius;
        qreal stopCyan = radius / gradientRadius;
        qreal stopTransparent2 = (radius + glowDistance) / gradientRadius;

        stopTransparent1 = qBound(0.0, stopTransparent1, 1.0);
        stopCyan = qBound(0.0, stopCyan, 1.0);
        stopTransparent2 = qBound(0.0, stopTransparent2, 1.0);

        // Radial gradient
        QRadialGradient radialGradient(center, gradientRadius, center);
        setGradientColor(radialGradient, stopTransparent1, stopTransparent2, stopCyan, style.color);

        // A pen with a glow
        const quint16 SIZE_PEN = style.size;
        QPen glowPen(QBrush(radialGradient), SIZE_PEN);
        glowPen.setCapStyle(Qt::RoundCap);
        glowPen.setJoinStyle(Qt::RoundJoin);

        painter.setPen(glowPen);
        painter.setBrush(Qt::NoBrush);

        // Building an arc
        QPainterPath arcPath;
        arcPath.arcMoveTo(rect, qtStart / 16.0);
        arcPath.arcTo(rect, qtStart / 16.0, qtSpan / 16.0);

        painter.drawPath(arcPath);
    }


    void drawID(QPainter& painter,
                const ID& arcID,
                const QPointF& center,
                const qreal startAngleDeg,
                const qreal endAngleDeg,
                const qreal radius,const IDLabelStyle& style) {
        if (!isValidID(arcID)) {
            return;
        }

        QString idText = QStringLiteral("ID: %1").arg(arcID.get());

        // Normalize the angles
        const qreal startAngle = fmod(startAngleDeg + 360.0, 360.0);
        const qreal endAngle = fmod(endAngleDeg + 360.0, 360.0);

        qreal span = endAngle - startAngle;
        constexpr qreal EPS = 0.1;
        if (span <= EPS) {
            span += 360.0;
        }

        qreal midAngleDeg = startAngle + span / 2.0;
        midAngleDeg = fmod(midAngleDeg, 360.0);

        const qreal midAngleRad = qDegreesToRadians(midAngleDeg);
        const qreal labelRadius = radius + 10.0;

        const QPointF labelPos(center.x() + labelRadius * qCos(midAngleRad)+style.label.offset.x(),
                               center.y() - labelRadius * qSin(midAngleRad)+style.label.offset.y());


        qreal tangentAngle = -midAngleDeg + 90.0;
        qreal normalizedAngle = fmod(tangentAngle + 360.0, 360.0);
        if (normalizedAngle > 90.0 && normalizedAngle < 270.0) {
            tangentAngle += 180.0;
        }

        painter.save();
        painter.translate(labelPos);
        painter.rotate(tangentAngle);

        const quint16 SIZE_PEN = style.label.size;
        QPen pen(ColorToQColor(style.label.color));
        pen.setWidth(SIZE_PEN);
        painter.setPen(pen);
        painter.drawText(QPointF(-painter.fontMetrics().horizontalAdvance(idText) / 2.0, 0), idText);

        painter.restore();
    }


    inline void setGradientColor(QRadialGradient& radialGradient, const qreal stopTransparent1,
                          const qreal stopTransparent2, const qreal stopCyan, const Color& color) {
        QColor baseColor = colorToGlow(color);
        baseColor.setAlpha(0);
        radialGradient.setColorAt(0.0, baseColor);
        baseColor.setAlpha(40);
        radialGradient.setColorAt(stopTransparent1, baseColor);
        baseColor.setAlpha(80);
        radialGradient.setColorAt(stopCyan, baseColor);
        baseColor.setAlpha(40);
        radialGradient.setColorAt(stopTransparent2, baseColor);
        baseColor.setAlpha(0);
        radialGradient.setColorAt(1.0, baseColor);
    }

    void setupPen(QPainter& painter, const ArcStyle& style) {
        QPen pen(ColorToQColor(style.figure.object.color));
        const quint16 SIZE_PEN = style.figure.object.size;
        pen.setWidth(SIZE_PEN);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
    }

}

namespace render{
    void drawFigure(QPainter& painter, const QPointF startPoint, const QPointF endPoint, const QPointF centerPoint,
                    const ArcStyle& style) {

        const QPointF center(Scaling::scaleCoordinate(centerPoint.x()), Scaling::scaleCoordinate(-centerPoint.y()));
        const QPointF start(Scaling::scaleCoordinate(startPoint.x()), Scaling::scaleCoordinate(-startPoint.y()));
        const QPointF end(Scaling::scaleCoordinate(endPoint.x()), Scaling::scaleCoordinate(-endPoint.y()));

        const qreal radius = std::hypot(start.x() - center.x(), start.y() - center.y());

        const qreal startAngleDeg = util::angleBetween(center, start);
        const qreal endAngleDeg = util::angleBetween(center, end);
        qreal spanAngleDeg = endAngleDeg - startAngleDeg;

        constexpr qreal EPS = 0.1;
        if (spanAngleDeg <= EPS) {
            spanAngleDeg += 360;
        }

        const QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

        const qint32 qtStart = static_cast<qint32>(startAngleDeg * 16);
        const qint32 qtSpan = static_cast<qint32>(spanAngleDeg * 16);

        util::setupPen(painter, style);
        painter.drawArc(rect, qtStart, qtSpan);
    }


    void drawFigures(QPainter& painter, const std::unordered_map<ID, arcShell>& arcs) {
        if (arcs.size() == 0) {
            return;
        }

        ArcStyle defaultStyle;

        for (const auto& elem: arcs) {

            const Arc* arc = elem.second.object;

            const ArcStyle& style = elem.second.style;
            util::setupPen(painter, style);

            const QPointF center(Scaling::scaleCoordinate(arc->center->x),
                                 Scaling::scaleCoordinate(-arc->center->y));
            const QPointF start(Scaling::scaleCoordinate(arc->beg->x), Scaling::scaleCoordinate(-arc->beg->y));
            const QPointF end(Scaling::scaleCoordinate(arc->end->x), Scaling::scaleCoordinate(-arc->end->y));

            const qreal radius = std::hypot(start.x() - center.x(), start.y() - center.y());

            const qreal startAngleDeg = util::angleBetween(center, start);
            const qreal endAngleDeg = util::angleBetween(center, end);
            qreal spanAngleDeg = endAngleDeg - startAngleDeg;
            constexpr qreal EPS = 0.1;

            if (spanAngleDeg <= EPS) {
                spanAngleDeg += 360;
            }

            const QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

            const qint32 qtStart = static_cast<qint32>(startAngleDeg * 16);
            const qint32 qtSpan = static_cast<qint32>(spanAngleDeg * 16);

            painter.drawArc(rect, qtStart, qtSpan);

            if (style.figure.glow.activity) {
                util::drawGlow(painter, rect, qtStart, qtSpan, style.figure.glow);
            }
            if (style.figure.id.activity) {
                const ID& id = elem.first;
                util::drawID(painter, id, center, startAngleDeg, endAngleDeg, radius, style.figure.id);
            }


        }
    }
}