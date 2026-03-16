#include "DrawTool.h"


DrawTool::DrawTool(QObject* parent)
        : QObject(parent),startCoordinates{},state(DrawState::NotStarted) {}


void DrawTool::pressButton(const QPointF& cursor) {
    if (state == DrawState::NotStarted) {
        startCoordinates = roundCursor(cursor);
        state = DrawState::Started;
    } else if (state == DrawState::Started) {
        state = DrawState::Completed;
    }
}


void DrawTool::clear() {
    startCoordinates = QPointF();
    state = DrawState::NotStarted;
}


QPointF DrawTool::roundCursor(const QPointF& point) {
    return {qRound(point.x() * 10.0) / 10.0, qRound(point.y() * 10.0) / 10.0};
}











