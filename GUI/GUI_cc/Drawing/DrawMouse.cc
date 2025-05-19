#include "DrawMouse.h"


[[maybe_unused]] DrawMouse::DrawMouse(QObject* parent)
        : QObject(parent), drawingInProgress(false), tabPressCount(0) {}


inline QColor DrawMouse::hintColor() {
    return {169, 169, 169, 128};
}


void DrawMouse::resetCoordinates() {
    startCoordinates = QPoint();
    closestStartPoint = QPoint();
    closestPointNext = QPoint();
}


void DrawMouse::releaseTabIfPressed() {
    if (ModeManager::getActiveMode(KeyMode::Tab)) {
        ModeManager::setActiveMode(KeyMode::ReleasingTab);
    }
}


inline qreal DrawMouse::snapAngle(qreal angle) {
    return qRound(angle / 45.0) * 45.0;
}


QPointF DrawMouse::getSnappedPoint(const QPointF& start, const QPointF& current) {
    const qreal dx = current.x() - start.x();
    const qreal dy = current.y() - start.y();
    const qreal angle = std::atan2(dy, dx) * 180.0 / M_PI;
    const qreal snappedAngle = snapAngle(angle);
    const qreal length = std::hypot(dx, dy);

    QPointF offset(0,0);

    const qreal diag = length / qSqrt(2.0);

    if (snappedAngle == 0 || snappedAngle == 360) {
        offset={length,0};
    } else if (snappedAngle == 45) {
        offset={diag,diag};
    } else if (snappedAngle == 90) {
        offset={0,length};
    } else if (snappedAngle == 135) {
        offset={-diag,diag};
    } else if (snappedAngle == 180 || snappedAngle == -180) {
        offset={-length,0};
    } else if (snappedAngle == -135) {
        offset={-diag,-diag};
    } else if (snappedAngle == -90) {
        offset={0,-length};
    } else if (snappedAngle == -45) {
        offset={diag,-diag};
    }

    QPointF snapped = start + offset;

    return snapped;
}


void DrawMouse::drawPreviewSection(QPainter& painter, const QPointF& start, const QPointF& end) {
    DrawFigures::setPen(QPen(hintColor()));
    DrawFigures::drawSection(painter, start, end);
}


void DrawMouse::DrawFiguresMouse(QPainter& painter,const QPointF& cursor) {

    painter.setPen(Qt::black);

    // Rounding the mouse to 1 decimal place
    const qreal cursorX = std::round(cursor.x() * 10.0) / 10.0;
    const qreal cursorY = std::round(cursor.y() * 10.0) / 10.0;
    const QPointF Cursor(cursorX, cursorY);

    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);
    const bool shiftPressed = ModeManager::getActiveMode(KeyMode::Shift);

    bool modePoint = ModeManager::getActiveMode(WorkModes::Point);
    bool modeSection = ModeManager::getActiveMode(WorkModes::Section);
    bool modeCircle = ModeManager::getActiveMode(WorkModes::Circle);
    bool modeArc = ModeManager::getActiveMode(WorkModes::Arc);


    if (modePoint && !leftClick) {
        DrawFigures::setPen(hintColor());
        DrawFigures::drawPoint(painter, Cursor);
    }

    if (leftClick) {
        if (modePoint) {
            // If one tap and a dot => dot
            emit SigPoint(Cursor);
        } else {
            if (!drawingInProgress) {
                ++tabPressCount;
                startCoordinates = Cursor;
                drawingInProgress = true;
            } else {
                if (modeSection) {

                    if (shiftPressed) {
                        // When pressed, the angles are fixed at 45
                        QPointF snapped = getSnappedPoint(startCoordinates, Cursor);
                        emit SigSection(startCoordinates,snapped);
                    } else {
                        emit SigSection(startCoordinates, cursor);
                    }
                } else if (modeCircle) {
                    const QPointF center=(cursor+startCoordinates)/2.0;
                    qreal radius = std::hypot(startCoordinates.x() - center.x(), startCoordinates.y() - center.y());
                    emit SigCircle(center, radius);
                } else if (modeArc) {
                    const QPointF center=(cursor+startCoordinates)/2.0;
                    emit SigArc(cursor, startCoordinates, center);
                }
                resetCoordinates();
                tabPressCount = 0;
                drawingInProgress = false;
            }
        }
        ModeManager::setActiveMode(MouseMode::ReleasingLeft);
    }

    if (rightClick) {
        resetCoordinates();
        tabPressCount = 0;
        drawingInProgress = false;
    }

    if (drawingInProgress) {
        if (modeCircle) {
            drawCircles(painter, startCoordinates, Cursor);

        } else if (modeSection) {
            drawSections(painter, startCoordinates, Cursor);
        } else if (modeArc) {
            const QPointF center = (startCoordinates + Cursor) / 2.0;
            DrawFigures::drawArc(painter, Cursor, startCoordinates, center);
        }
    }
}


void DrawMouse::drawCircles(QPainter& painter, const QPointF& startCoordinates,const QPointF& cursor) {
    const QPointF center = (startCoordinates + cursor) / 2.0;
    const qreal radius = std::hypot(startCoordinates.x() - center.x(), startCoordinates.y() - center.y());

    DrawFigures::drawCircle(painter, center, radius);
    DrawFigures::drawPoint(painter, center);

    DrawFigures::setPen(hintColor());
    DrawFigures::drawSection(painter, center, cursor);
}


void DrawMouse::drawSections(QPainter& painter, const QPointF& startCoordinates,const QPointF& cursor) {
    if (!ModeManager::getActiveMode(WorkModes::Section)) {
        return;
    }

    // Rounding the mouse to 1 decimal place
    const qreal cursorX = std::round(cursor.x() * 10.0) / 10.0;
    const qreal cursorY = std::round(cursor.y() * 10.0) / 10.0;
    const QPointF Cursor(cursorX, cursorY);
    const bool shiftPressed = ModeManager::getActiveMode(KeyMode::Shift);

    if (shiftPressed) {
        const QPointF snapped = getSnappedPoint(startCoordinates, Cursor);
        DrawFigures::drawSection(painter, startCoordinates, snapped);
    } else {
        DrawFigures::drawSection(painter, startCoordinates, Cursor);
    }
}


void DrawMouse::drawHints(QPainter& painter, const QPointF& closesPoint,const QPointF& cursor) {

    const bool tabPressed = ModeManager::getActiveMode(KeyMode::Tab);
    const  bool modeSection = ModeManager::getActiveMode(WorkModes::Section);

    if (modeSection) {
        if (tabPressed) {
            ++tabPressCount;
            if (tabPressCount == 1) {
                startCoordinates = closestStartPoint;
            }
        }

        if (tabPressCount == 0) {
            closestStartPoint = closesPoint;

            // Rounding the mouse to 1 decimal place
            const qreal cursorX = std::round(cursor.x() * 10.0) / 10.0;
            const qreal cursorY = std::round(cursor.y() * 10.0) / 10.0;
            const QPointF Cursor = QPointF(cursorX, cursorY);

            drawPreviewSection(painter, closestStartPoint, Cursor);
            releaseTabIfPressed();
        } else if (tabPressCount == 1) {
            drawingInProgress = true;
            closestPointNext = closesPoint;
            drawPreviewSection(painter, closestStartPoint, closestPointNext);
            releaseTabIfPressed();
        } else if (tabPressCount == 2) {
            releaseTabIfPressed();
            tabPressCount = 0;
            drawingInProgress = false;

            emit SigSection(closestStartPoint, closestPointNext);

            resetCoordinates();
        }
    } else {
        tabPressCount = 0;
        resetCoordinates();
    }
}


void DrawMouse::clear() {
    closestStartPoint = QPointF();
    closestPointNext = QPointF();
    startCoordinates = QPointF();
    drawingInProgress = false;
    tabPressCount = 0;
}