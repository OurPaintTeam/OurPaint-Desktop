#include "SelectedRectangle.h"

QRectF SelectedRectangle::selected(QPainter &painter) {
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
        return QRectF(startMouse, endMouse);

    }
    return QRectF{};
}

void SelectedRectangle::clear(){
    startMouse = QPointF();
    endMouse = QPointF();
    end = false;
    isSelecting = false;
}