#ifndef UNNECESSARY
#define UNNECESSARY

#include <QPointF>
#include <QRectF>
#include <QPainter>

#include "Modes.h"
#include "Scaling.h"
#include "DrawFigures.h"

class [[maybe_unused]] SelectedRectangle {
private:
    QPointF startMouse;
    QPointF endMouse;
    bool isSelecting;
    bool end;

public:
    SelectedRectangle();
    QRectF selected(QPainter& painter);
    void clear();
};

#endif UNNECESSARY
