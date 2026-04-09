#ifndef OURPAINT_APPLICATION_TOOL_ID_H_
#define OURPAINT_APPLICATION_TOOL_ID_H_

enum class ToolId {
    Cursor,
    Size,

    Point,
    Line,
    Polyline,
    InfiniteLine,

    CircleByDiameter,
    CircleTwoPoints,
    EllipseThreePoints,

    ArcByRadius,
    ArcByDiameter,
    ArcByThreePoints,

    ConstraintPointLineDistance,
    ConstraintPointOnLine,
    ConstraintPointPointDistance,
    ConstraintCoincidentPoints,
    ConstraintLineCircleDistance,
    ConstraintLineOnCircle,
    ConstraintLineInCircle,
    ConstraintParallelLines,
    ConstraintPerpendicularLines,
    ConstraintAngleBetweenLines
};

#endif // ! OURPAINT_APPLICATION_TOOL_ID_H_

