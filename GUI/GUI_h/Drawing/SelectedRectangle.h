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
    QRectF selected(QPainter &painter);

    void clear();
};

#endif //OURPAINT_SELECTEDRECTANGLE_H
