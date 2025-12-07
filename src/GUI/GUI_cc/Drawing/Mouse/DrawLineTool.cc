#include "DrawLineTool.h"


void DrawLineTool::draw(QPainter& painter, const QPointF& nowCursor) {
    // Rounding the mouse to 1 decimal place
    const QPointF cursor = roundCursor(nowCursor);

    switch (state) {
        case DrawState::Completed:
            emit SigSection(startCoordinates, cursor);
            clear();
            break;
        case DrawState::Started:
            drawRunningSection(painter, startCoordinates, cursor);
            break;
        default:
            clear();
            break;
    }
}


void DrawLineTool::drawRunningSection(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) {
    // Rounding the mouse to 1 decimal place
    const QPointF Cursor = roundCursor(cursor);
    DrawFigures::drawSection(painter, startCoordinates, Cursor);
}


void DrawLineTool::drawPreview(QPainter& painter, const QPointF& start, const QPointF& end) {
    DrawFigures::setPen(QPen(hintColor()));
    DrawFigures::drawSection(painter, start, end);
}


qreal DrawLineTool::snapAngle(qreal angle) {
    return qRound(angle / 45.0) * 45.0;
}


QPointF DrawLineTool::getSnappedPoint(const QPointF& current) {
    if (state != DrawState::Started) {
        return current;
    }

    const qreal dx = current.x() - startCoordinates.x();
    const qreal dy = current.y() - startCoordinates.y();
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

    QPointF snapped = startCoordinates + offset;

    return snapped;
}


void DrawLineTool::tabPressed() {
    if (state == DrawState::NotStarted) {
        startCoordinates = closestStartPoint;
        state = DrawState::Started;
    }else if (state == DrawState::Started) {
        state = DrawState::Completed;
    }
}


void DrawLineTool::pressButton(const QPointF& cursor) {
    if (state == DrawState::NotStarted) {
        startCoordinates = roundCursor(cursor);
        state = DrawState::Started;
    }else if (state == DrawState::Started) {
        state = DrawState::Completed;
    }
}


void DrawLineTool::drawHints(QPainter& painter, const QPointF& closestPoint, const QPointF& cursor) {
    if (state == DrawState::NotStarted) {
        closestStartPoint = closestPoint;
        // Rounding the mouse to 1 decimal place
        const QPointF Cursor = roundCursor(cursor);
        drawPreview(painter, closestStartPoint, Cursor);
    } else if (state == DrawState::Started) {
        closestPointNext = closestPoint;
        drawPreview(painter, startCoordinates, closestPointNext);
    } else if (state == DrawState::Completed) {
        emit SigSection(startCoordinates, closestPointNext);
        clear();
        closestStartPoint = QPointF();
        closestPointNext = QPointF();
    }
}
