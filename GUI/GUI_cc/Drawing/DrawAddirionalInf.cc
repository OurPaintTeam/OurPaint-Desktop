#include "DrawAdditionalInf.h"


void DrawAdditionalInf::drawCoordinateLabels(QPainter &painter,
                                             const std::vector<QPointF> &pointXR,
                                             const std::vector<QPointF> &pointXL,
                                             const std::vector<QPointF> &pointYU,
                                             const std::vector<QPointF> &pointYD) {


    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    QFontMetrics fontMetrics(font);
    int heightOfText = fontMetrics.height();
    short const  int MARGINS = 3;

    painter.setPen(Qt::black);

    // by X to the left
    for (const QPointF &point: pointXR) {
        QPointF newPoint = point;
        newPoint.setY(point.y() + heightOfText - MARGINS);
        newPoint.setX(point.x() + MARGINS);
        double value = Scaling::logic(point.x());
        QString text = QString::number(value, 'f', 1);
        painter.drawText(newPoint, text);
    }
    // down Y
    for (const QPointF &point: pointXL) {
        QPointF newPoint = point;
        newPoint.setY(point.y() + heightOfText - MARGINS);
        newPoint.setX(point.x() + MARGINS);
        double value = Scaling::logic(point.x());
        QString text = QString::number(value, 'f', 1);
        painter.drawText(newPoint, text);
    }
    // up Y
    for (const QPointF &point: pointYD) {
        QPointF newPoint = point;
        newPoint.setX(point.x() + MARGINS);
        newPoint.setY(point.y() - MARGINS);
        double value = Scaling::logic(-point.y());
        QString text = QString::number(value, 'f', 1);
        painter.drawText(newPoint, text);
    }
    // down Y
    for (const QPointF &point: pointYU) {
        QPointF newPoint = point;
        newPoint.setX(point.x() + MARGINS);
        newPoint.setY(point.y() - MARGINS);
        double value = Scaling::logic(-point.y());
        QString text = QString::number(value, 'f', 1);
        painter.drawText(newPoint, text);
    }

}


[[maybe_unused]] void DrawAdditionalInf::drawAxes(QPainter &painter) {
    // Centering
    double _width = Scaling::getCenteredCoordinatesX();
    double _height = Scaling::getCenteredCoordinatesY();

    // Offset
    double deltaX = Scaling::getDeltaX();
    double deltaY = Scaling::getDeltaY();

    double logicXR = Scaling::logic(_width) - Scaling::logic(deltaX); // x right
    double logicYU = Scaling::logic(_height) + Scaling::logic(deltaY); // у upper
    double logicXL = -Scaling::logic(_width) - Scaling::logic(deltaX); // х left
    double logicYD = -Scaling::logic(_height) + Scaling::logic(deltaY); // у down

    // Window dimensions
    short int width = Scaling::getActualMonitorWidth();
    short int height = Scaling::getActualMonitorHeight();

    QFontMetrics metrics(painter.font());
    int textHeight = metrics.height(); // Text size
    const short int MARGINS = 5; // Offset

    // Checking the visibility of the axes
    bool xAxisVisible = (_height + deltaY >= 0) && (_height + deltaY <= height);
    bool yAxisVisible = (_width + deltaX >= 0) && (_width + deltaX <= width);

    // Left X-coordinate (negative part)
    if (logicXL < 0 && xAxisVisible) {
        QString textLeftX = QString::number(logicXL);
        int x = MARGINS;
        int y = static_cast<int>(_height) + deltaY - MARGINS;
        painter.drawText(x, y, textLeftX);
    } else if (!xAxisVisible && std::abs(logicXL) > logicXR) {
        QString textCorner = QString::number(logicXL);
        if (std::abs(logicYU) > std::abs(logicYD)) {
            int y = height - MARGINS; // Lower left corner (II)
            painter.drawText(MARGINS, y, textCorner);
        } else {
            int y = textHeight; // Upper left corner (III)
            painter.drawText(MARGINS, y, textCorner);
        }
    }

    // The right X-coordinate (the positive part)
    if (logicXR > 0 && xAxisVisible) {
        QString textRightX = QString::number(logicXR);
        int textWidth = metrics.horizontalAdvance(textRightX);
        int x = width - textWidth - MARGINS;
        int y = static_cast<int>(_height) + deltaY - MARGINS;
        painter.drawText(x, y, textRightX);
    } else if (!xAxisVisible && std::abs(logicXR) > std::abs(logicXL)) {
        QString textCorner = QString::number(logicXR);
        int textWidth = metrics.horizontalAdvance(textCorner);
        if (std::abs(logicYU) > std::abs(logicYD)) {
            int x = width - textWidth - MARGINS;
            int y = height - MARGINS; // Lower right corner (I)
            painter.drawText(x, y, textCorner);
        } else {
            int x = width - textWidth - MARGINS;
            int y = textHeight; // Upper right corner (IV)
            painter.drawText(x, y, textCorner);
        }
    }

    // Upper Y-coordinate (positive part)
    if (logicYU > 0 && yAxisVisible) {
        QString textTopY = QString::number(logicYU);
        int x = static_cast<int>(_width) + deltaX + MARGINS;  // moving the text to the right
        int y = textHeight;
        painter.drawText(x, y, textTopY);
    } else if (!yAxisVisible && std::abs(logicYU) > std::abs(logicYD)) {
        QString textCorner = QString::number(logicYU);
        if (std::abs(logicXR) > std::abs(logicXL)) {
            int y = textHeight;
            painter.drawText(MARGINS, y, textCorner); // Upper left corner (I)
        } else {
            int textWidth = metrics.horizontalAdvance(textCorner);
            int x = width - textWidth - MARGINS;
            int y = textHeight; // Upper right corner (II)
            painter.drawText(x, y, textCorner);
        }
    }

    // Lower Y-coordinate (negative part)
    if (logicYD < 0 && yAxisVisible) {
        QString textBottomY = QString::number(logicYD);
        int x = static_cast<int>(_width) + deltaX + MARGINS;
        int y = height - MARGINS;
        painter.drawText(x, y, textBottomY);
    } else if (!yAxisVisible && std::abs(logicYD) > std::abs(logicYU)) {
        QString textCorner = QString::number(logicYD);
        int textWidth = metrics.horizontalAdvance(textCorner);
        if (std::abs(logicXR) < std::abs(logicXL)) {
            int x = width - textWidth - MARGINS;
            int y = height - MARGINS; // Lower right corner (III)
            painter.drawText(x, y, textCorner);
        } else {
            int x = MARGINS;
            int y = height - MARGINS; // Lower left corner (IV)
            painter.drawText(x, y, textCorner);
        }
    }

}


void DrawAdditionalInf::drawCursor(QPainter &painter) {
    double cursorX = Scaling::scaleCoordinateX(Scaling::getCursorX());
    double cursorY = Scaling::scaleCoordinateY(Scaling::getCursorY());

    painter.setPen(Qt::black);
    double logicalX = Scaling::logicCursorX();
    double logicalY = Scaling::logicCursorY();
    const short int RANGE = 5;

    painter.drawText(QPointF(cursorX + RANGE, cursorY -RANGE),
                     QString("X: %1, Y: %2").arg(logicalX, 0, 'f', 1)
                             .arg(logicalY, 0, 'f', 1));

}


void DrawAdditionalInf::drawPointID(QPainter &painter, QPointF &point,const ID &pointID) {
    if (pointID != ID(0)) {
        return;
    }
        QString idText = QString("ID: %1").arg(pointID.get());

        // Offset up above the point
        const short int offsetY = 10;

        QRectF textRect = painter.fontMetrics().boundingRect(idText);
        double textX = point.x() - textRect.width() / 2;
        double textY = point.y() - offsetY;

        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(QPointF(textX, textY), idText);
}


void DrawAdditionalInf::drawSectionID(QPainter &painter, QPointF &start, QPointF &end,const ID &sectionID) {
    if (sectionID == ID(0)) {
        return;
    }
        // Converting coordinates
        qreal dx = start.x() - end.x();
        qreal dy = start.y() - end.y();

        double midX = (start.x() + end.x()) / 2;
        double midY = (start.y() + end.y()) / 2;

        QString idText = QString("ID: %1").arg(sectionID.get());
        QFontMetrics metrics(painter.font());
        QRectF textRect = metrics.boundingRect(idText);

        // Calculating the slope angle of the line
        qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

        // If the line is in the opposite direction, flip the text
        if (angle > 90 || angle < -90) {
            angle += 180;
        }

        // Offset from the line
        const qreal fixedOffset = 1.0;
        qreal perpAngle = angle - 90.0;
        qreal rad = qDegreesToRadians(perpAngle);

        // Смещаем на фиксированное расстояние от линии
        qreal offsetX = fixedOffset * qCos(rad);
        qreal offsetY = fixedOffset * qSin(rad);

        painter.save();
        painter.translate(midX, midY);
        painter.rotate(angle);

         // Centering horizontally and vertically
        painter.translate(offsetX - textRect.width() / 2, offsetY - textRect.height() / 2);

        painter.drawText(0, 0, idText);
        painter.restore();
}


void DrawAdditionalInf::drawCircleID(QPainter &painter, QPointF &center, double r,const ID &circleID) {
    if (circleID == ID(0)) {
        return;
    }
        QString idText = QString("ID: %1").arg(circleID.get());

        // Offset above the circle
        short int offsetY = 10;

        QRectF textRect = painter.fontMetrics().boundingRect(idText);
        double textX = (center.x() - textRect.width() / 2);
        double textY = center.y() - r - offsetY;

        painter.save();
        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(QPointF(textX, textY), idText);
        painter.restore();
}


void DrawAdditionalInf::drawArcID(QPainter &painter,
                                  const ID &arcID,
                                  const QPointF &center,
                                  double startAngleDeg,
                                  double endAngleDeg,
                                  double radius)
{
    if (arcID == ID(0)) {
        return;
    }

    QString idText = QString("ID: %1").arg(arcID.get());

    // Normalize the angles
    startAngleDeg = fmod(startAngleDeg + 360.0, 360.0);
    endAngleDeg = fmod(endAngleDeg + 360.0, 360.0);

    double span = endAngleDeg - startAngleDeg;
    if (span <= 0.1) {
        span += 360.0;
    }

    double midAngleDeg = startAngleDeg + span / 2.0;
    midAngleDeg = fmod(midAngleDeg, 360.0);
    double midAngleRad = qDegreesToRadians(midAngleDeg);

    double labelRadius = radius + 10.0;
    QPointF labelPos(center.x() + labelRadius * std::cos(midAngleRad),
                     center.y() - labelRadius * std::sin(midAngleRad));


    double tangentAngle = -midAngleDeg + 90.0;
    double normalizedAngle = fmod(tangentAngle + 360.0, 360.0);
    if (normalizedAngle > 90.0 && normalizedAngle < 270.0) {
        tangentAngle += 180.0;
    }

    painter.save();
    painter.translate(labelPos);
    painter.rotate(tangentAngle);

    painter.setPen(QPen(Qt::black, 1));
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


void DrawAdditionalInf::drawPointGlow(QPainter &painter, QPointF &point,const Color &color) {
    const short int glowRadius = 5;
    QRadialGradient radialGradient(point, glowRadius, point);
    setPointGradientColor(radialGradient,color);

    QBrush glowBrush(radialGradient);
    painter.setBrush(glowBrush);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(point.x() - glowRadius, point.y() - glowRadius), glowRadius * 2, glowRadius * 2);
}


void DrawAdditionalInf::drawSectionGlow(QPainter &painter, QPointF &start, QPointF &end,const Color &color) {

    // Calculating the direction of the line
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    qreal length = qSqrt(dx * dx + dy * dy);

    if (length == 0) {
        length = 1;
    }

    // Perpendicular vector
    qreal perpX = -dy / length;
    qreal perpY = dx / length;

    // Setting the distance for the gradient
    const qreal glowDistance = 10.0;

    // Defining the points of the beginning and end of the gradient
    qreal gradStartX = start.x() + perpX * glowDistance;
    qreal gradStartY = start.y() + perpY * glowDistance;
    qreal gradEndX = start.x() - perpX * glowDistance;
    qreal gradEndY = start.y() - perpY * glowDistance;

    // Creating a linear gradient perpendicular to the line
    QLinearGradient gradient(gradStartX, gradStartY, gradEndX, gradEndY);
    setSectionGradientColor(gradient,color);

    QPen glowPen;
    glowPen.setWidth(8);
    glowPen.setBrush(gradient);
    glowPen.setCapStyle(Qt::RoundCap);
    painter.setPen(glowPen);

    painter.drawLine(start.x(), start.y(), end.x(), end.y());
}


void DrawAdditionalInf::drawCircleGlow(QPainter &painter, QPointF &center, double Radius,const Color &color) {
    const qreal glowDistance = 20.0;
    qreal gradientRadius = Radius + glowDistance;

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
    QPen glowPen(QBrush(radialGradient), 8);
    glowPen.setJoinStyle(Qt::RoundJoin);
    glowPen.setCapStyle(Qt::RoundCap);
    painter.setPen(glowPen);
    painter.setBrush(Qt::NoBrush);

    painter.drawEllipse(center, Radius, Radius);
}


void DrawAdditionalInf::drawArcGlow(QPainter& painter, QRectF &rect, int qtStart, int qtSpan,const Color &color) {
    const qreal glowDistance = 20.0;
    const qreal radius = rect.width() / 2.0;
    QPointF center = rect.center();
    qreal gradientRadius = radius + glowDistance;

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
    QPen glowPen(QBrush(radialGradient), 8);
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


void DrawAdditionalInf::drawCoordinateLine(QPainter &painter, QPointF &start, QPointF &end) {
    double dx = start.x() - end.x();
    double dy = start.y() - end.y();

    qreal length = Scaling::logic(qSqrt(dx * dx + dy * dy));

    QString lengthText = QString::number(length, 'f', 2);
    QFontMetrics metrics(painter.font());
    QRectF textRect = metrics.boundingRect(lengthText);

    if (textRect.width() >= QLineF(start, end).length()) {
        return;
    }

    // Middle of the line
    double midX = (start.x() + end.x()) / 2;
    double midY = (start.y() + end.y()) / 2;

    painter.save();

    // The case of a vertical line
    if (dx == 0) {
        qreal offsetX = textRect.width() / 2;
        qreal offsetY = textRect.height() / 2;

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
        qreal perpAngle = angle + 90.0;
        qreal rad = qDegreesToRadians(perpAngle);
        qreal offsetX = qCos(rad);
        qreal offsetY = qSin(rad);

        painter.translate(midX, midY);
        painter.rotate(angle);

        painter.translate(offsetX - textRect.width() / 2, offsetY + textRect.height());

        painter.drawText(0, 0, lengthText);
    }

    painter.restore();
}
