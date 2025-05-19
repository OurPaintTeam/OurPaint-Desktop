#include "SelectedRectangle.h"

SelectedRectangle::SelectedRectangle() : isSelecting(false),end(false) {}

QRectF SelectedRectangle::selected(QPainter& painter) {
    bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);

    if (rightClick) {
        clear();
    }

    // Start of selection
    if (leftClick && !isSelecting) {
        const qreal cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
        const qreal cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;
        startMouse = QPointF(cursorX, cursorY);

        ModeManager::setActiveMode(MouseMode::ReleasingLeft);

        leftClick = false;
        isSelecting = true;
        end = false;
    }

    // On the move
    if (!leftClick && isSelecting) {
        const qreal cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
        const qreal cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;
        const QPointF cursor(cursorX, cursorY);

        DrawFigures::drawRectangle(painter, startMouse, cursor);
        end = false;

        return QRectF(startMouse, cursor);
    }

    // Completion of selection
    if (leftClick) {
        const qreal cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
        const qreal cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;

        endMouse = QPointF(cursorX, cursorY);

        ModeManager::setActiveMode(MouseMode::ReleasingLeft);
        isSelecting = false;
        end = true;

        return QRectF(startMouse, endMouse);
    }


    if (end) {
        DrawFigures::drawRectangle(painter, startMouse, endMouse);

        return QRectF(startMouse, endMouse);
    }

    return QRectF{};
}

void SelectedRectangle::clear() {
    startMouse = QPointF();
    endMouse = QPointF();
    end = false;
    isSelecting = false;
}
