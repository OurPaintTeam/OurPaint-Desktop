#ifndef OURPAINT_DRAWBACKGROUND_H
#define OURPAINT_DRAWBACKGROUND_H

#include <QPainter>
#include <QtMath>

#include "DrawAdditionalInf.h"
#include "Scaling.h"
#include "Modes.h"

// The class responsible for rendering the background of the widget
// The center is in the center of the widget
// The cells create the effect of movement
// The axes create the effect of infinity

class DrawBackground {
public:
    explicit DrawBackground() = default;

    // So that there are no strongly fractional numbers on the grid,
    // we are looking for a close value of the cell size
    static qreal Step(const qreal rawStep);

    // Background rendering
    static void backgroundRender(QPainter& painter);

    // Drawing the main coordinate axes
    static void mainBackgroundRender(QPainter& painter);

    static void switchProjection(QPainter& painter);

    static void drawScribbleCloud(QPainter& painter, const QPointF& center, QColor color, int radius, int count);
};

#endif //OURPAINT_DRAWBACKGROUND_H