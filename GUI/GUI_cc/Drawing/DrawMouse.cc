#include "DrawMouse.h"


[[maybe_unused]] DrawMouse::DrawMouse(QObject* parent)
        : QObject(parent), tabPressCount(0), drawingInProgress(false) {}


inline QColor DrawMouse::hintColor() {
    return {169, 169, 169, 128};
}


inline qreal DrawMouse::snapAngle(qreal angle) {
    return qRound(angle / 45.0) * 45.0;
}


QPointF DrawMouse::roundCursor(const QPointF& point) {
    return {qRound(point.x() * 10.0) / 10.0, qRound(point.y() * 10.0) / 10.0};
}


void DrawMouse::releaseTabIfPressed() {
    if (ModeManager::getActiveMode(KeyMode::Tab)) {
        ModeManager::setActiveMode(KeyMode::ReleasingTab);
    }
}


QPointF DrawMouse::getSnappedPoint(const QPointF& start, const QPointF& current) {
    const qreal dx = current.x() - start.x();
    const qreal dy = current.y() - start.y();
    const qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;
    qint32 snappedAngle = static_cast<qint32>(snapAngle(angle));
    const qreal length = qHypot(dx, dy);

    QPointF offset(0, 0);

    const qreal diag = length / qSqrt(2.0);

    switch (snappedAngle) {
        case 0:
        case 360:
            offset = {length, 0};
            break;
        case 45:
            offset = {diag, diag};
            break;
        case 90:
            offset = {0, length};
            break;
        case 135:
            offset = {-diag, diag};
            break;
        case 180:
        case -180:
            offset = {-length, 0};
            break;
        case -135:
            offset = {-diag, -diag};
            break;
        case -90:
            offset = {0, -length};
            break;
        case -45:
            offset = {diag, -diag};
            break;
    }

    QPointF snapped = start + offset;

    return snapped;
}


void DrawMouse::drawPreviewSection(QPainter& painter, const QPointF& start, const QPointF& end) {
    DrawFigures::setPen(QPen(hintColor()));
    DrawFigures::drawSection(painter, start, end);
}


void DrawMouse::drawPreviewPoint(QPainter& painter, const QPointF& point) {
    DrawFigures::setPen(hintColor());
    DrawFigures::drawPoint(painter, point);
}


void DrawMouse::drawPreviewArcs(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) {
    const QPointF center = (startCoordinates + cursor) / 2.0;
    DrawFigures::drawArc(painter, cursor, startCoordinates, center);
}


void DrawMouse::DrawPoint(QPainter& painter, const QPointF& nowCursor) {
    if (!ModeManager::getActiveMode(WorkModes::Point)) {
        return;
    }

    painter.setPen(Qt::black);
    // Rounding the mouse to 1 decimal place
    const QPointF Cursor = roundCursor(nowCursor);
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);

    if (rightClick) {
        clear();
        return;
    }

    if (leftClick) {
        ModeManager::setActiveMode(MouseMode::ReleasingLeft);
        // If one tap and a dot => dot
        emit SigPoint(Cursor);
        clear();
    } else {
        DrawMouse::drawPreviewPoint(painter, Cursor);
    }
}


void DrawMouse::DrawSection(QPainter& painter, const QPointF& nowCursor) {
    if (!ModeManager::getActiveMode(WorkModes::Section)) {
        return;
    }

    painter.setPen(Qt::black);
    // Rounding the mouse to 1 decimal place
    const QPointF Cursor = roundCursor(nowCursor);
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool shiftPressed = ModeManager::getActiveMode(KeyMode::Shift);
    const bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);

    if (rightClick) {
        clear();
        return;
    }

    if (leftClick) {
        ModeManager::setActiveMode(MouseMode::ReleasingLeft);

        if (!drawingInProgress) {
            ++tabPressCount;
            startCoordinates = Cursor;
            drawingInProgress = true;
            return;
        }

        if (shiftPressed) {
            // When pressed, the angles are fixed at 45
            QPointF snapped = getSnappedPoint(startCoordinates, Cursor);
            emit SigSection(startCoordinates, snapped);
        } else {
            emit SigSection(startCoordinates, Cursor);
        }
        clear();
    } else {
        if (drawingInProgress) {
            drawRunningSection(painter, startCoordinates, Cursor);
        }
    }
}


void DrawMouse::DrawCircle(QPainter& painter, const QPointF& nowCursor) {
    if (!ModeManager::getActiveMode(WorkModes::Circle)) {
        return;
    }
    const QPointF Cursor = roundCursor(nowCursor);
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);

    if (rightClick) {
        clear();
        return;
    }

    if (leftClick) {
        ModeManager::setActiveMode(MouseMode::ReleasingLeft);

        if (!drawingInProgress) {
            ++tabPressCount;
            startCoordinates = Cursor;
            drawingInProgress = true;
            return;
        }

        const QPointF center = (Cursor + startCoordinates) / 2.0;
        qreal radius = qHypot(startCoordinates.x() - center.x(), startCoordinates.y() - center.y());
        emit SigCircle(center, radius);
        clear();
    } else {
        if (drawingInProgress) {
            drawPreviewCircles(painter, startCoordinates, Cursor);
        }
    }
}


void DrawMouse::DrawArc(QPainter& painter, const QPointF& nowCursor) {
    if (!ModeManager::getActiveMode(WorkModes::Arc)) {
        return;
    }
    const QPointF Cursor = roundCursor(nowCursor);
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);

    if (rightClick) {
        clear();
        return;
    }

    if (leftClick) {
        ModeManager::setActiveMode(MouseMode::ReleasingLeft);

        if (!drawingInProgress) {
            ++tabPressCount;
            startCoordinates = Cursor;
            drawingInProgress = true;
            return;
        }

        const QPointF center = (Cursor + startCoordinates) / 2.0;
        emit SigArc(Cursor, startCoordinates, center);
        clear();
    } else {
        if (drawingInProgress) {
            DrawMouse::drawPreviewArcs(painter, startCoordinates, Cursor);
        }
    }
}


void DrawMouse::drawPreviewCircles(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) {
    if (!ModeManager::getActiveMode(WorkModes::Circle)) {
        return;
    }
    const QPointF center = (startCoordinates + cursor) / 2.0;
    const qreal radius = qHypot(startCoordinates.x() - center.x(), startCoordinates.y() - center.y());

    DrawFigures::drawCircle(painter, center, radius);
    DrawFigures::drawPoint(painter, center);

    DrawFigures::setPen(hintColor());
    DrawFigures::drawSection(painter, center, cursor);
}


void DrawMouse::drawRunningSection(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) {
    if (!ModeManager::getActiveMode(WorkModes::Section)) {
        return;
    }

    // Rounding the mouse to 1 decimal place
    const QPointF Cursor = roundCursor(cursor);
    const bool shiftPressed = ModeManager::getActiveMode(KeyMode::Shift);

    if (shiftPressed) {
        const QPointF snapped = getSnappedPoint(startCoordinates, Cursor);
        DrawFigures::drawSection(painter, startCoordinates, snapped);
    } else {
        DrawFigures::drawSection(painter, startCoordinates, Cursor);
    }
}


void DrawMouse::drawHints(QPainter& painter, const QPointF& closesPoint, const QPointF& cursor) {

    const bool tabPressed = ModeManager::getActiveMode(KeyMode::Tab);
    const bool modeSection = ModeManager::getActiveMode(WorkModes::Section);

    if (!modeSection) {
        clear();
        return;
    }

    if (tabPressed) {
        ++tabPressCount;
        if (tabPressCount == 1) {
            startCoordinates = closestStartPoint;
        }
    }

    if (tabPressCount == 0) {
        closestStartPoint = closesPoint;

        // Rounding the mouse to 1 decimal place
        const QPointF Cursor = roundCursor(cursor);

        drawPreviewSection(painter, closestStartPoint, Cursor);
        releaseTabIfPressed();
    } else if (tabPressCount == 1) {
        drawingInProgress = true;
        closestPointNext = closesPoint;
        drawPreviewSection(painter, startCoordinates, closestPointNext);
        releaseTabIfPressed();
    } else if (tabPressCount == 2) {
        releaseTabIfPressed();

        emit SigSection(startCoordinates, closestPointNext);

        clear();
    }


}


void DrawMouse::clear() {
    closestStartPoint = QPointF();
    closestPointNext = QPointF();
    startCoordinates = QPointF();
    drawingInProgress = false;
    tabPressCount = 0;
}
