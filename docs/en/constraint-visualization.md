# Constraint visualization

Constraint markers are derived viewport data in App. Core owns the constraint
definitions and sketch geometry; it has no screen positions, marker styles, or
selection regions. RenderScene contains only generic drawing primitives.

## Ownership and data flow

`DocumentView` owns `ViewportStyle` and `app::ConstraintLayout` before the editor
and viewport controller. The controller updates the layout from Sketch, Camera2D,
and `ViewportStyle::constraintMarker`. `RenderSceneBuilder` receives a const
reference to that same layout and copies its strokes into the constraints layer.

Each `ConstraintMarker` identifies a nonempty sorted list of public `ConstraintId`
values and a `GeometryRef` attachment. It contains the final stroke segments and
`hitRadiusPx`. The segments plus that radius describe capsules for future picking,
so picking can use the
actual displayed strokes without repeating placement or symbol construction.
Parallel produces at most one marker per line, collecting all Parallel constraint
IDs involving that line. Other symbols represent one constraint per attachment.
Picking and constraint selection are not yet connected to editor tools.

All layout coordinates, sizes, and hit radii are logical screen pixels with a
top-left origin. Symbol size stays constant under zoom and follows display DPI.
The scene builder alone converts strokes to framebuffer pixels with a bottom-left
origin, including scaling stroke width and edge softness by devicePixelRatio.
Zero stroke width is valid: the renderer can draw the stroke using edge softness.

## Placement and symbols

The layout groups drawable constraints by their attached lines and sorts each
group by constraint type, then smallest represented ID. Unsupported types do not
occupy slots, and multiple Parallel constraints share a single slot on each line.
Ordinary placement uses `t = (i + 1) / (N + 1)` along the line: one marker is at
the midpoint, two at the thirds, three at the quarters. Local symbol construction
is separate from this placement policy.

- Parallel uses two short strokes parallel to the attached line, on opposite
  sides of it.
- Horizontal and Vertical use a horizontal or vertical stroke with three small
  diagonal hatch strokes on one side.
- Perpendicular uses a perpendicular symbol formed by a baseline and a stem at
  a right angle, oriented relative to the attached line and offset to one side.

When a short line cannot fit all symbols with the configured minimum gap, the
layout distributes them across additional rows offset along the line normal.
This is a local spacing policy, not a global collision solver for nearby entities
or window edges. Invalid, nonfinite, or collapsed lines are skipped individually.

## Refresh and future picking

The controller refreshes the layout before mouse/key events reach tools, after
camera resize/zoom, and immediately before building each rendered scene. This
avoids depending on the previous rendered frame when future picking runs before
a redraw. Mouse movement that pans the camera refreshes after the pan and before
tool dispatch. Tool edits are reflected by the refresh preceding the next render
or input event.

There is no persistent geometry mirror or revision cache. A refresh performs
batch queries for constraints and lines and rebuilds derived marker values.
A measured need for caching can later change refresh internals while keeping
the marker contract shared by rendering and picking.

Future constraint picking should read the same layout, return the represented
constraint IDs and attachment, and use segment capsules rather than a single
rectangle covering the base line. Geometry-only tools should keep their current
picking behavior.
Selection/highlighting should use ConstraintId membership so the appearances of
a Parallel constraint act as one constraint. A grouped symbol can expose several
constraint candidates without discarding their identity. New symbols or placement
policies belong in ConstraintLayout, not in the renderer or Cpu2dPicker.
