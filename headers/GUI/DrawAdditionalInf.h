#ifndef OURPAINT_DRAWADDITIONALINF_H
#define OURPAINT_DRAWADDITIONALINF_H
#include <QPainter>
#include "Scaling.h"

// Класс для отрисовки доп информации (обводки, цифр)

class DrawAdditionalInf {
    static unsigned long long id; // айди для вывода
    static unsigned long long LeftMenuID;
public:

    static void setLeftMenuID(unsigned long long ID) {
        LeftMenuID = ID;
    }

    static void setID(unsigned long long ID) {
        id = ID;
    }

    explicit DrawAdditionalInf() = default;
    static void drawCoordinateLabels(QPainter &painter, const std::vector<QPointF> &pointXR,const std::vector<QPointF> &pointXL,const std::vector<QPointF> &pointYU,const std::vector<QPointF> &pointYD) {

        // Устанавливаем фиксированный размер шрифта для текста
        QFont font = painter.font();
        font.setPointSize(8);  // фиксированный размер шрифта
        painter.setFont(font);
        QFontMetrics fontMetrics(font);
        int heightOfText = fontMetrics.height();
        short const int MARGINS=3;

        // Рисуем текст — координаты остаются трансформированными
        painter.setPen(Qt::black);

        int i = -1;
        for (const QPointF &point: pointXR) {
            QString text;
            QPointF newPoint = point;
            newPoint.setY(point.y() + heightOfText-MARGINS);
            newPoint.setX(point.x() + MARGINS);
            text = QString::number(i-- * 5);
            painter.drawText(newPoint, text);
        }
        int j = 1;
        for (const QPointF &point: pointXL) {
            QString text;
            QPointF newPoint = point;
            newPoint.setY(point.y() + heightOfText-MARGINS);
            newPoint.setX(point.x() + MARGINS);
            text = QString::number(j++ * 5);
            painter.drawText(newPoint, text);
        }
        int k = 1;
        for (const QPointF &point: pointYD) {
            QString text;
            QPointF newPoint = point;
            newPoint.setX(point.x() + MARGINS);
            newPoint.setY(point.y() - MARGINS);
            text = QString::number(k++ * 5);
            painter.drawText(newPoint, text);
        }
        int m = -1;
        for (const QPointF &point: pointYU) {
            QString text;
            QPointF newPoint = point;
            newPoint.setX(point.x() + MARGINS);
            newPoint.setY(point.y() - MARGINS);
            text = QString::number(m-- * 5);
            painter.drawText(newPoint, text);
        }

    }
// Рисуем значения координат на осях
    static void drawAxes(QPainter &painter) {
        // Центрируем
        double _width = Scaling::getCenteredCoordinatesX();
        double _height = Scaling::getCenteredCoordinatesY();

        // Смещение
        int deltaX = Scaling::getDeltaX();
        int deltaY = Scaling::getDeltaY();

        double logicXR = Scaling::logicDouble(_width) - Scaling::logicInt(deltaX); // х правый
        double logicYU = Scaling::logicDouble(_height) + Scaling::logicInt(deltaY); // у верхний
        double logicXL = -Scaling::logicDouble(_width) - Scaling::logicInt(deltaX); // х левый
        double logicYD = -Scaling::logicDouble(_height) + Scaling::logicInt(deltaY); // у нижний

        // Размеры окна
        short int width = Scaling::getDisplayCoordinateX();
        short int height = Scaling::getDisplayCoordinateY();

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
    static void drawCursor(QPainter &painter) {
        double cursorX = Scaling::scaleCoordinateX(Scaling::getCursorX());
        double cursorY = Scaling::scaleCoordinateY(Scaling::getCursorY());

        painter.setPen(Qt::black);
        double logicalX = Scaling::logicCursorX();
        double logicalY = Scaling::logicCursorY();

        painter.drawText((int) cursorX + 5, (int) cursorY - 5,
                         QString("X: %1, Y: %2").arg(logicalX).arg(logicalY));


    }

    /*********************************/
    //      ID


    // Отрисовка айди для точки
    static void drawPointID(QPainter &painter,QPointF &point) {
        if (id != 0) {
            QString idText = QString("ID: %1").arg(id);

            // Смещение вверх над точкой
            double offsetY = 10;

            QRectF textRect = painter.fontMetrics().boundingRect(idText);
            double textX = point.x() - textRect.width() / 2;
            double textY = point.y() - offsetY;

            painter.setPen(QPen(Qt::black, 1));
            painter.drawText( QPointF(textX,  textY), idText);
            id = 0;
        }
    }

    // Отрисовка айди для линии
    static void drawSectionID(QPainter &painter, QPointF &start, QPointF &end) {
        if (id != 0) {
            // Преобразуем координаты
            qreal dx = start.x() - end.x();
            qreal dy = start.y() - end.y();

            double midX = (start.x() + end.x()) / 2;
            double midY = (start.y() + end.y()) / 2;

            QString idText = QString("ID: %1").arg(id);
            QFontMetrics metrics(painter.font());
            QRectF textRect = metrics.boundingRect(idText);

            // Вычисление угла наклона линии
            qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

            // Если линия "в обратную сторону", перевернем текст
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
            painter.translate(offsetX - textRect.width() / 2, offsetY - textRect.height() /2);

            painter.drawText(0, 0, idText);
            painter.restore();

            id = 0;
        }
    }

    // Отрисовка айди для круга
    static void drawCircleID(QPainter& painter, QPointF &center, double r) {
        if (id != 0) {
            // Преобразование

            QString idText = QString("ID: %1").arg(id);

            // Смещение над кругом
            int offsetY =   10;

            QRectF textRect = painter.fontMetrics().boundingRect(idText);
            double textX = (center.x() - textRect.width()/2);
            double textY = center.y()-r - offsetY;

            painter.save();
            painter.setPen(QPen(Qt::black, 1));
            painter.drawText( QPointF(textX,  textY), idText);
            painter.restore();

            id = 0;
        }
    }

    // Отрисовка айди для круга
    static void drawCircleLeftID(QPainter& painter,QPointF &center,double r) {
        if (LeftMenuID != 0) {
            // Преобразование

            QString idText = QString("ID: %1").arg(LeftMenuID);

            // Смещение над кругом
            int offsetY =   10;

            QRectF textRect = painter.fontMetrics().boundingRect(idText);
            double textX = (center.x() - textRect.width()/2);
            double textY = center.y()-r - offsetY;

            painter.save();
            painter.setPen(QPen(Qt::black, 1));
            painter.drawText( QPointF(textX,  textY), idText);
            painter.restore();

        }
    }

    // Отрисовка айди для линии
    static void drawSectionLeftID(QPainter &painter, QPointF &start,QPointF &end) {
        if (LeftMenuID != 0) {
            // Преобразуем координаты

            qreal dx = start.x() - end.x();
            qreal dy = start.y() - start.y();

            double midX = (start.x() + end.x()) / 2;
            double midY = (start.y() + end.y()) / 2;

            QString idText = QString("ID: %1").arg(LeftMenuID);
            QFontMetrics metrics(painter.font());
            QRectF textRect = metrics.boundingRect(idText);

            // Вычисление угла наклона линии
            qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

            // Если линия "в обратную сторону", перевернем текст
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
            painter.translate(offsetX - textRect.width() / 2, offsetY - textRect.height() /2);

            painter.drawText(0, 0, idText);
            painter.restore();

        }
    }

    // Отрисовка айди для точки
    static void drawPointLeftID(QPainter &painter, QPointF &point) {
        if (LeftMenuID != 0) {

            QString idText = QString("ID: %1").arg(LeftMenuID);

            // Смещение вверх над точкой
            int offsetY = 10;

            QRectF textRect = painter.fontMetrics().boundingRect(idText);
            double textX = (point.x() - textRect.width() / 2);
            double textY = point.y() - offsetY;

            painter.save();
            painter.setPen(QPen(Qt::black, 1));
            painter.drawText( QPointF(textX,  textY), idText);
            painter.restore();

        }
    }

/*********************************/
// Свечение

    // Отрисовка свечения для точки
    static void drawPointGlow(QPainter &painter, QPointF &point) {
        // Рисуем свечение для выбранной точки
        int glowRadius = 5;
        QRadialGradient radialGradient(point, glowRadius, point);
        radialGradient.setColorAt(0.0, QColor(0, 128, 255, 200));
        radialGradient.setColorAt(1.0, QColor(0, 128, 255, 0));

        QBrush glowBrush(radialGradient);
        painter.setBrush(glowBrush);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(point.x() - glowRadius, point.y() - glowRadius), glowRadius * 2, glowRadius * 2);
    }

    // Отрисовка свечения для линии
    static void drawSectionGlow(QPainter &painter, QPointF &start,QPointF &end) {

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
        qreal glowDistance = 10.0;

        // Определяем точки начала и конца градиента
        qreal gradStartX = start.x() + perpX * glowDistance;
        qreal gradStartY = start.y() + perpY * glowDistance;
        qreal gradEndX = start.x() - perpX * glowDistance;
        qreal gradEndY = start.y() - perpY * glowDistance;

        // Создаём линейный градиент перпендикулярный линии
        QLinearGradient gradient(gradStartX, gradStartY, gradEndX, gradEndY);
        gradient.setColorAt(0.0, QColor(0, 255, 255, 150)); // Голубой с прозрачностью
        gradient.setColorAt(1.0, QColor(0, 255, 255, 0));   // Прозрачный

        QPen glowPen;
        glowPen.setWidth(8);
        glowPen.setBrush(gradient);
        glowPen.setCapStyle(Qt::RoundCap); // Устанавливаем закругленные концы
        painter.setPen(glowPen);

        painter.drawLine(start.x(), start.y(), end.x(), end.y());
    }

    // Отрисовка свечения для круга
    static void drawCircleGlow(QPainter& painter, QPointF &center,double Radius) {
        // Преобразуе
        // Рисуем свечение для выбранной точки
        // Параметры свечения
        const qreal glowDistance = 20.0; // Расстояние свечения от обводки
        qreal gradientRadius = (Radius ) + glowDistance; // Общий радиус градиента

        // Вычисляем относительные позиции для цветовых остановок
        qreal stopTransparent1 = (Radius - glowDistance) / gradientRadius;
        qreal stopCyan = (Radius ) / gradientRadius;
        qreal stopTransparent2 = (Radius  + glowDistance) / gradientRadius;

        // Убедимся, что все значения позиций лежат в пределах от 0 до 1
        stopTransparent1 = qBound(0.0, stopTransparent1, 1.0);
        stopCyan = qBound(0.0, stopCyan, 1.0);
        stopTransparent2 = qBound(0.0, stopTransparent2, 1.0);

        // Создаём радиальный градиент
        QRadialGradient radialGradient(center, gradientRadius,center);
        radialGradient.setColorAt(0.0, QColor(0, 255, 255, 0));         // Прозрачный в центре
        radialGradient.setColorAt(stopTransparent1, QColor(0, 255, 255, 0));   // Прозрачный до внутренней границы свечения
        radialGradient.setColorAt(stopCyan, QColor(0, 255, 255, 150));      // Насыщенный циан на обводке
        radialGradient.setColorAt(stopTransparent2, QColor(0, 255, 255, 0)); // Прозрачный после внешней границы свечения
        radialGradient.setColorAt(1.0, QColor(0, 255, 255, 0));            // Прозрачный за пределами свечения        // Прозрачный за пределами свечения

        // Настраиваем перо для свечения
        QPen glowPen(QBrush(radialGradient), 4); // Толщина пера 4 пикселя
        glowPen.setJoinStyle(Qt::RoundJoin);
        glowPen.setCapStyle(Qt::RoundCap);
        painter.setPen(glowPen);
        painter.setBrush(Qt::NoBrush); // Без заливки

        // Рисуем круг с градиентной обводкой для свечения
        painter.drawEllipse(center, Radius,  Radius);
    }

    ///////////////////////////////////////////

    // Отрисовка длины линии
    static void drawCoordinateLine(QPainter &painter, QPointF& start,QPointF &end) {
        // Преобразуем в экранные координаты
        double dx = start.x() - end.x();
        double dy = start.y() - end.y();

        qreal length = Scaling::logicDouble(qSqrt(dx * dx + dy * dy));

        QString lengthText = QString::number(length, 'f', 2);
        QFontMetrics metrics(painter.font());
        QRectF textRect = metrics.boundingRect(lengthText);

        // Середина линии
        double midX = (start.x() + end.x()) / 2;
        double midY = (start.y() + end.y()) / 2;

        painter.save();

        // Случай вертикальной линии
        if (dx ==0 ) {
            qreal offsetX =  textRect.width()/2;
            qreal offsetY = textRect.height() / 2;

            painter.translate(midX, midY);
            painter.translate(offsetX, offsetY);
            painter.rotate(-90);  // Поворачиваем текст на 90 градусов
            painter.drawText(0, 0, lengthText);
        } else {
            // Вычисляем угол линии
            qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

            // Проверка на "обратное" направление
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
};

#endif //OURPAINT_DRAWADDITIONALINF_H

