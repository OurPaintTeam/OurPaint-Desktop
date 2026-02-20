#ifndef OURPAINT_RENDERDISTANCE_H
#define OURPAINT_RENDERDISTANCE_H
#include "RenderLines.h"
#include "RenderStyle.h"
#include "ID.h"
#include "GeometricObjects.h"
#include "Scaling.h"

namespace render {
    [[maybe_unused]] void drawDistanceBetweenPoints(QPainter& painter, const QPointF& firstP, const QPointF& secondP, LineStyle& style);
}


#endif //OURPAINT_RENDERDISTANCE_H
