#ifndef OURPAINT_DRAWBACKGROUND_H
#define OURPAINT_DRAWBACKGROUND_H

#include <QPainter>
#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include <QDebug>

// Класс отвечающий за отрисовку фона виджета
// Центр у нас в центре виджета
// Клетки создают эффект передвижения
// Оси создают эффект бесконечности

class drawBackground {
private:

public:
    explicit drawBackground() = default;

// Отрисовка фона
    static void drawFon(QPainter &painter) {

        double _width = Scaling::getCenteredCoordinatesX();
        double _height = Scaling::getCenteredCoordinatesY();
        short int width = Scaling::getDisplayCoordinateX();
        short int height = Scaling::getDisplayCoordinateY();
        int zoom = Scaling::getZoom();
        double deltaX = Scaling::getDeltaX();
        double deltaY = Scaling::getDeltaY();

        const int maxCellSize = 60; // Максимальный размер клетки при масштабировании
        const int minCellSize = 10; // Минимальный размер клетки при масштабировании
        const int CellSize = 45;





        // Отрисовка клетчатого фона
        if (ModeManager::getCell()) {
            // Размер клетки
            int currentCellSize=zoom;




            std::vector<QPointF> pointXR;
            std::vector<QPointF> pointXL;
            std::vector<QPointF> pointYU;
            std::vector<QPointF> pointYD;
            int index = 1; // Делаем блоки 5 на 5

            painter.setPen(QPen(Qt::lightGray, 1));
            // Вертикальные линии (оси Y), симметрично от центра
            for (int x = currentCellSize; x <= (_width + abs(deltaX)); x += currentCellSize) {
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
            for (int y = currentCellSize; y <= (_height + abs(deltaY)); y += currentCellSize) {
                painter.setPen(index % 5 == 0 ? Qt::darkGray : Qt::lightGray);
                if (index % 5 == 0) {
                    pointYU.emplace_back(0, y);
                    pointYD.emplace_back(0, -y);
                }
                painter.drawLine(QPointF((-_width - abs(deltaX)), y), QPointF((_width + abs(deltaX)), y));
                painter.drawLine(QPointF((-_width - abs(deltaX)), -y), QPointF((_width + abs(deltaX)), -y));
                ++index;
            }

            DrawAdditionalInf::drawCoordinateLabels(painter, pointXL,pointXR,pointYU,pointYD);
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