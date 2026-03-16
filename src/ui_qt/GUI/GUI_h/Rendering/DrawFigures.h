#ifndef OURPAINT_DRAWFIGURES_H
#define OURPAINT_DRAWFIGURES_H

#include <QPainter>
#include <unordered_set>
#include <cmath>

#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include "GeometricObjects.h"
#include "Colors.h"
#include "RenderStyle.h"

// A class for drawing shapes
// Coordinate transformation taking into account scaling and the inverted Y-axis,
// and also uses additional functions to select objects


class DrawFigures {
private:
    static QPen MyColor;

public:
    static void setPen(const QPen& color);
    static void drawRectangle(QPainter& painter, const QPointF startPoint, const QPointF endPoint);


};

#endif // OURPAINT_DRAWFIGURES_H
