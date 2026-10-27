# Constraint visualization

Constraint markers are derived viewport data in App. Core owns the constraint
definitions and sketch geometry; it has no screen positions, marker styles, or
selection regions. RenderScene contains only generic drawing primitives.

## Ownership and data flow

`DocumentView` owns `ViewportStyle`, `app::ConstraintPresentationState`, and
`app::ConstraintLayout` before the editor and viewport controller. The controller
updates the layout from Sketch, Camera2D, `ViewportStyle::constraintMarker`,
a const reference to presentation state, and a `render::text::TextMeasurer`.
The current renderer supplies this CPU-only measurement interface from its font
metrics; layout does not call OpenGL. `RenderSceneBuilder` receives a const
reference to that same layout and converts its primitives into the constraints
layer. It does not interpret constraint types or decide where symbols belong.
`SketchEditor` passes a const layout reference and a mutable presentation-state
reference to `CursorTool`. Both remain owned by the view and outlive its tools.

Each `ConstraintMarker` identifies a nonempty sorted list of public `ConstraintId`
values and a deterministic `GeometryRef` attachment. The attachment is a local
representative, not a replacement for the constraint's complete references.
The marker contains final stroke segments, dots, circle outlines, arc outlines,
and labels. A `ConstraintLabel` stores text, center, measured size, nominal font
height and rotation. Its measured rectangle is used for dimension picking.
Its `hitRadiusPx` is the picking distance from any segment, circle or arc outline
(half the pen width plus tolerance). Each `ConstraintDot` carries its own center,
visible `radiusPx`, and circular picking `hitRadiusPx`. Future picking can use
these same values without repeating placement or symbol construction. Dimension
picking is connected to CursorTool; general constraint selection remains future work.

All layout coordinates, sizes, and hit radii are logical screen pixels with a
top-left origin. Glyph lengths, point-ring radii and offsets stay constant under
zoom and follow display DPI. Inset circle/arc radii follow the underlying geometry.
The scene builder alone converts positions to framebuffer pixels with a
bottom-left origin, scaling stroke width, dot radius, and edge softness by
devicePixelRatio. Strokes become `render::LineBatch` values; dots become
`render::MarkerBatch` values grouped by visible radius. Circle and arc outlines
become `render::CircleBatch` and `render::ArcBatch` with the same stroke style.
Labels become centered `render::TextBatch` entries with the same color. The
adapter scales their anchors and font height by DPR and negates their screen
rotation when flipping Y. Text is drawn after the other primitives in the layer.
`OverlayModel::hoveredDimension` and `draggedDimension` provide transient feedback.
The builder batches primitives by normal, hovered or dragged color, matching
ConstraintId membership. Highlighting does not change stroke width or hit regions.
Zero stroke width is
valid: the renderer can draw the stroke using edge softness.

Arc angles are counterclockwise radians from screen-right in a right/up basis:
`center + radius * (cos(angle), -sin(angle))` gives a logical screen position.
The adapter flips the center's Y, but keeps the angles unchanged for the renderer's
bottom-left origin. The circle/arc shader uses scale 1 for Screen layers because
their radii are already in framebuffer pixels; World layers use camera zoom.

## Placement and symbols

The layout dispatches constraints to separate placement families. Line symbols
are grouped by attached line and sorted by constraint type, then smallest
represented ID. Multiple Horizontal, Vertical, Parallel, Perpendicular, or Fix
constraints of the same type share a single marker and slot on each line.
Unimplemented types do not occupy slots.
Ordinary placement uses `t = (i + 1) / (N + 1)` along the line: one marker is at
the midpoint, two at the thirds, three at the quarters. Local symbol construction
is separate from this placement policy.

- Parallel uses two short strokes parallel to the attached line, on opposite
  sides of it.
- Horizontal and Vertical use a horizontal or vertical stroke with three small
  diagonal hatch strokes on one side.
- Perpendicular uses a perpendicular symbol formed by a baseline and a stem at
  a right angle, oriented relative to the attached line and offset to one side.
- Whole-line Fix uses one parallel stroke of `lengthPx`, offset by `offsetPx`
  along the canonical line normal. Reversing the source endpoints does not change
  its side. It shares the same slot and row placement as other line symbols.

When a short line cannot fit all symbols with the configured minimum gap, the
layout distributes them across additional rows offset along the line normal.
This is a local spacing policy, not a global collision solver for nearby entities
or window edges. Invalid, nonfinite, or collapsed lines are skipped individually.

Other Fix forms do not use line slots:

- A point-like reference gets a circle centered on the point, with radius
  `offsetPx`. `Sketch::pointPosition()` supports standalone points, line and arc
  endpoints, and curve centers. The radius is measured from the mathematical point.
- A whole circle gets a concentric outline with radius
  `worldRadius * camera.zoom() - offsetPx` in logical pixels.
- A whole arc gets an arc of the same center, start angle and end angle, using
  the same inward radius offset. Its radius comes from the start point, matching
  base arc rendering during unsolved edits.

Inset curves with a nonpositive radius are skipped rather than clamped or moved
outside the source geometry. No geometry or constraint definition is changed.

Point-to-point Coincident constraints form a graph whose vertices are `GeometryRef` values.
Each connected component produces one marker with every represented constraint
ID, sorted deterministically, and a deterministic representative attachment.
There is one dot at each distinct finite screen position within that component.
Exact duplicate positions share a dot; nearby positions are not merged. If a
component is unsatisfied, its dots remain at the actual reference positions
instead of inventing a midpoint. Independent components remain independent even
when their positions happen to coincide.

Point-on-curve Coincident constraints instead produce independent dots at their
point-like references. Sharing a supporting curve does not put those points in
the same coincidence group.

The geometric-relation family still reserves Tangent, Equal, Concentric,
Symmetric, and Midpoint as explicit stubs emitting no primitives. A new symbol
or placement policy belongs in the family that understands its geometry;
rendering backends remain unaware of constraint semantics.

## Dimensions

`DimensionLayout.cc` implements `appendDimension()` using ordinary helper
functions rather than a class hierarchy. Each dimension becomes one marker with
its public constraint ID. The pipeline resolves geometry, formats and measures
the label, reads optional placement, then emits ordinary primitives.

- `linearAnchors()` resolves two point-like references, a point and a supporting
  line, two parallel supporting lines, or a point and a supporting circle.
  Whole-line `Length` uses the line endpoints. Nonparallel whole-line Distance
  is skipped because the supporting lines intersect.
- `linearDimension()` handles Distance, H/V Distance, Length, Radius and Diameter.
  It projects the two anchors onto the dimension axis passing through the label.
  H/V Distance fixes the axis to the corresponding screen axis. Other distances
  use the direction between anchors. Labels remain readable when references are
  reversed. Extension lines have a small gap at geometry and overrun the dimension
  line. The dimension line is split around the measured text plus padding.
  A label beyond either endpoint extends the dimension line with a short tail;
  a label further along the normal lengthens the extension lines. Short dimensions
  put arrowheads outside the measured span and automatic labels beyond the arrows,
  keeping even a zero distance legible. Explicit label centers remain unchanged.
- Radius and Diameter derive their direction from the center toward the manual
  label, or default to 45 degrees for circles and the CCW midpoint for arcs.
  They reuse the linear builder without extension lines: Radius has one rim arrow,
  Diameter two. An arc is treated as its supporting circle; manually placed labels
  are not restricted to the arc sweep.
- `angularDimension()` uses the intersection of the two directed supporting lines
  and their minor angle in `[0, pi]`. The label's distance from the vertex controls
  the arc radius. The measured label leaves a gap in the arc. A label outside the
  sector gets a leader. Parallel or antiparallel lines have no unique vertex, so
  they use two leaders and a label instead of an invented circular arc.

`makeLabel()` displays the stored target value, not a fresh measurement of possibly
unsolved geometry. Angles are converted from radians to degrees. Formatting uses
six significant digits by default, with no length-unit suffix. The current font
atlas is ASCII-only, so Radius, Diameter and Angle use `R`, `D` and `deg` instead
of Unicode symbols. These are presentation choices, not solver semantics.
`DimensionMarkerStyle` groups text height, offset, gap, arrow size, extension
settings and precision; all lengths are logical pixels.

`OpenGL2dRenderer::measureText()` and `renderText()` use the same glyph advances,
bearings and scale. The latter honors nominal font height, center alignment,
rotation and alpha. A missing font or unsupported glyph yields no metrics, and
the corresponding dimension is omitted rather than guessing its text bounds.
Metrics become available during renderer initialization, before the first scene
build; queries themselves do not need a current graphics context.

Layout support does not imply solver support. `Length` is actively created by
`ConstraintActions` when the Dimension tool selects one whole line. Radius and
Diameter are advertised and validated by Sketch but currently have no cases in
its solver adapter. Whole-line/whole-line Distance is rejected by validation;
some other accepted Distance reference combinations also lack matching adapter
handling. Their visualization is prepared without changing those Core behaviors.

## User placement state

`ConstraintPresentationState::labelPlacements` maps public `ConstraintId` values
to `ConstraintLabelPlacement`. Its `position` is a user-chosen label center in sketch
coordinates, independent of camera zoom and DPI. This input is owned by the view
and survives layout rebuilds; it is separate from both Core constraint definitions
and the derived screen primitives.

Dimension layout consumes these centers now. Missing or nonfinite placements use
automatic placement without changing the map. Automatic positions belong only to
the rebuilt output. Layout rebuilds never clear or modify user placements.
CursorTool writes the desired label center through `Camera2D::screenLogicalToWorld()`.
General deletion cleanup, undo/redo and persistence remain future work.

## Dimension picking and dragging

`Cpu2dPicker::pickDimensionAtScreenLogical()` reads a supplied layout without
querying font metrics or reconstructing geometry. It accepts dimension markers
with one owner and one label. Its value result contains the constraint ID, label
center, hit distance and `Label`/`Body` part. Existing geometry-picking methods
continue to return `GeometryRef` values.

Labels are tested against their measured rectangle after undoing rotation.
Body hits use distance to finite segments or finite arcs, including endpoint
tolerance and arcs crossing zero. All distances and tolerances use logical pixels.
Labels win over bodies; within the same part, the nearest primitive wins, with
ConstraintId breaking exact ties. CursorTool adds the overall priority:
label, geometry point, dimension body, remaining geometry.

An unmodified left press starts a dimension gesture and clears geometry selection.
The tool stores the ID, press position, logical cursor-to-label offset and optional
previous placement. No marker pointer or iterator survives a layout rebuild.
A three-logical-pixel threshold separates clicking from dragging. Once crossed,
each move writes `screenLogicalToWorld(cursor + grabOffset)` as the label center.
The current camera is used at every move, while the grab offset stays in logical
pixels. Labels, dimension strokes, witnesses, arrows and arcs all move this same
anchor; they do not have independent editing roles yet.

Left release applies the final cursor position and commits placement. A click
below the threshold never creates a placement entry. Escape, tool/tab changes
and a subsequent mouse move without the left button cancel the preview, restoring
the previous entry or its absence. If the constraint disappeared during the
gesture, its placement is erased instead of being restored. Cancelled gestures
cannot be committed by a later release. Alt, Shift and other modified presses
retain the existing geometry-tool path.

Moving a dimension neither edits Sketch nor invokes its solver, and emits no
model-change ActionReport. Geometry drag keeps its existing cancellation semantics.
The existing editor cancellation path handles both kinds of gesture.

## Refresh and future constraint selection

The controller refreshes the layout before mouse/key events reach tools, after
camera resize/zoom, and immediately before building each rendered scene. This
avoids depending on the previous rendered frame when picking runs before
a redraw. Mouse movement that pans the camera refreshes after the pan and before
tool dispatch. Tool edits are reflected by the refresh preceding the next render
or input event.

There is no persistent geometry mirror or revision cache. A refresh performs
batch queries for constraints and lines, resolves point-like references through
Sketch, and rebuilds derived marker values.
A measured need for caching can later change refresh internals while keeping
the marker contract shared by rendering and picking.

Future constraint picking should read the same layout, return the represented
constraint IDs and attachment, and test segment capsules, circle/arc outlines,
dot circles and rotated label rectangles rather
than a single rectangle covering the base line. Geometry-only tools should keep
their current picking behavior.
Selection/highlighting should use ConstraintId membership so the appearances of
a constraint act as one constraint across several attachments or dots. A grouped
symbol can expose several constraint candidates without discarding their identity.
New symbols or placement policies belong in ConstraintLayout, not in the renderer
or Cpu2dPicker.

## Extending the layout

Keep the steps as ordinary functions; no per-constraint class hierarchy is needed.

1. Route the type in `ConstraintLayout::rebuild()`. It queries constraints, gathers
   line frames, dispatches families, then finishes coincidence groups and line slots.
2. For a stroked line glyph, add its reference count to `expectedRefCount()`, build
   local segments in `makeGlyph()`, and decide whether same-type markers should
   merge in `glyphIsEntityAgnostic()`. `placeMarkers()` computes bounds and translates
   those segments; it currently handles stroked line glyphs only.
3. For a point or whole-curve annotation, resolve its geometry in a family helper
   and append a `ConstraintMarker` with final logical screen primitives. Keep its
   public IDs and attachment so future selection can map the visual back to Sketch.
4. Reuse `strokes`, `dots`, `circles`, `arcs` and `labels` where possible. Adding a new primitive
   requires a value type in `ConstraintLayout.h` and conversion in
   `RenderSceneBuilder::buildConstraintsMarkers()`. The builder must stay independent
   of constraint types. Backend changes are needed only for generic rendering support.
5. Extend dimensions in `DimensionLayout.cc`: resolve references, use `makeLabel()`,
   and reuse `linearDimension()` when the geometry permits it. Keep optional user
   placement as const input and automatic placement as derived output. New units
   or notation belong in label formatting; picking and dragging belong in tools.

`ConstraintLayoutGTEST.cc` and `DimensionLayoutGTEST.cc` cover geometry, grouping,
line slots, pixel offsets, measured text gaps, explicit placement, degenerate
dimensions and zoom/DPI independence. A deterministic `TestTextMeasurer` keeps
these tests independent of installed fonts and a graphics context.
`DimensionInteractionGTEST.cc`, registered in the CursorToolGTEST target, covers
rotated-label and arc picking, hit priorities, gesture offsets, commit/cancel,
camera changes, deleted targets and preservation of Sketch geometry.

## Decisions to revisit

- Reconcile Sketch capability reporting, validation and solver adapters before
  exposing the remaining dimension commands. Do not infer semantic support from
  this presentation module.
- Text currently uses one fixed bitmap font and a minimal measurement interface.
  Move font metrics into a shared CPU font service when multiple fonts, Unicode,
  shaping or sharper HiDPI rasterization are needed. `TextStyle` also has unused
  font/point-size fields and two competing color representations.
- Decide document units, numeric formatting and dimension notation explicitly.
  The current target-value labels and ASCII prefixes are intentionally basic.
- A world-space label center is sufficient for this placement contract, but does
  not follow an entity translated independently. Relative attachments, undo/redo,
  persistence and cleanup of deleted constraint IDs require a separate decision.
- Placement is local: there is no collision avoidance against other annotations,
  geometry or viewport edges. A manually placed label close to a witness endpoint
  can overlap an arrowhead. Angular sector choice, arc-radius leaders and crowded
  dimensions should be refined as interaction requirements become concrete.
- Supporting a dedicated normal-only drag of the dimension line will require
  semantic roles for strokes (dimension line, witness, arrow), rather than
  inferring roles from segment orientation in the picker or cursor tool.
