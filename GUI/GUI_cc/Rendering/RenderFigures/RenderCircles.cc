#include "RenderCircles.h"


namespace render::util{
    void drawID(QPainter& painter, const ID& circleID, const QPointF& center, const qreal radius,const IDLabelStyle& style) {
        if (!isValidID(circleID)) {
            return;
        }

        const QString idText = QStringLiteral("ID: %1").arg(circleID.get());

        // Offset above the circle
        constexpr quint16 offsetY = 10;

        const QRectF textRect = painter.fontMetrics().boundingRect(idText);
        const qreal textX = (center.x() - textRect.width() / 2)+style.label.offset.x();
        const qreal textY = center.y() - radius - offsetY+style.label.offset.y();
        const quint16 SIZE_PEN = +style.label.size;

        painter.save();
        painter.setPen(QPen(ColorToQColor(style.label.color), SIZE_PEN));
        painter.drawText(QPointF(textX, textY), idText);
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


    void
    drawGlow(QPainter& painter, const QPointF& center, const qreal Radius,const GlowStyle& style) {
        constexpr quint16 glowDistance = 20;
        const qreal gradientRadius = Radius + glowDistance;

        // Calculating relative positions for color stops
        qreal stopTransparent1 = (Radius - glowDistance) / gradientRadius;
        qreal stopCyan = Radius / gradientRadius;
        qreal stopTransparent2 = (Radius + glowDistance) / gradientRadius;

        stopTransparent1 = qBound(0.0, stopTransparent1, 1.0);
        stopCyan = qBound(0.0, stopCyan, 1.0);
        stopTransparent2 = qBound(0.0, stopTransparent2, 1.0);

        // Creating a radial gradient
        QRadialGradient radialGradient(center, gradientRadius, center);
        setGradientColor(radialGradient, stopTransparent1, stopTransparent2, stopCyan, style.color);

        // Setting up the pen for the glow
        const quint16 SIZE_PEN = style.size;
        QPen glowPen(QBrush(radialGradient), SIZE_PEN);
        glowPen.setJoinStyle(Qt::RoundJoin);
        glowPen.setCapStyle(Qt::RoundCap);
        painter.setPen(glowPen);
        painter.setBrush(Qt::NoBrush);

        painter.drawEllipse(center, Radius, Radius);
    }

    void setupPen(QPainter& painter, const CircleStyle& style) {
        QPen pen = QPen(ColorToQColor(style.figure.object.color));
        const quint16 SIZE = style.figure.object.size;
        pen.setWidth(SIZE);

        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        painter.setBrush(Qt::NoBrush);

        painter.setPen(pen);
    }

}


namespace render {
    void drawFigures(QPainter& painter, const std::unordered_map<ID, circleShell>& circles) {
        if (circles.empty()) {
            return;
        }

        CircleStyle defaultStyle;

        for (const auto& circlePtr: circles) {

            const Circle* circle = circlePtr.second.object;

            const qreal scaledRadius = Scaling::scaleCoordinate(circle->r);
            const QPointF logicCenter(Scaling::scaleCoordinate(circle->center->x),
                                      Scaling::scaleCoordinate(-circle->center->y));


            const CircleStyle& style = circlePtr.second.style;
            util::setupPen(painter, style);

            painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);

            if (style.figure.glow.activity) {
                const ID& id = circlePtr.first;
                util::drawID(painter, id, logicCenter, scaledRadius, style.figure.id);
            }
            if (style.figure.id.activity) {
                util::drawGlow(painter, logicCenter, scaledRadius, style.figure.glow);
            }
        }

    }


    void drawFigure(QPainter& painter, const QPointF center, const qreal radius, const CircleStyle& style) {
        const qreal scaledRadius = radius * Scaling::getZoom();
        const QPointF logicCenter(Scaling::scaleCoordinate(center.x()), Scaling::scaleCoordinate(-center.y()));

        util::setupPen(painter, style);

        painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);

    }

}

