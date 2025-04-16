#ifndef OURPAINT_DRAWBACKGROUND_H
#define OURPAINT_DRAWBACKGROUND_H

#include <QPainter>
#include <QDebug>

#include "DrawAdditionalInf.h"
#include "Scaling.h"
#include "Modes.h"
// Класс отвечающий за отрисовку фона виджета
// Центр у нас в центре виджета
// Клетки создают эффект передвижения
// Оси создают эффект бесконечности

class DrawBackground {
private:

public:
    explicit DrawBackground() = default;

    static double Step(double rawStep) {
        double exp = std::floor(std::log10(rawStep));
        double fraction = rawStep / std::pow(10.0, exp);

        double newFraction=0;
        if (fraction < 1.5) { newFraction = 1; }
        else if (fraction < 2.5) { newFraction = 2; }
        else if (fraction < 3.5) { newFraction = 2.5; }
        else if (fraction < 5.0) { newFraction = 4; }
        else if (fraction < 7.5) { newFraction = 5; }
        else { newFraction = 10; }

        return newFraction * std::pow(10.0, exp);
    }


    // Отрисовка фона
    static void drawFon(QPainter &painter) {

        double _width = Scaling::getCenteredCoordinatesX();
        double _height = Scaling::getCenteredCoordinatesY();
        short int width = Scaling::getActualMonitorWidth();
        short int height = Scaling::getActualMonitorHeight();

        double deltaX = Scaling::getDeltaX();
        double deltaY = Scaling::getDeltaY();


        double zoom = Scaling::getZoom();

        // Отрисовка клетчатого фона
        if (ModeManager::getCell()) {

            const double minStep = Scaling::getUserUnitSize();
            double stepLogical = Step(minStep / zoom);
            double currentCellSize = stepLogical * zoom;

            std::vector<QPointF> pointXR;
            std::vector<QPointF> pointXL;
            std::vector<QPointF> pointYU;
            std::vector<QPointF> pointYD;
            int index = 1; // Делаем блоки 5 на 5

            painter.setPen(QPen(Qt::lightGray, 1));
            // Вертикальные линии (оси Y), симметрично от центра
            for (double x = currentCellSize; x <= (_width + abs(deltaX)); x += currentCellSize) {
                painter.setPen(index % 5 == 0 ? Qt::darkGray : Qt::lightGray);
                if (index % 5 == 0) {
                    pointXR.emplace_back(x, 0);
                    pointXL.emplace_back(-x, 0);
                }
                painter.drawLine(QPointF(x, (-_height - abs(deltaY))), QPointF(x, (_height + abs(deltaY))));
                painter.drawLine(QPointF(-x, (-_height - abs(deltaY))), QPointF(-x, (_height + abs(deltaY))));
                ++index;
            }

            // Горизонтальные линии (оси X), симметрично от центра
            index = 1;
            for (double y = currentCellSize; y <= (_height + abs(deltaY)); y += currentCellSize) {
                painter.setPen(index % 5 == 0 ? Qt::darkGray : Qt::lightGray);
                if (index % 5 == 0) {
                    pointYU.emplace_back(0, y);
                    pointYD.emplace_back(0, -y);
                }
                painter.drawLine(QPointF((-_width - abs(deltaX)), y), QPointF((_width + abs(deltaX)), y));
                painter.drawLine(QPointF((-_width - abs(deltaX)), -y), QPointF((_width + abs(deltaX)), -y));
                ++index;
            }

            DrawAdditionalInf::drawCoordinateLabels(painter, pointXL, pointXR, pointYU, pointYD);


        }

        // Оси координат
        painter.setPen(Qt::black);

        // Отрисовка вертикальной оси Oy
        if (_width <= std::abs(deltaX)) {
            if (deltaX > 0) {
                // крайняя правая
                painter.drawLine(QPointF(_width - 1 - deltaX, -_height - deltaY),
                                 QPointF(_width - 1 - deltaX, height - deltaY));  // Oy
            } else {
                // крайняя левая
                painter.drawLine(QPointF(-_width + 1 - deltaX, -_height - deltaY),
                                 QPointF(-_width + 1 - deltaX, height - deltaY));  // Oy
            }
        } else {
            painter.drawLine(QPointF(0, (-_height - deltaY)), QPointF(0, (height - deltaY)));  // Oy
        }

        // Отрисовка горизонтальной оси Ox
        if (_height <= std::abs(deltaY)) {
            if (deltaY > 0) {
                // Нижняя
                painter.drawLine(QPointF(-_width - deltaX, _height - 1 - deltaY),
                                 QPointF(_width - deltaX, _height - 1 - deltaY));  // Ox
            } else {
                // Верхняя
                painter.drawLine(QPointF(-_width - deltaX, -_height - deltaY),
                                 QPointF(_width - deltaX, -_height - deltaY));  // Ox
            }
        } else {
            painter.drawLine(QPointF((-_width - deltaX), 0), QPointF((_width - deltaX), 0));  // Ox
        }

        //   DrawAdditionalInf::drawAxes(painter);

    }

    static double computeCyclicCellSize(double baseSize, double minSize, double maxSize) {
        // Применяем масштабирование:
        double scaledValue = Scaling::scaleCoordinate(baseSize);

        // Зададим диапазон изменения:
        double cycleRange = maxSize - minSize;

        // Приведём scaledValue к циклической форме:
        // Сместим на minSize, чтобы цикл шел от 0 до cycleRange
        double modValue = fmod(scaledValue - minSize, cycleRange);

        // fmod может вернуть отрицательное значение, выправим его:
        if (modValue < 0) {
            modValue += cycleRange;
        }

        // Теперь текущий размер клетки будет циклически меняться от minSize до maxSize.
        double currentCellSize = minSize + modValue;
        return currentCellSize;
    }


};

#endif //OURPAINT_DRAWBACKGROUND_H