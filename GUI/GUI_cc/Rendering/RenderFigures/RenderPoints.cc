#include "RenderPoints.h"

namespace render::util {


    QPointF scaleLogicPoint(const QPointF& pt) {
        return QPointF(Scaling::scaleCoordinate(pt.x()), Scaling::scaleCoordinate(-pt.y()));
    }


    QPointF scaleLogicPoint(const Point* pt) {
        return scaleLogicPoint(QPointF(pt->x, pt->y));
    }


    inline void setGradientColor(QRadialGradient& radialGradient, const Color& color) {
        QColor baseColor = colorToGlow(color);
        baseColor.setAlpha(120);
        radialGradient.setColorAt(0.0, baseColor);

        baseColor.setAlpha(60);
        radialGradient.setColorAt(0.8, baseColor);

        baseColor.setAlpha(0);
        radialGradient.setColorAt(1.0, baseColor);
    }

    QRadialGradient createGlowGradient(const QPointF& point, const GlowStyle& glowStyle) {
        QRadialGradient gradient(point, glowStyle.size, point);
        QColor baseColor = colorToGlow(glowStyle.color);

        baseColor.setAlpha(120);
        gradient.setColorAt(0.0, baseColor);

        baseColor.setAlpha(60);
        gradient.setColorAt(0.8, baseColor);

        baseColor.setAlpha(0);
        gradient.setColorAt(1.0, baseColor);

        return gradient;
    }


    void drawGlow(QPainter& painter, const QPointF& point, const GlowStyle& glowStyle) {
        QRadialGradient gradient = createGlowGradient(point, glowStyle);
        painter.setBrush(QBrush(gradient));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(point, glowStyle.size, glowStyle.size);
    }


    struct IDTextData {
        QString text;
        QPointF position;
        QPen pen;
    };


    IDTextData prepareIDText(const QFontMetrics& metrics, const ID& pointID, const QPointF& point, const IDLabelStyle& idStyle) {
        if (!render::isValidID(pointID)) {
            return {};
        }

        const QString idText = QStringLiteral("ID: %1").arg(pointID.get());
        constexpr quint16 OFFSET_Y = 10;

        QRectF textRect = metrics.boundingRect(idText);
        QPointF textPos(point.x() - textRect.width() / 2 + idStyle.label.offset.x(),
                        point.y() - OFFSET_Y + idStyle.label.offset.y());

        QPen pen(ColorToQColor(idStyle.label.color));
        pen.setWidth(idStyle.label.size);

        return { idText, textPos, pen };
    }


    void drawID(QPainter& painter, const ID& pointID, const QPointF& point, const IDLabelStyle& idStyle) {
        auto data = prepareIDText(painter.fontMetrics(), pointID, point, idStyle);
        if (data.text.isEmpty()) {
            return;
        }

        painter.setPen(data.pen);
        painter.drawText(data.position, data.text);
    }


   void setupPenAndBrush(QPainter& painter, const PointStyle& style) {
       painter.setBrush(ColorToQColor(style.figure.object.color));
       painter.setPen(Qt::NoPen);
   }


} // namespace render::util


namespace render {

    void drawFigure(QPainter& painter, const QPointF& point, const PointStyle& style) {
        const QPointF logicPoint = util::scaleLogicPoint(point);
        util::setupPenAndBrush(painter, style);
        painter.drawEllipse(logicPoint, style.pointRadius, style.pointRadius);
    }

    void drawFigures(QPainter& painter, const std::unordered_map<ID, pointShell>& points) {
        if (points.empty()) {
            return;
        }

        for (const auto& [id, pointPtr]: points) {
            const QPointF logicPoint = util::scaleLogicPoint(pointPtr.object);

            const PointStyle& style = pointPtr.style;

            util::setupPenAndBrush(painter, style);
            painter.drawEllipse(logicPoint, style.pointRadius, style.pointRadius);

            if (style.figure.glow.activity) {
                util::drawID(painter, id, logicPoint, style.figure.id);
            }
            if (style.figure.id.activity) {
                util::drawGlow(painter, logicPoint, style.figure.glow);
            }
        }
    }

} // namespace render
