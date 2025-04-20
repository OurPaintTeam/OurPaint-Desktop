#ifndef OURPAINT_SELECTEDRECTANGLE_H
#define OURPAINT_SELECTEDRECTANGLE_H

#include <QPointF>
#include <QRectF>
#include <QPainter>

#include "Modes.h"
#include "Scaling.h"
#include "DrawFigures.h"

class SelectedRectangle {
private:
    QPointF startMouse;
    QPointF endMouse;
    bool isSelecting = false;
    bool end = false;

public:
    QRectF selected(QPainter &painter) {
        bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
        bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);
        if (rightClick) {
           clear();
        }
        // Начало выделения
        if (leftClick && !isSelecting) {
            double cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
            double cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;
            startMouse=QPointF(cursorX, cursorY);

            ModeManager::setActiveMode(MouseMode::ReleasingLeft);
            leftClick = false;
            isSelecting = true;
            end=false;
        }

        // В движении
        if (!leftClick && isSelecting) {
            double cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
            double cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;
            QPointF cursor(cursorX, cursorY);
            DrawFigures::drawRectangle(painter, startMouse, cursor);
            end=false;
            return QRectF(startMouse, cursor);
        }

        // Завершение выделения
        if (leftClick && isSelecting) {
            double cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
            double cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;
            endMouse=QPointF(cursorX, cursorY);
            isSelecting = false;
            leftClick = false;
            ModeManager::setActiveMode(MouseMode::ReleasingLeft);
            end = true;
            return QRectF(startMouse, endMouse);
        }



        if (end) {
            DrawFigures::drawRectangle(painter, startMouse, endMouse);
        }
        return QRectF{};
    }

    void clear(){
        startMouse = QPointF();
        endMouse = QPointF();
        end = false;
        isSelecting = false;
    }
};

#endif //OURPAINT_SELECTEDRECTANGLE_H
