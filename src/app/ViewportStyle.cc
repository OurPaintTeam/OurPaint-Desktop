#include "ViewportStyle.h"

namespace app {

ViewportStyle ViewportStyle::makeDefault() {
    ViewportStyle style;

    {
        // base point
        style.baseMarker.color = {63.0f / 255.0f, 72.0f / 255.0f, 204.0f / 255.0f, 1.0f};
        style.baseMarker.radius = 1.5f;
        style.baseMarker.edgeSoftnessPx = 1.5f;

        // selected point
        style.selectedMarker.color = {0.0f, 1.0f, 1.0f, 1.0f};
        style.selectedMarker.radius = 3.0f;
        style.selectedMarker.edgeSoftnessPx = 3.0f;

        // overlay point
        style.overlayMarker = style.baseMarker;
    }

    {
        // base line
        style.baseLine.color = {0.0f, 0.0f, 0.0f, 1.0f};
        style.baseLine.width = 0.0f;
        style.baseLine.edgeSoftnessPx = 2.0f;

        // selected line
        style.selectedLine.color = {0.0f, 1.0f, 1.0f, 1.0f};
        style.selectedLine.width = 3.0f;
        style.selectedLine.edgeSoftnessPx = 2.0f;

        // overlay line
        style.overlayLine = style.baseLine;

        // special line
        style.specialLine.color = {0.5f, 0.5f, 0.5f, 0.4f};
        style.specialLine.width = 1.0f;
    }

    {
        // base circle
        style.baseCircle.color = {0.0f, 0.0f, 0.0f, 1.0f};
        style.baseCircle.width = 0.0f;
        style.baseCircle.edgeSoftnessPx = 2.0f;

        // selected circle
        style.selectedCircle.color = {0.0f, 1.0f, 1.0f, 1.0f};
        style.selectedCircle.width = 3.0f;
        style.selectedCircle.edgeSoftnessPx = 2.0f;

        // overlay circle
        style.overlayCircle = style.baseCircle;
    }

    {
        // selection rect
        style.selectionRectStroke.color = {0.2f, 0.5f, 1.0f, 0.9f};
        style.selectionRectStroke.width = 1.0f;

        style.selectionRectFill = {0.2f, 0.5f, 1.0f, 0.12f};
    }

    {
        style.gridMinorColor = {0.85f, 0.85f, 0.85f, 1.0f};
        style.gridMajorColor = {0.75f, 0.75f, 0.75f, 1.0f};
        style.gridAxisColor = {0.45f, 0.45f, 0.45f, 1.0f};
    }

    return style;
}

} // namespace app