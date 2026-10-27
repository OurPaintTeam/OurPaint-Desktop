#include "ViewportStyle.h"

namespace app {

ViewportStyle ViewportStyle::makeDefault() {
    ViewportStyle style;

    {
        // base point
        style.baseMarker.color = {63.0f / 255.0f, 72.0f / 255.0f, 204.0f / 255.0f, 1.0f};
        style.baseMarker.radius = 2.0f;
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
        style.baseLine.widthPx = 0.0f;
        style.baseLine.edgeSoftnessPx = 1.5f;

        // selected line
        style.selectedLine.color = {0.0f, 1.0f, 1.0f, 1.0f};
        style.selectedLine.widthPx = 3.0f;
        style.selectedLine.edgeSoftnessPx = 2.0f;

        // overlay line
        style.overlayLine = style.baseLine;

        // special line
        style.specialLine.color = {0.5f, 0.5f, 0.5f, 0.4f};
        style.specialLine.widthPx = 1.0f;
    }

    {
        style.constraintMarker.stroke.color = {1.0f, 127.0f / 255.0f, 39.0f / 255.0f, 1.0f};
        style.constraintMarker.stroke.widthPx = 0.5f;
        style.constraintMarker.stroke.edgeSoftnessPx = 1.5f;
        style.constraintMarker.point.color = style.constraintMarker.stroke.color;
    }

    {
        style.snapFeedback.marker.color = {0.8f, 0.1f, 0.6f, 1.0f};
        style.snapFeedback.marker.widthPx = 1.5f;
        style.snapFeedback.marker.edgeSoftnessPx = 1.0f;

        style.snapFeedback.referencePoint.color = style.snapFeedback.marker.color;
        style.snapFeedback.referencePoint.radius = 3.0f;
        style.snapFeedback.referencePoint.edgeSoftnessPx = 1.0f;

        style.snapFeedback.referenceCurve.color = style.snapFeedback.marker.color;
        style.snapFeedback.referenceCurve.widthPx = 2.0f;
        style.snapFeedback.referenceCurve.edgeSoftnessPx = 1.0f;

        style.snapFeedback.guide.color = {0.6f, 0.2f, 0.6f, 0.8f};
        style.snapFeedback.guide.widthPx = 1.0f;
        style.snapFeedback.guide.edgeSoftnessPx = 1.0f;
    }

    {
        // base circle
        style.baseCircle.color = {0.0f, 0.0f, 0.0f, 1.0f};
        style.baseCircle.widthPx = 0.0f;
        style.baseCircle.edgeSoftnessPx = 1.5f;

        // selected circle
        style.selectedCircle.color = {0.0f, 1.0f, 1.0f, 1.0f};
        style.selectedCircle.widthPx = 3.0f;
        style.selectedCircle.edgeSoftnessPx = 2.0f;

        // overlay circle
        style.overlayCircle = style.baseCircle;
    }

    {
        // selection rect
        style.selectionRectStroke.color = {0.2f, 0.5f, 1.0f, 0.9f};
        style.selectionRectStroke.widthPx = 2.0f;

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
