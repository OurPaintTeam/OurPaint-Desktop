#ifndef OURPAINT_RENDER_VIEWPORTSTYLE_H_
#define OURPAINT_RENDER_VIEWPORTSTYLE_H_

#include "../../../render/RenderScene.h"
#include "constraints/ConstraintMarkerStyle.h"
#include "snap/SnapFeedbackStyle.h"

namespace app {

using namespace render;

struct SelectionRectStyle {
    StrokeStyle stroke;
    Color fill;
};

struct ViewportStyle {
    MarkerStyle baseMarker;
    MarkerStyle selectedMarker;
    MarkerStyle overlayMarker;

    StrokeStyle baseLine;
    StrokeStyle selectedLine;
    StrokeStyle overlayLine;
    StrokeStyle specialLine;

    Color constraintHoverColor;
    ConstraintMarkerStyle constraintMarker;
    SnapFeedbackStyle snapFeedback;

    StrokeStyle baseCircle;
    StrokeStyle selectedCircle;
    StrokeStyle overlayCircle;

    SelectionRectStyle windowSelectionRect;
    SelectionRectStyle crossingSelectionRect;

    Color gridMinorColor;
    Color gridMajorColor;
    Color gridAxisColor;

    static ViewportStyle makeDefault();
};

} // namespace app

#endif
