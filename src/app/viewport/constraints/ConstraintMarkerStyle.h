#ifndef OURPAINT_APPLICATION_CONSTRAINT_MARKER_STYLE_H_
#define OURPAINT_APPLICATION_CONSTRAINT_MARKER_STYLE_H_

#include "RenderScene.h"

namespace app {

// Visual style of a dimension line (the line with arrows and a numeric label
// drawn to show a measured distance/length).
struct DimensionMarkerStyle {
    // Font height of the numeric label, in logical screen pixels.
    double textHeightPx = 14.0;
    // Perpendicular distance from the measured geometry to the dimension line.
    double offsetPx = 30.0;
    // Gap in the dimension line behind the label so the text stays readable.
    double textGapPx = 4.0;
    // Length of the arrowheads at the ends of the dimension line.
    double arrowLengthPx = 7.0;
    // Gap between the measured geometry and the start of the extension line.
    double extensionGapPx = 3.0;
    // How far the extension lines run past the dimension line (the "tail").
    double extensionOverrunPx = 4.0;
    // Number of digits after the decimal point when formatting the value.
    int precision = 6;
};

// All marker sizes and stroke widths are in logical screen pixels.
struct ConstraintMarkerStyle {
    // Base stroke style shared by all marker outlines (color, width, dash).
    render::StrokeStyle stroke;
    // Style of the small point marker itself: {color, radius-size, edge softness}.
    render::MarkerStyle point{{}, 1.0f, 0.5f};
    // Length of a marker segment
    double lengthPx = 20.0;
    // Also the Fix point-ring radius and the inward offset for fixed circles/arcs.
    double offsetPx = 5.0;
    // Length of a single hatch stroke in a hatched marker.
    double hatchLengthPx = 6.0;
    // Spacing between hatch strokes in a hatched marker.
    double hatchSpacingPx = 6.0;
    // Minimum gap between a marker and geometry (or between markers) to avoid visual overlap.
    double minGapPx = 6.0;
    // Mouse hit-test tolerance around a marker, in pixels.
    double hitTolerancePx = 4.0;

    // Style of dimension lines.
    DimensionMarkerStyle dimension;
    // Interaction changes color only, so hit regions and layout remain stable.
    render::Color hoveredColor{0.95f, 0.25f, 0.05f, 1.0f};
    render::Color draggedColor{0.0f, 0.6f, 0.8f, 1.0f};
};

}  // namespace app

#endif  // OURPAINT_APPLICATION_CONSTRAINT_MARKER_STYLE_H_
