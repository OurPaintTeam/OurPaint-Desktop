#include "DrawAdditionalInf.h"


void DrawAdditionalInf::drawCoordinateLabels(QPainter &painter,
                                             const QVector<QPointF> &pointsXRight,
                                             const QVector<QPointF> &pointsXLeft,
                                             const QVector<QPointF> &pointsYUp,
                                             const QVector<QPointF> &pointsYDown)
{
    constexpr int FontSize = 8;
    constexpr int MARGIN = 3;

    QFont font = painter.font();
    font.setPointSize(FontSize);
    painter.setFont(font);

    QFontMetrics fontMetrics(font);
    const int TEXT_HEIGHT = fontMetrics.height();
    const int MARGIN_HEIGHT = TEXT_HEIGHT - MARGIN;

    painter.setPen(Qt::black);

    auto drawLabel = [&](const QPointF &point, qreal value, bool below) {
        QPointF textPos = point;
        textPos.setX(point.x() + MARGIN);
        textPos.setY(point.y() + (below ? MARGIN_HEIGHT : -MARGIN));
        painter.drawText(textPos, QString::number(value, 'f', 1));
    };

    // Draw X-axis labels
    for (const QPointF &point : pointsXRight) {
        drawLabel(point, Scaling::logic(point.x()), true);
    }

    for (const QPointF &point : pointsXLeft) {
        drawLabel(point, Scaling::logic(point.x()), true);
    }

    // Draw Y-axis labels
    for (const QPointF &point : pointsYDown) {
        drawLabel(point, Scaling::logic(point.y()), false);
    }

    for (const QPointF &point : pointsYUp) {
        drawLabel(point, Scaling::logic(point.y()), false);
    }
}


[[maybe_unused]] void DrawAdditionalInf::drawAxes(QPainter &painter) {

    // Centering
    const QPointF centerMonitor =   Scaling::getCenteredCoordinates();

    // Offset
    const QPointF delta = Scaling::getDelta();

    // Logical coordinates
    const qreal logicRightX = Scaling::logic(centerMonitor.x()) - Scaling::logic(delta.x());
    const qreal logicUpperY = Scaling::logic(centerMonitor.y()) + Scaling::logic(delta.y());
    const qreal logicLeftX = -Scaling::logic(centerMonitor.x()) - Scaling::logic(delta.x());
    const qreal logicLowerY = -Scaling::logic(centerMonitor.y()) + Scaling::logic(delta.y());

    // Window dimensions
    const QPointF windowSize = Scaling::getActualMonitorSize();

    // Font metrics
    QFontMetrics metrics(painter.font());
    const int textHeight = metrics.height();

    constexpr int MARGIN = 5;

    // Check axis visibility
    bool xAxisVisible = (centerMonitor.y() + delta.y() >= 0) && (centerMonitor.y() + delta.y() <= windowSize.y());
    bool yAxisVisible = (centerMonitor.x() + delta.x() >= 0) && (centerMonitor.x() + delta.x() <= windowSize.x());

    auto drawTextCorner = [&](const QString& text, int x, int y) {
        painter.drawText(x, y, text);
    };

    // Draw left X value
    if (logicLeftX < 0 && xAxisVisible) {
        QString text = QString::number(logicLeftX);
        int x = MARGIN;
        int y = static_cast<int>(centerMonitor.y()) + delta.y() - MARGIN;
        drawTextCorner(text, x, y);
    } else if (!xAxisVisible && std::abs(logicLeftX) > logicRightX) {
        QString text = QString::number(logicLeftX);
        int y = (std::abs(logicUpperY) > std::abs(logicLowerY)) ? (windowSize.y() - MARGIN) : textHeight;
        drawTextCorner(text, MARGIN, y);
    }

    // Draw right X value
    if (logicRightX > 0 && xAxisVisible) {
        QString text = QString::number(logicRightX);
        int textWidth = metrics.horizontalAdvance(text);
        int x = windowSize.y() - textWidth - MARGIN;
        int y = static_cast<int>(centerMonitor.y()) + delta.y() - MARGIN;
        drawTextCorner(text, x, y);
    } else if (!xAxisVisible && std::abs(logicRightX) > std::abs(logicLeftX)) {
        QString text = QString::number(logicRightX);
        int textWidth = metrics.horizontalAdvance(text);
        int x = windowSize.x() - textWidth - MARGIN;
        int y = (std::abs(logicUpperY) > std::abs(logicLowerY)) ? (windowSize.y() - MARGIN) : textHeight;
        drawTextCorner(text, x, y);
    }

    // Draw upper Y value
    if (logicUpperY > 0 && yAxisVisible) {
        QString text = QString::number(logicUpperY);
        int x = static_cast<int>(centerMonitor.x()) + delta.x() + MARGIN;
        drawTextCorner(text, x, textHeight);
    } else if (!yAxisVisible && std::abs(logicUpperY) > std::abs(logicLowerY)) {
        QString text = QString::number(logicUpperY);
        int x = (std::abs(logicRightX) > std::abs(logicLeftX)) ? MARGIN : (windowSize.x() - metrics.horizontalAdvance(text) - MARGIN);
        drawTextCorner(text, x, textHeight);
    }

    // Draw lower Y value
    if (logicLowerY < 0 && yAxisVisible) {
        QString text = QString::number(logicLowerY);
        int x = static_cast<int>(centerMonitor.x()) + delta.x() + MARGIN;
        int y = windowSize.y() - MARGIN;
        drawTextCorner(text, x, y);
    } else if (!yAxisVisible && std::abs(logicLowerY) > std::abs(logicUpperY)) {
        QString text = QString::number(logicLowerY);
        int y = windowSize.y() - MARGIN;
        int x = (std::abs(logicRightX) < std::abs(logicLeftX))
                ? (windowSize.x() - metrics.horizontalAdvance(text) - MARGIN)
                : MARGIN;
        drawTextCorner(text, x, y);
    }
}


void DrawAdditionalInf::drawCursor(QPainter &painter) {
    const QPointF cursor(Scaling::scaleCoordinateX(Scaling::getCursorX()),
                         Scaling::scaleCoordinateY(Scaling::getCursorY()));

    const QPointF logicCursor=Scaling::logicCursor();
    const qint16 RANGE = 5;

    painter.setPen(Qt::black);


    painter.drawText(QPointF(cursor.x() + RANGE, cursor.y() - RANGE),
                     QStringLiteral("X: %1, Y: %2").arg(logicCursor.x(), 0, 'f', 1)
                             .arg(logicCursor.y(), 0, 'f', 1));

}


void DrawAdditionalInf::drawPointID(QPainter &painter,const ID &pointID,const QPointF &point) {

    if (pointID == ID(0)) {
        return;
    }

    const QString idText = QStringLiteral("ID: %1").arg(pointID.get());

    // Offset up above the point
    constexpr qint16 OFFSET_Y = 10;

    const QRectF textRect = painter.fontMetrics().boundingRect(idText);
    const qreal textX = point.x() - textRect.width() / 2;
    const qreal textY = point.y() - OFFSET_Y;

    constexpr qint16 SIZE_PEN = 1;
    painter.setPen(QPen(Qt::black, SIZE_PEN));
    painter.drawText(QPointF(textX, textY), idText);
}


void DrawAdditionalInf::drawSectionID(QPainter &painter,const ID &sectionID,const QPointF &start,const QPointF &end) {
    if (sectionID == ID(0)) {
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
    constexpr qreal FIX_OFFSET = 1.0;
    const qreal rad = qDegreesToRadians(angle - 90.0);

    // Offset by a fixed distance from the line
    const qreal offsetX = FIX_OFFSET * qCos(rad);
    const qreal offsetY = FIX_OFFSET * qSin(rad);

    // Midpoint
    const qreal midX = (start.x() + end.x()) / 2.0;
    const qreal midY = (start.y() + end.y()) / 2.0;

    painter.save();
    painter.translate(midX, midY);
    painter.rotate(angle);

    // Centering horizontally and vertically
    painter.translate(offsetX - textRect.width() / 2, offsetY - textRect.height() / 2);

    painter.drawText(0, 0, idText);
    painter.restore();
}


void DrawAdditionalInf::drawCircleID(QPainter &painter,const ID &circleID,const QPointF &center, qreal radius) {
    if (circleID == ID(0)) {
        return;
    }

    const QString idText = QStringLiteral("ID: %1").arg(circleID.get());

    // Offset above the circle
    const qint16 offsetY = 10;

    const QRectF textRect = painter.fontMetrics().boundingRect(idText);
    const qreal textX = (center.x() - textRect.width() / 2);
    const qreal textY = center.y() - radius - offsetY;
    constexpr qint16 SIZE_PEN = 1;

    painter.save();
    painter.setPen(QPen(Qt::black, SIZE_PEN));
    painter.drawText(QPointF(textX, textY), idText);
    painter.restore();
}


void DrawAdditionalInf::drawArcID(QPainter &painter,
                                  const ID &arcID,
                                  const QPointF &center,
                                  qreal startAngleDeg,
                                  qreal endAngleDeg,
                                  qreal radius)
{
    if (arcID == ID(0)) {
        return;
    }

    QString idText = QStringLiteral("ID: %1").arg(arcID.get());

    // Normalize the angles
    startAngleDeg = fmod(startAngleDeg + 360.0, 360.0);
    endAngleDeg = fmod(endAngleDeg + 360.0, 360.0);

    qreal span = endAngleDeg - startAngleDeg;
    if (span <= 0.1) {
        span += 360.0;
    }

    qreal midAngleDeg = startAngleDeg + span / 2.0;
    midAngleDeg = fmod(midAngleDeg, 360.0);

    const qreal midAngleRad = qDegreesToRadians(midAngleDeg);
    const qreal labelRadius = radius + 10.0;

    const QPointF labelPos(center.x() + labelRadius * std::cos(midAngleRad),
                     center.y() - labelRadius * std::sin(midAngleRad));


    qreal tangentAngle = -midAngleDeg + 90.0;
    qreal normalizedAngle = fmod(tangentAngle + 360.0, 360.0);
    if (normalizedAngle > 90.0 && normalizedAngle < 270.0) {
        tangentAngle += 180.0;
    }

    painter.save();
    painter.translate(labelPos);
    painter.rotate(tangentAngle);

    constexpr qint16 SIZE_PEN = 1;

    painter.setPen(QPen(Qt::black, SIZE_PEN));
    painter.drawText(QPointF(-painter.fontMetrics().horizontalAdvance(idText) / 2.0, 0), idText);

    painter.restore();
}


void DrawAdditionalInf::setPointGradientColor(QRadialGradient &radialGradient, const Color &color) {
    if(colorToString(color)=="Unknown"){
        return;
    }

    QColor baseColor = colorToQColor(color);
    baseColor.setAlpha(120);
    radialGradient.setColorAt(0.0, baseColor);

    baseColor.setAlpha(60);
    radialGradient.setColorAt(0.8, baseColor);

    baseColor.setAlpha(0);
    radialGradient.setColorAt(1.0, baseColor);
}


void DrawAdditionalInf::setSectionGradientColor(QLinearGradient &gradient, const Color &color) {
    if(colorToString(color)=="Unknown"){
        return;
    }

    QColor baseColor = colorToQColor(color);
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


void DrawAdditionalInf::setCircleGradientColor(QRadialGradient &radialGradient,qreal stopTransparent1,qreal stopTransparent2,qreal stopCyan,const Color &color){
    if(colorToString(color)=="Unknown"){
        return;
    }

    QColor baseColor = colorToQColor(color);
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


void DrawAdditionalInf::drawPointGlow(QPainter &painter,const QPointF &point,const Color &color) {
    const qint16 glowRadius = 5;
    QRadialGradient radialGradient(point, glowRadius, point);
    setPointGradientColor(radialGradient,color);

    QBrush glowBrush(radialGradient);
    painter.setBrush(glowBrush);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(point.x() - glowRadius, point.y() - glowRadius), glowRadius * 2, glowRadius * 2);
}


void DrawAdditionalInf::drawSectionGlow(QPainter &painter,const QPointF &start,const QPointF &end,const Color &color) {

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
    constexpr qreal glowDistance = 10.0;

    // Defining the points of the beginning and end of the gradient
    const qreal gradStartX = start.x() + perpX * glowDistance;
    const qreal gradStartY = start.y() + perpY * glowDistance;
    const qreal gradEndX = start.x() - perpX * glowDistance;
    const qreal gradEndY = start.y() - perpY * glowDistance;

    // Creating a linear gradient perpendicular to the line
    QLinearGradient gradient(gradStartX, gradStartY, gradEndX, gradEndY);
    setSectionGradientColor(gradient,color);

    QPen glowPen;
    constexpr qint16 SIZE_PEN = 8;
    glowPen.setWidth(SIZE_PEN);
    glowPen.setBrush(gradient);
    glowPen.setCapStyle(Qt::RoundCap);
    painter.setPen(glowPen);

    painter.drawLine(start.x(), start.y(), end.x(), end.y());
}


void DrawAdditionalInf::drawCircleGlow(QPainter &painter,const QPointF &center,const qreal Radius,const Color &color) {
    constexpr qreal glowDistance = 20.0;
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
    setCircleGradientColor(radialGradient,stopTransparent1,stopTransparent2,stopCyan,color);

    // Setting up the pen for the glow
    constexpr qint16 SIZE_PEN = 8;
    QPen glowPen(QBrush(radialGradient), SIZE_PEN);
    glowPen.setJoinStyle(Qt::RoundJoin);
    glowPen.setCapStyle(Qt::RoundCap);
    painter.setPen(glowPen);
    painter.setBrush(Qt::NoBrush);

    painter.drawEllipse(center, Radius, Radius);
}


void DrawAdditionalInf::drawArcGlow(QPainter& painter,const QRectF &rect,const int qtStart, const int qtSpan,const Color &color) {
    constexpr qreal glowDistance = 20.0;
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
    setCircleGradientColor(radialGradient, stopTransparent1, stopTransparent2, stopCyan, color);

    // A pen with a glow
    constexpr qint16 SIZE_PEN = 8;
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


void DrawAdditionalInf::drawCoordinateLine(QPainter &painter,const QPointF &start,const QPointF &end) {
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
        const qreal offsetX = textRect.width() / 2;
        const qreal offsetY = textRect.height() / 2;

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

        painter.setPen(Qt::black);
        painter.drawText(0, 0, lengthText);
    }

    painter.restore();
}
