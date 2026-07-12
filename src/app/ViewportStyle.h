#ifndef OURPAINT_RENDER_VIEWPORTSTYLE_H_
#define OURPAINT_RENDER_VIEWPORTSTYLE_H_

#include "../render/render/RenderScene.h"

namespace app {

using namespace render;

struct ViewportStyle {
    MarkerStyle baseMarker;
    MarkerStyle selectedMarker;
    MarkerStyle overlayMarker;

    StrokeStyle baseLine;
    StrokeStyle selectedLine;
    StrokeStyle overlayLine;
    StrokeStyle specialLine;

    StrokeStyle baseCircle;
    StrokeStyle selectedCircle;
    StrokeStyle overlayCircle;

    StrokeStyle selectionRectStroke;
    Color selectionRectFill;

    Color gridMinorColor;
    Color gridMajorColor;
    Color gridAxisColor;

    static ViewportStyle makeDefault();
};

} // namespace app

#endif