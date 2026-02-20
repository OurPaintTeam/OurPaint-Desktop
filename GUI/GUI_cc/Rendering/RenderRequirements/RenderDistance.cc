#include "RenderDistance.h"

namespace render {
    [[maybe_unused]] void drawDistanceBetweenPoints(QPainter& painter, const QPointF& firstP, const QPointF& secondP, LineStyle& style){
        drawScaleLineWithLeftRightArrow(painter,firstP,secondP,style);
    }

}