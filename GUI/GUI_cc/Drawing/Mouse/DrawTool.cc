#include "DrawTool.h"


DrawTool::DrawTool(QObject* parent)
        : QObject(parent),startCoordinates{},countClick(0) {}


void DrawTool::pressButton(const QPointF& cursor) {
    if (countClick == 0) {
        startCoordinates = roundCursor(cursor);
    }
    ++countClick;
};

void DrawTool::clear() {
    startCoordinates = QPointF();
    countClick = 0;
};


QPointF DrawTool::roundCursor(const QPointF& point) {
    return {qRound(point.x() * 10.0) / 10.0, qRound(point.y() * 10.0) / 10.0};
}

QColor DrawTool::hintColor() {
    return {169, 169, 169, 128};
}












