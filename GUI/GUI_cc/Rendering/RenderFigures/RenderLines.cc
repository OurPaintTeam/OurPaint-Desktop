#include "RenderLines.h"

namespace render::util{

    void drawCoordinateLine(QPainter& painter, const QPointF& start, const QPointF& end,const LineLengthLabel& style) {
        const qreal dx = start.x() - end.x();
        const qreal dy = start.y() - end.y();
        const qreal length = Scaling::logic(qSqrt(dx * dx + dy * dy));

        const QString lengthText = QString::number(length, 'f', 2);
        const QFontMetrics metrics(painter.font());
        const QRectF textRect = metrics.boundingRect(lengthText);

        if (textRect.width() >= QLineF(start, end).length()) {
            return;
        }

        // Middle of the line
        const qreal midX = (start.x() + end.x()) / 2;
        const qreal midY = (start.y() + end.y()) / 2;

        painter.save();

        // The case of a vertical line
        if (dx == 0) {
            const qreal offsetX = textRect.width() / 2 + style.label.offset.x();
            const qreal offsetY = textRect.height() / 2 + style.label.offset.y();

            painter.translate(midX, midY);
            painter.translate(offsetX, offsetY);
            painter.rotate(-90);  // Rotate the text by 90 degrees
            painter.drawText(0, 0, lengthText);
        } else {
            // Calculating the angle of the line
            qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

            // Check for the opposite direction
            if (angle > 90 || angle < -90) {
                angle += 180;
            }

            // Offset down from the line
            const qreal perpAngle = angle + 90.0;
            const qreal rad = qDegreesToRadians(perpAngle);
            const qreal offsetX = qCos(rad);
            const qreal offsetY = qSin(rad);

            painter.translate(midX, midY);
            painter.rotate(angle);

            painter.translate(offsetX - textRect.width() / 2, offsetY + textRect.height());

            QPen pen(ColorToQColor(style.label.color));
            pen.setWidth(style.label.size);
            painter.setPen(pen);

            painter.drawText(0, 0, lengthText);
        }

        painter.restore();
    }


    void drawGlow(QPainter& painter, const QPointF& start, const QPointF& end,const GlowStyle& style) {

        // Calculating the direction of the line
        const qreal dx = end.x() - start.x();
        const qreal dy = end.y() - start.y();
        qreal length = qSqrt(dx * dx + dy * dy);

        if (length == 0) {
            length = 1;
        }

        // Perpendicular vector
        const qreal perpX = -dy / length;
        const qreal perpY = dx / length;

        // Setting the distance for the gradient
        constexpr quint16 glowDistance = 10;

        // Defining the points of the beginning and end of the gradient
        const qreal gradStartX = start.x() + perpX * glowDistance;
        const qreal gradStartY = start.y() + perpY * glowDistance;
        const qreal gradEndX = start.x() - perpX * glowDistance;
        const qreal gradEndY = start.y() - perpY * glowDistance;

        // Creating a linear gradient perpendicular to the line
        QLinearGradient gradient(gradStartX, gradStartY, gradEndX, gradEndY);
        setGradientColor(gradient, style.color);

        QPen glowPen;
        const quint16 SIZE_PEN = style.size;
        glowPen.setWidth(SIZE_PEN);
        glowPen.setBrush(gradient);
        glowPen.setCapStyle(Qt::RoundCap);
        painter.setPen(glowPen);

        painter.drawLine(start.x(), start.y(), end.x(), end.y());
    }


    inline void setGradientColor(QLinearGradient& gradient, const Color& color) {
        QColor baseColor = colorToGlow(color);
        baseColor.setAlpha(0);
        gradient.setColorAt(0.0, baseColor);
        baseColor.setAlpha(40);
        gradient.setColorAt(0.3, baseColor);
        baseColor.setAlpha(80);
        gradient.setColorAt(0.5, baseColor);
        baseColor.setAlpha(40);
        gradient.setColorAt(0.7, baseColor);
        baseColor.setAlpha(0);
        gradient.setColorAt(1.0, baseColor);
    }

    void drawID(QPainter& painter, const ID& sectionID, const QPointF& start, const QPointF& end,const IDLabelStyle& style) {
        if (!render::isValidID(sectionID)) {
            return;
        }

        // Delta for angle
        const qreal dx = end.x() - start.x();
        const qreal dy = end.y() - start.y();

        // Calculating the slope angle of the line
        qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

        // If the line is in the opposite direction, flip the text
        if (angle > 90 || angle < -90) {
            angle += 180;
        }

        const QString idText = QStringLiteral("ID: %1").arg(sectionID.get());
        const QFontMetrics metrics(painter.font());
        const QRectF textRect = metrics.boundingRect(idText);

        // Offset from the line
        const quint16 FIX_OFFSET = style.label.size;
        const qreal rad = qDegreesToRadians(angle - 90.0);

        // Offset by a fixed distance from the line
        const qreal offsetX = FIX_OFFSET * qCos(rad) + style.label.offset.x();
        const qreal offsetY = FIX_OFFSET * qSin(rad)+ style.label.offset.y();

        // Midpoint
        const qreal midX = (start.x() + end.x()) / 2.0;
        const qreal midY = (start.y() + end.y()) / 2.0;

        painter.save();
        painter.translate(midX, midY);
        painter.rotate(angle);

        // Centering horizontally and vertically
        painter.translate(offsetX - textRect.width() / 2, offsetY - textRect.height() / 2);

        painter.setPen(ColorToQColor(style.label.color));
        painter.drawText(0, 0, idText);
        painter.restore();
    }


    void setupPen(QPainter& painter, const LineStyle& style) {

        QPen currentPen((ColorToQColor(style.figure.object.color)));
        const quint16 SIZE_PEN = style.figure.object.size;
        currentPen.setWidth(SIZE_PEN);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);
    }

}
namespace render {
    void drawFigure(QPainter& painter, const QPointF beg, const QPointF end, const LineStyle& style) {

        const QPointF start(Scaling::scaleCoordinate(beg.x()), Scaling::scaleCoordinate(-beg.y()));
        const QPointF endLogic(Scaling::scaleCoordinate(end.x()), Scaling::scaleCoordinate(-end.y()));

        util::setupPen(painter, style);
        painter.drawLine(start, endLogic);

        util::drawCoordinateLine(painter, start, endLogic, style.lengthLabel);
    }

    void drawFigures(QPainter& painter, const std::unordered_map<ID, lineShell>& sections) {
        if (sections.empty()) {
            return;
        }

        const LineStyle defaultStyle;

        for (const auto& elem: sections) {
            const Section* section = elem.second.object;

            const QPointF start(Scaling::scaleCoordinate(section->beg->x),
                                Scaling::scaleCoordinate(-section->beg->y));

            const QPointF end(Scaling::scaleCoordinate(section->end->x),
                              Scaling::scaleCoordinate(-section->end->y));


            const LineStyle style = elem.second.style;

            util::setupPen(painter, style);
            painter.drawLine(start, end);

            util::drawCoordinateLine(painter, start, end, style.lengthLabel);

            if (style.figure.glow.activity) {
                const ID& id = elem.first;
                util::drawID(painter, id, start, end, style.figure.id);
            }
            if (style.figure.id.activity) {
                util::drawGlow(painter, start, end, style.figure.glow);
            }
        }

    }

}