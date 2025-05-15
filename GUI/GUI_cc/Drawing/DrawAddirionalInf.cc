#include "DrawAdditionalInf.h"

// Отрисовка значений координaт
void DrawAdditionalInf::drawCoordinateLabels(QPainter &painter,
                                             const std::vector<QPointF> &pointXR,
                                             const std::vector<QPointF> &pointXL,
                                             const std::vector<QPointF> &pointYU,
                                             const std::vector<QPointF> &pointYD) {


    QFont font = painter.font();
    font.setPointSize(8);  // фиксированный размер шрифта
    painter.setFont(font);
    QFontMetrics fontMetrics(font);
    int heightOfText = fontMetrics.height();
    short const  int MARGINS = 3; // Отступ

    painter.setPen(Qt::black);

    // по X влево
    for (const QPointF &point: pointXR) {
        QPointF newPoint = point;
        newPoint.setY(point.y() + heightOfText - MARGINS);
        newPoint.setX(point.x() + MARGINS);
        double value = Scaling::logic(point.x());
        QString text = QString::number(value, 'f', 1);
        painter.drawText(newPoint, text);
    }
    // по Y вниз
    for (const QPointF &point: pointXL) {
        QPointF newPoint = point;
        newPoint.setY(point.y() + heightOfText - MARGINS);
        newPoint.setX(point.x() + MARGINS);
        double value = Scaling::logic(point.x());
        QString text = QString::number(value, 'f', 1);
        painter.drawText(newPoint, text);
    }
    // по Y вверх
    for (const QPointF &point: pointYD) {
        QPointF newPoint = point;
        newPoint.setX(point.x() + MARGINS);
        newPoint.setY(point.y() - MARGINS);
        double value = Scaling::logic(-point.y());
        QString text = QString::number(value, 'f', 1);
        painter.drawText(newPoint, text);
    }
    // по Y вниз
    for (const QPointF &point: pointYU) {
        QPointF newPoint = point;
        newPoint.setX(point.x() + MARGINS);
        newPoint.setY(point.y() - MARGINS);
        double value = Scaling::logic(-point.y());
        QString text = QString::number(value, 'f', 1);
        painter.drawText(newPoint, text);
    }

}

// Рисуем значения координат на осях
[[maybe_unused]] void DrawAdditionalInf::drawAxes(QPainter &painter) {
    // Центрируем
    double _width = Scaling::getCenteredCoordinatesX();
    double _height = Scaling::getCenteredCoordinatesY();

    // Смещение
    double deltaX = Scaling::getDeltaX();
    double deltaY = Scaling::getDeltaY();

    double logicXR = Scaling::logic(_width) - Scaling::logic(deltaX); // х правый
    double logicYU = Scaling::logic(_height) + Scaling::logic(deltaY); // у верхний
    double logicXL = -Scaling::logic(_width) - Scaling::logic(deltaX); // х левый
    double logicYD = -Scaling::logic(_height) + Scaling::logic(deltaY); // у нижний

    // Размеры окна
    short int width = Scaling::getActualMonitorWidth();
    short int height = Scaling::getActualMonitorHeight();

    QFontMetrics metrics(painter.font());
    int textHeight = metrics.height(); // Размер текста
    const short int MARGINS = 5; // Отступы

    // Проверяем видимость осей
    bool xAxisVisible = (_height + deltaY >= 0) && (_height + deltaY <= height);
    bool yAxisVisible = (_width + deltaX >= 0) && (_width + deltaX <= width);

    // Левая координата по X (отрицательная часть)
    if (logicXL < 0 && xAxisVisible) {
        QString textLeftX = QString::number(logicXL);
        int x = MARGINS;
        int y = static_cast<int>(_height) + deltaY - MARGINS;
        painter.drawText(x, y, textLeftX);
    } else if (!xAxisVisible && std::abs(logicXL) > logicXR) {
        QString textCorner = QString::number(logicXL);
        if (std::abs(logicYU) > std::abs(logicYD)) {
            int y = height - MARGINS; // Левый нижний угол (II)
            painter.drawText(MARGINS, y, textCorner);
        } else {
            int y = textHeight; // Левый верхний угол (III)
            painter.drawText(MARGINS, y, textCorner);
        }
    }

    //  Правая координата по X (положительная часть)
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
            int y = height - MARGINS; // Правый нижний угол (I)
            painter.drawText(x, y, textCorner);
        } else {
            int x = width - textWidth - MARGINS;
            int y = textHeight; // Правый верхний угол (IV)
            painter.drawText(x, y, textCorner);
        }
    }

    // Верхняя координата по Y (положительная часть)
    if (logicYU > 0 && yAxisVisible) {
        QString textTopY = QString::number(logicYU);
        int x = static_cast<int>(_width) + deltaX + MARGINS;  // смещаем текст вправо
        int y = textHeight;
        painter.drawText(x, y, textTopY);
    } else if (!yAxisVisible && std::abs(logicYU) > std::abs(logicYD)) {
        QString textCorner = QString::number(logicYU);
        if (std::abs(logicXR) > std::abs(logicXL)) {
            int y = textHeight;
            painter.drawText(MARGINS, y, textCorner); // Левый верхний угол (I)
        } else {
            int textWidth = metrics.horizontalAdvance(textCorner);
            int x = width - textWidth - MARGINS;
            int y = textHeight; // Правый верхний угол (II)
            painter.drawText(x, y, textCorner);
        }
    }

    //  Нижняя координата по Y (отрицательная часть)
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
            int y = height - MARGINS; // Правый нижний угол (III)
            painter.drawText(x, y, textCorner);
        } else {
            int x = MARGINS;
            int y = height - MARGINS; // Левый нижний угол (IV)
            painter.drawText(x, y, textCorner);
        }
    }

}

// Функция отрисовки координат курсора
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

// Отрисовка айди для точки
void DrawAdditionalInf::drawPointID(QPainter &painter, QPointF &point,const ID &pointID) {
    if (pointID != ID(0)) {
        return;
    }
        QString idText = QString("ID: %1").arg(pointID.get());

        // Смещение вверх над точкой
        const short int offsetY = 10;

        QRectF textRect = painter.fontMetrics().boundingRect(idText);
        double textX = point.x() - textRect.width() / 2;
        double textY = point.y() - offsetY;

        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(QPointF(textX, textY), idText);
}

// Отрисовка айди для линии
void DrawAdditionalInf::drawSectionID(QPainter &painter, QPointF &start, QPointF &end,const ID &sectionID) {
    if (sectionID == ID(0)) {
        return;
    }
        // Преобразуем координаты
        qreal dx = start.x() - end.x();
        qreal dy = start.y() - end.y();

        double midX = (start.x() + end.x()) / 2;
        double midY = (start.y() + end.y()) / 2;

        QString idText = QString("ID: %1").arg(sectionID.get());
        QFontMetrics metrics(painter.font());
        QRectF textRect = metrics.boundingRect(idText);

        // Вычисление угла наклона линии
        qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

        // Если линия в обратную сторону, перевернем текст
        if (angle > 90 || angle < -90) {
            angle += 180;
        }

        // Смещение от линии
        const qreal fixedOffset = 1.0;
        qreal perpAngle = angle - 90.0;
        qreal rad = qDegreesToRadians(perpAngle);

        // Смещаем на фиксированное расстояние от линии
        qreal offsetX = fixedOffset * qCos(rad);
        qreal offsetY = fixedOffset * qSin(rad);

        painter.save();
        painter.translate(midX, midY);
        painter.rotate(angle);

        // Центрируем по горизонтали и вертикали
        painter.translate(offsetX - textRect.width() / 2, offsetY - textRect.height() / 2);

        painter.drawText(0, 0, idText);
        painter.restore();
}

// Отрисовка айди для круга
void DrawAdditionalInf::drawCircleID(QPainter &painter, QPointF &center, double r,const ID &circleID) {
    if (circleID == ID(0)) {
        return;
    }
        QString idText = QString("ID: %1").arg(circleID.get());

        // Смещение над кругом
        short int offsetY = 10;

        QRectF textRect = painter.fontMetrics().boundingRect(idText);
        double textX = (center.x() - textRect.width() / 2);
        double textY = center.y() - r - offsetY;

        painter.save();
        painter.setPen(QPen(Qt::black, 1));
        painter.drawText(QPointF(textX, textY), idText);
        painter.restore();
}

// Отрисовка айди для арки
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

    // Нормализуем углы
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
    if (color == Color::blue) {
        radialGradient.setColorAt(0.0, QColor(30, 120, 200, 120));
        radialGradient.setColorAt(0.8, QColor(30, 120, 200, 60));
        radialGradient.setColorAt(1.0, QColor(30, 120, 200, 0));
    } else if (color == Color::purple) {
        radialGradient.setColorAt(0.0, QColor(170, 0, 255, 120));
        radialGradient.setColorAt(0.8, QColor(170, 0, 255, 60));
        radialGradient.setColorAt(1.0, QColor(170, 0, 255, 0));
    } else if (color == Color::cyan) {
    }
}

void DrawAdditionalInf::setSectionGradientColor(QLinearGradient &gradient, const Color &color) {
    if (color == Color::blue) {
        gradient.setColorAt(0.0, QColor(30, 120, 200, 0));
        gradient.setColorAt(0.3, QColor(30, 120, 200, 40));
        gradient.setColorAt(0.5, QColor(30, 120, 200, 80));
        gradient.setColorAt(0.7, QColor(30, 120, 200, 40));
        gradient.setColorAt(1.0, QColor(30, 120, 200, 0));
    } else if (color == Color::purple) {
        gradient.setColorAt(0.0, QColor(170, 0, 255, 0));
        gradient.setColorAt(0.4, QColor(170, 0, 255, 40));
        gradient.setColorAt(0.5, QColor(170, 0, 255, 80));
        gradient.setColorAt(0.6, QColor(170, 0, 255,40));
        gradient.setColorAt(1.0, QColor(170, 0, 255, 0));
    } else if (color == Color::cyan) {
    }
}

void DrawAdditionalInf::setCircleGradientColor(QRadialGradient &radialGradient,qreal stopTransparent1,qreal stopTransparent2,qreal stopCyan,const Color &color){
    if(color==Color::blue){
        radialGradient.setColorAt(0.0,              QColor(30, 120, 200, 0));
        radialGradient.setColorAt(stopTransparent1, QColor(30, 120, 200, 40));
        radialGradient.setColorAt(stopCyan,         QColor(30, 120, 200, 80));
        radialGradient.setColorAt(stopTransparent2, QColor(30, 120, 200, 40));
        radialGradient.setColorAt(1.0,              QColor(30, 120, 200, 0));
    }else if(color==Color::purple){
        radialGradient.setColorAt(0.0,              QColor(170, 0, 255, 0));
        radialGradient.setColorAt(stopTransparent1, QColor(170, 0, 255, 40));
        radialGradient.setColorAt(stopCyan,         QColor(170, 0, 255, 80));
        radialGradient.setColorAt(stopTransparent2, QColor(170, 0, 255, 40));
        radialGradient.setColorAt(1.0,              QColor(170, 0, 255, 0));
    }else if (color == Color::cyan) {
    }
}

// Отрисовка свечения для точки
void DrawAdditionalInf::drawPointGlow(QPainter &painter, QPointF &point,const Color &color) {
    const short int glowRadius = 5;
    QRadialGradient radialGradient(point, glowRadius, point);
    setPointGradientColor(radialGradient,color);

    QBrush glowBrush(radialGradient);
    painter.setBrush(glowBrush);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(point.x() - glowRadius, point.y() - glowRadius), glowRadius * 2, glowRadius * 2);
}

// Отрисовка свечения для линии
void DrawAdditionalInf::drawSectionGlow(QPainter &painter, QPointF &start, QPointF &end,const Color &color) {

    // Вычисляем направление линии
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    qreal length = qSqrt(dx * dx + dy * dy);

    if (length == 0) {
        length = 1;
    }

    // Перпендикулярный вектор
    qreal perpX = -dy / length;
    qreal perpY = dx / length;

    // Задаем расстояние для градиента
    const qreal glowDistance = 10.0;

    // Определяем точки начала и конца градиента
    qreal gradStartX = start.x() + perpX * glowDistance;
    qreal gradStartY = start.y() + perpY * glowDistance;
    qreal gradEndX = start.x() - perpX * glowDistance;
    qreal gradEndY = start.y() - perpY * glowDistance;

    // Создаём линейный градиент перпендикулярный линии
    QLinearGradient gradient(gradStartX, gradStartY, gradEndX, gradEndY);
    setSectionGradientColor(gradient,color);

    QPen glowPen;
    glowPen.setWidth(8);
    glowPen.setBrush(gradient);
    glowPen.setCapStyle(Qt::RoundCap);
    painter.setPen(glowPen);

    painter.drawLine(start.x(), start.y(), end.x(), end.y());
}

// Отрисовка свечения для круга
void DrawAdditionalInf::drawCircleGlow(QPainter &painter, QPointF &center, double Radius,const Color &color) {
    const qreal glowDistance = 20.0;
    qreal gradientRadius = Radius + glowDistance;

    // Вычисляем относительные позиции для цветовых остановок
    qreal stopTransparent1 = (Radius - glowDistance) / gradientRadius;
    qreal stopCyan = Radius / gradientRadius;
    qreal stopTransparent2 = (Radius + glowDistance) / gradientRadius;

    stopTransparent1 = qBound(0.0, stopTransparent1, 1.0);
    stopCyan = qBound(0.0, stopCyan, 1.0);
    stopTransparent2 = qBound(0.0, stopTransparent2, 1.0);

    // Создаём радиальный градиент
    QRadialGradient radialGradient(center, gradientRadius, center);
    setCircleGradientColor(radialGradient,stopTransparent1,stopTransparent2,stopCyan,color);

    // Настраиваем перо для свечения
    QPen glowPen(QBrush(radialGradient), 8);
    glowPen.setJoinStyle(Qt::RoundJoin);
    glowPen.setCapStyle(Qt::RoundCap);
    painter.setPen(glowPen);
    painter.setBrush(Qt::NoBrush);

    painter.drawEllipse(center, Radius, Radius);
}

// Отрисовка свечения для арки
void DrawAdditionalInf::drawArcGlow(QPainter& painter, QRectF &rect, int qtStart, int qtSpan,const Color &color) {
    const qreal glowDistance = 20.0;
    const qreal radius = rect.width() / 2.0;
    QPointF center = rect.center();
    qreal gradientRadius = radius + glowDistance;

    // Позиции остановок цвета
    qreal stopTransparent1 = (radius - glowDistance) / gradientRadius;
    qreal stopCyan = radius / gradientRadius;
    qreal stopTransparent2 = (radius + glowDistance) / gradientRadius;

    stopTransparent1 = qBound(0.0, stopTransparent1, 1.0);
    stopCyan = qBound(0.0, stopCyan, 1.0);
    stopTransparent2 = qBound(0.0, stopTransparent2, 1.0);

    // Радильный градиент
    QRadialGradient radialGradient(center, gradientRadius, center);
    setCircleGradientColor(radialGradient, stopTransparent1, stopTransparent2, stopCyan, color);

    // Перо со свечением
    QPen glowPen(QBrush(radialGradient), 8);
    glowPen.setCapStyle(Qt::RoundCap);
    glowPen.setJoinStyle(Qt::RoundJoin);

    painter.setPen(glowPen);
    painter.setBrush(Qt::NoBrush);

    // Построение дуги
    QPainterPath arcPath;
    arcPath.arcMoveTo(rect, qtStart / 16.0);
    arcPath.arcTo(rect, qtStart / 16.0, qtSpan / 16.0);

    painter.drawPath(arcPath);
}

// Отрисовка длины линии
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

    // Середина линии
    double midX = (start.x() + end.x()) / 2;
    double midY = (start.y() + end.y()) / 2;

    painter.save();

    // Случай вертикальной линии
    if (dx == 0) {
        qreal offsetX = textRect.width() / 2;
        qreal offsetY = textRect.height() / 2;

        painter.translate(midX, midY);
        painter.translate(offsetX, offsetY);
        painter.rotate(-90);  // Поворачиваем текст на 90 градусов
        painter.drawText(0, 0, lengthText);
    } else {
        // Вычисляем угол линии
        qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

        // Проверка на обратное направление
        if (angle > 90 || angle < -90) {
            angle += 180;
        }

        // Смещение вниз от линии
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
