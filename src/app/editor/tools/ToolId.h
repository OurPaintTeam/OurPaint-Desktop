#ifndef OURPAINT_APPLICATION_TOOL_ID_H_
#define OURPAINT_APPLICATION_TOOL_ID_H_

enum class ToolId {
    // First group
    Cursor,

    // Second group: primitives
    Point,
    Line,
    Polyline,
    InfiniteLine,

    CircleByRadius,
    CircleByDiameter,
    CircleByTwoPoints,
    CircleByThreePoints,
    CircleTangentTwoLines,
    CircleTangentThreeLines,

    ArcByRadius,
    ArcByDiameter,
    ArcByThreePoints,

    CubicBezier,

    // third group constraints
    Distance,
    HorizontalDistance,
    VerticalDistance,
    Angle,

    Coincident,
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    Tangent,
    Equal,
    Fix,
    Concentric,
    Midpoint,
    Collinear,
    Symmetric
};

#endif // ! OURPAINT_APPLICATION_TOOL_ID_H_

