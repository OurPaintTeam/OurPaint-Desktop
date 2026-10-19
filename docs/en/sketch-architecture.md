# Core Sketch architecture and API

`core::sketch::Sketch` represents one parametric 2D sketch in local coordinates.
Include `sketch/Sketch.h` and link `OurPaint::Sketch`. The current `Document`
owns a Sketch through `std::unique_ptr`; other owners can use the same API. A
future sketch feature may add a support plane and a placement transform around
it. Sketch has no document identity, world transform, or persistence format.

## Responsibilities and ownership

```text
Document / App / tools / render scene builder
                    |
                  Sketch
                    |
               Sketch::Impl
                    |
                DCMManager
```

Sketch directly uses DCM as its only solver. Its private nonpolymorphic Impl,
defined in Sketch.cc, owns DCMManager and its model resources. Sketch owns the
implementation lifetime, public ID allocators, validation, and state-replacement
policy. Sketch.h exposes neither DCM types nor solver handles. There is no
backend interface, adapter selection, live geometry mirror, or type index.
Temporary batch records and query values are detached copies, never another
authoritative mutable model.

Impl retains public-to-private handle mappings, immutable entity kinds,
construction flags, public constraint definitions, and resource ownership
metadata. Coordinates come directly from DCM storage. Constraint definitions
describe equations, not a second current geometry model. Internal point-reference
helpers live in sketch/PointElements.h and do not expose DCM handles.

Sketch handles local geometry, edits, constraints, reference interpretation,
queries, solving, diagnostics, and explicit state transfer. Qt, rendering and
graphics APIs, operating-system APIs, selection, screen-space picking, gestures,
undo/redo, clipboard integration, document management, and persistence remain
outside it. Core can therefore serve a future WebAssembly implementation. This
boundary keeps App tools independent of DCM handles.

App's derived viewport marker model and its shared rendering/picking contract
are described in [Constraint visualization](constraint-visualization.md).

Calls on one instance, including const queries and destruction, must be
externally serialized. Returned geometry, vectors, diagnostics, and snapshots
own their data and survive later edits or Sketch destruction. Editing a returned
value does not edit Sketch. Independent instances own independent models and ID
allocators.

## Geometry, identity, and results

Coordinates and lengths use one consistent unit chosen by the owner; Sketch does
not convert units. Angles use radians. `SketchGeometry` is a variant of:

| Value | Fields and edit validation |
| --- | --- |
| `Point2` | Finite `position` |
| `Line2` | Finite distinct `start` and `end`; finite positive length |
| `Circle2` | Finite `center`; finite positive `radius` |
| `Arc2` | Finite `center`, `start`, `end`; distinct endpoints, positive finite radii equal within relative tolerance `1e-9` |

An arc is the counterclockwise start-to-end sweep, strictly between zero and one
full turn. Unsolved query values may have unequal radii after constraint
projection or a failed solve. Queries do not repair the current iterate.
State transfer accepts unequal arc radii, but rejects nonfinite or degenerate
geometry. `entityKind(geometry)` returns its variant kind.

`EntityId` and `ConstraintId` are distinct wrappers around Core ID values.
`get()` exposes the integer; equality and ordering compare values. Nonpositive
IDs are invalid; positive absent IDs produce NotFound. IDs are local to a Sketch
lineage, not globally unique or interchangeable between sketches. Each allocator
is monotonic and never recycles deleted, cleared, or allocated but unsuccessfully
inserted IDs. Updates, solving, and dragging retain identity. Snapshots carry
allocation watermarks, preserving deletion history through transfer. Exhaustion
produces BackendFailure before overflow.

`SketchEntity` contains `id`, owning `geometry`, and `construction`. Construction
is consumer metadata, not a different equation or exclusion from queries, counts,
or solving. Creation helpers default it to false. Geometry updates preserve it
unless an explicit optional value is supplied.

`GeometryRef` identifies an entity and `SubElement`. Valid point-like references
and their within-entity query order are:

| Kind | Point elements |
| --- | --- |
| Point | `Whole` |
| Line | `Start`, `End` |
| Circle | `Center` |
| Arc | `Start`, `End`, `Center` |

Curve Whole references identify curves, not points. Curve endpoints/centers are
private owned resources, not standalone Point entities. Coincident coordinates
or Coincident constraints never merge public reference identities.
`PointElement { GeometryRef ref; Vec2 position; }` exposes references and current
coordinates, not solver topology or coincidence groups.

`Result<T>` contains an owning value or `SketchError { code, message }`;
`Status` is `Result<void>`. Test its explicit boolean conversion before calling
`value()` or `error()`; only the active alternative may be accessed. Successful
`Result<SolveDiagnostics>` still requires checking `diagnostics.status`.

| Error | Meaning |
| --- | --- |
| InvalidArgument | Invalid geometry, enum, ID, incompatible reference, dimension, kind change, duplicate target, or malformed snapshot |
| NotFound | Positive entity/constraint ID does not exist |
| Unsupported | Operation disabled or exact constraint form unsupported |
| BackendFailure | DCM/resource error or identity/handle exhaustion; ordinary edits do not promise rollback |
| SolveFailure | Reserved category; the current implementation does not use it for numerical nonconvergence |

InvalidArgument, NotFound, and Unsupported preflight failures leave the model
unchanged. DCM failure after mutation begins can leave a partial edit.
Result is not a general allocation-exception/out-of-memory recovery mechanism.
No API promises rollback for arbitrary system failures.

## Desktop selection integration

The desktop picker and SelectionModel use GeometryRef throughout. Whole selects
one entity; Start, End and Center select its point elements independently.
Selecting a curve does not implicitly select its markers, and selecting a marker
does not select its curve. Coincident points retain distinct references.

Cpu2dPicker queries pointElements(All) for point hits and entities for curve hits.
Rectangle selection includes each point inside the rectangle and each curve
intersecting it. RenderSceneBuilder draws base/selected markers from pointElements
and highlights a curve only when its Whole reference is selected. SelectionModel
deduplicates identical references while retaining selection order.

CursorTool captures selected point positions and the cursor position at press.
Whole curves expand to their point elements; overlapping selections are
deduplicated. Every move sends absolute targets (initial positions plus cursor
displacement since press) through dragPoints once, rather than updating geometry
and then calling solve. This keeps targets stable after solver projection and
allows partial arc dragging without submitting a noncircular geometry update.
A single Circle Whole selection edits the radius and calls solve; Circle Center
drags the center instead. Solve diagnostics are checked for convergence. The first
failed step stops further mutation until release/cancellation; one gesture report
retains the failure and any geometry changes. Detached entity queries at the first
actual step and at gesture end identify changes (including solver movement of
unselected geometry) and suppress successful unchanged gestures. They are not
rollback snapshots and are not performed on every mouse move. Release and
cancellation discard gesture targets; cancellation does not restore geometry.
Shift rectangle selection combines the initial selection
with the current rectangle hits, without retaining hits from earlier rectangles.

### Constraint interaction in App

Toolbar requests are translated into `ConstraintRequest` values by the Qt binder
and forwarded through UIController to the requested document's SketchEditor.
SketchEditor is a coordinator, not an IInteractionTool. Its key dispatcher maps
the constraint shortcuts to the same requests and forwards other keys to the
active tool. Escape first cancels pending tool input, then exits an empty tool.

ConstraintActions owns the shared preparation/application path over a borrowed
Sketch. Preparation is read-only: it checks compatible partial input, exact
arity, numeric values, Sketch capabilities and complete definition support.
A ready preselection is applied immediately without changing the active tool;
an empty or compatible partial selection starts ConstraintTool or DimensionTool.
Invalid/ambiguous selections and unsupported operations return without changing
the mode. No subset is silently chosen from a larger selection.

ConstraintTool collects geometric-relation inputs; DimensionTool remains a
separate interaction tool for dimensional inputs. Both borrow ConstraintActions,
the picker and OverlayModel. Their pending references are detached from shared
SelectionModel and highlighted through `constraintRefs_`; successful application
clears those inputs and retains the active mode for repetition. A whole line
receives Length, while two point-like references receive Distance, including
line endpoints. ConstraintRequests use sketch units and radians.

Line-only relations pick whole lines; Equal/Tangent pick whole curves without
endpoint priority. Coincident/Fix pick point elements, while Dimension retains
point priority so choosing a line endpoint creates a point-distance input.

ConstraintActions applies coincidence with Midpoint placement and returns an
owning App `ActionReport`. Preparation preserves typed rejection reasons and
owning Core query/support errors. Confirmed insertion IDs remain separate from
solve diagnostics/errors: a failed solve does not undo insertion. Both interaction
tools clear pending references after confirmed or potentially partial insertion,
so solver failure cannot leave an inserted constraint pending for an accidental
retry. Transactions, rollback and undo/redo remain unimplemented; failed edits
can retain model changes.
The FixUnfix toolbar button currently creates point Fix only; unfix is not
implemented. Buttons for relations absent from the public Core API report
unsupported operations, and exact DCM support controls Equal/Tangent execution.
Toolbar visual activation is still controlled by the UI submodule independently
of successful application or the editor's active mode.

OverlayModel separates `clearPreview()` from `clearSelection()`. Tool preview
cleanup preserves shared selection; `clear()` explicitly clears both.

### Action reporting and UI notifications

`src/app/editor/ActionReport.h` is a small detached outcome value, not a command
or event bus. It owns the operation kind, requested batch counts, confirmed
entity/constraint IDs, rejection reason, operation error, solve error and optional
`SolveDiagnostics`. `ModelChange` distinguishes Unchanged, Changed and
PotentiallyChanged. Ordinary DCM mutation failure is conservatively potentially
changed; read-only preparation errors are unchanged. Confirmed insertion stays
changed even if subsequent solving fails.

Tool mouse/key handlers return `std::optional<ActionReport>`; previews, partial
input, empty deletion/paste and unchanged successful dragging return no report.
Cancellation returns a handled flag and an optional completed gesture report.
ViewportController routes mouse events through SketchEditor, which delivers all
tool, keyboard and toolbar reports through one synchronous
callback. Ready toolbar/key constraints use the same ConstraintActions path as
interactive tools and Alt-click coincidence. Batch deletion/paste returns one
summary; paste retains confirmed entity IDs and each confirmed constraint ID
even if a later insertion/solve fails. Sketch does not expose successful-prefix
IDs for a failed entity batch, so reports do not invent those IDs.

`src/app/ui/ActionReportPresenter` owns translation and message formatting and
calls the existing text-based `UI::ProjectManager::addNotification`. For example,
confirmed insertion with failed diagnostics reads “Constraint added, but the
solver did not converge.” Convergence is never described as fully constrained;
missing diagnostics remain unavailable. Core and tools contain no notification
widgets or Qt values. Technical error messages remain owned by the report.

Application owns DocumentViews with `unique_ptr`. Each editor callback borrows
UIController and its originating Document only for synchronous delivery. The
presenter reads that document's current name at delivery instead of the active
tab and includes it in the text for the notification API's shared-window fallback.
UIController disconnects callbacks and removes viewport sinks before close
or shutdown destroys views; closing drops pending feedback rather than routing
it to another document. Parameter dialogs retain a document identity and resolve
only live views, so renames keep the request associated and closed views receive
no delayed edit. The binder is destroyed before UIController during shutdown.

## Creation, capabilities, and source compatibility

| Function | Result and behavior |
| --- | --- |
| `defaultBackend()` | Always BackendKind::Dcm; constant-time |
| `availableBackends()` | Owning vector containing only BackendKind::Dcm; bounded constant cost |
| `create(backend = defaultBackend())` | Result of unique_ptr owning an empty DCM Sketch; unknown enum InvalidArgument, construction exceptions BackendFailure |
| `~Sketch()` | Releases Impl and DCM resources; detached values survive |
| `backendKind() const` | Always BackendKind::Dcm; constant-time, read-only |
| `capabilities() const` | Detached coarse support flags; constant-time, read-only |
| `supportsConstraint(definition) const` | Status after common validation and exact support preflight; no edit, ID allocation, or solve |

Sketch is noncopyable and not movable; transfer state explicitly. Capability
`supports(EntityKind)` and `supports(ConstraintType)` safely test coarse arrays,
returning false for unknown enums. Other flags cover updates/removals, solve,
drag, and diagnostics. Coarse type support does not replace exact preflight.

BackendKind contains only Dcm. The selection methods remain for Core source
compatibility; App and UI do not select, switch, or display solver implementations.
CMake always links Sketch to DCM and has no solver-selection options or definitions.

## Entity and point-element queries

Queries are read-only and return current, possibly unsolved values. Entity and
constraint collections use ascending public IDs, even after importing an
arbitrarily ordered snapshot. Point elements use ascending entity ID and the
within-entity order above. There are no borrowed views or stable iterators.

Let N be public entities, M public constraints, K selected entities, B batch
inputs, and P total private records. Metadata scanning is O(N). Each geometry
read uses a bounded number of native map lookups, O(log(P + 1)). Bulk geometry
queries therefore cost O(N + K log(P + 1)), not a type-index lookup. Each entity
has at most three point elements.

| Function | Result, errors, and cost |
| --- | --- |
| `entity(id) const` | Result of SketchEntity; positive existing ID required. InvalidArgument/NotFound/native read failures. O(log(N + 1) + log(P + 1)) |
| `entities() const` | Result of vector of all SketchEntity values, including metadata. Empty success; O(N log(P + 1)) time, O(N) result space |
| `entities(kind) const` | Same result, filtered in Impl before geometry reads. Unknown kind is InvalidArgument; empty matches succeed. O(N + K log(P + 1)) time, O(K) result space |
| `points()`, `lines()`, `circles()`, `arcs()` const | Delegate to entities(kind), with identical errors/ownership/order/cost. points() includes only standalone Point entities |
| `pointPosition(ref) const` | Result of Vec2; existing entity and compatible point-like sub-element required. Nonpositive ID/incompatible sub-element is InvalidArgument, absent entity NotFound. Reads one entity: O(log(N + 1) + log(P + 1)) |
| `pointElements() const` | Result of vector of PointElement; union of standalone points and curve sub-elements, equivalent to scope All |
| `pointElements(scope) const` | Same result; StandalonePoints, CurveSubElements, or All. Unknown scope is InvalidArgument. Impl scans metadata and reads only matches without a full entity collection: O(N + K log(P + 1)) time, O(K) result space |
| `pointElements(entityId) const` | Same result for that entity's valid elements. InvalidArgument/NotFound/native read errors as entity(). O(log(N + 1) + log(P + 1)); at most three values |
| `entityCount() const` | size_t, public records, O(1), no geometry read/allocation |
| `entityCount(kind) const` | Result of size_t, public count by kind; unknown enum InvalidArgument. O(N) metadata scan, constant extra space |
| `constraintCount() const` | size_t, public constraints, O(1), excludes intrinsic equations, anchors, and private resources |

Native query failures may produce BackendFailure; C++ allocation exceptions are
subject to the general limitation above. Queries never invoke solve or enforce
geometry validity, even when constraints are dirty or a current iterate is invalid.

```cpp
using namespace core::sketch;
auto queried = sketch.lines();
if (queried) {
    for (const SketchEntity& entity : queried.value()) {
        const Line2& line = std::get<Line2>(entity.geometry);
        // Use entity.id, entity.construction, line.start, and line.end.
    }
}

auto elements = sketch.pointElements(); // Standalone points plus curve elements.
if (elements) {
    for (const PointElement& point : elements.value()) {
        // Use point.ref for constraints/drag, point.position for display.
    }
}
```

## Geometry edits and batches

Ordinary mutations defer numerical solving. Hard fixed/coincident projection can
alter geometry and neighbors as described below. Updates are initial guesses,
not rigid transformations or solver group drag.

| Function | Result and side effects |
| --- | --- |
| `addEntity(geometry, construction = false)` | Result of EntityId; validates geometry/support/capacity, allocates ID, inserts native resources and any intrinsic arc equation |
| `addPoint(p)` | Same result; Point2 with construction false |
| `addLine(start, end)` | Same result; nondegenerate Line2 with construction false |
| `addCircle(center, radius)` | Same result; positive-radius Circle2 with construction false |
| `addArc(center, start, end)` | Same result; circular counterclockwise Arc2 with construction false; note parameter order |
| `updateEntity(id, geometry, construction = std::nullopt)` | Status; same-kind only. Omitted/nullopt construction preserves metadata; explicit bool remains supported. ID and referencing constraints survive |
| `setConstruction(id, construction)` | Status; changes existing metadata only. No geometry resubmission/read, projection, equation change, or solve. O(log(N + 1)) |
| `removeEntity(id)` | Status; removes entity, its private resources/intrinsic constraints, and all public constraints referencing Whole or any sub-element |
| `addEntities(std::span<const EntityCreation> input)` | Result of vector of EntityId; inputs contain geometry and construction defaulting false; output IDs match input order |
| `updateEntities(std::span<const EntityUpdate> input)` | Status; inputs contain ID, same-kind geometry, and optional construction. Omission preserves pre-batch metadata |
| `moveEntities(std::span<const EntityId> ids, Vec2 offset)` | Status; translates current geometry proposals by a finite local offset through updateEntities. Preserves IDs, construction and constraints; circle radii stay unchanged. Empty batches succeed; invalid/duplicate IDs or invalid resulting geometry fail before mutation. No numerical solve; DCM projection rules still apply |
| `removeEntities(std::span<const EntityId> ids)` | Status; removes targets in order, cascading their public constraints |
| `clear()` | Status; removes all public entities/constraints and associated private resources in place. Retains Impl and public watermarks; empty/repeated clear succeeds. No solve/state replacement |

Creation rejects invalid geometry with InvalidArgument, unsupported kinds with
Unsupported. Updates additionally reject kind changes/duplicate targets with
InvalidArgument, missing IDs with NotFound, and disabled editing with Unsupported.
Removal and metadata edits require positive existing IDs and corresponding edit
support. Clear is supported. Native errors/capacity failures produce
BackendFailure. Singles use the same geometry batch validation/mutation path.

### Batch validation and failure boundaries

Sketch validates the complete input before ID allocation or native mutation:
geometry validity, same-kind updates, existing IDs, duplicate update/removal
targets, operation support, and public creation ID capacity. Repeated geometry
in creation is valid; creation has no target IDs. Empty batches succeed without
native mutation or advancing watermarks.

Preflight InvalidArgument, NotFound, and Unsupported never partially apply the
batch. Public ID exhaustion also fails before allocation. Following preflight,
mutation proceeds in input order and stops at the first DCM error. There is
no rollback on BackendFailure: earlier edits can remain, and a failing native
operation can itself have partial effects. Query the model to recover. Creation
allocates the whole batch's IDs before native insertion; all stay consumed if insertion
fails. Its error result does not list successfully inserted IDs; entities() can
inspect the remaining public model. Allocation/system exceptions have no separate
transactional recovery guarantee.

No snapshots, Impl recreation, persistent point mirror, or transaction log
are used for batches. Temporary records/descriptors use O(B) space. Creation
prevalidation costs O(B); update/removal duplicate detection costs
O(B log(B + 1)) plus B entity reads. Native edits include handle lookup and graph
maintenance costs, with no constant-time-per-input guarantee.

DCM uses native updateFigures for mixed updates. It prevalidates descriptors,
resolves fixed/coincident state once, applies input order, and synchronizes
coincident coordinates once. GLOBAL mode does not numerically solve. Within a
line Start precedes End; within an arc Start precedes End precedes Center. The
last proposal to an unfixed coincidence group wins, including distinct references
inside one input geometry. A group with Fix ignores coordinate proposals and
retains native fixed representative/anchor behavior. Contradictory Fix targets
are handled by explicit solve, not batch validation. Projection can make curves
degenerate or arcs noncircular.

Addition/removal use direct Impl loops: private endpoints/centers are
exclusively owned, and neither operation numerically solves. Removal scans public
constraints per entity, O(B M) reference visits in the worst case. DCM may also
rebuild component/equation caches after removals; it does not recreate Impl.

Clear calls DCMManager's native clear to release model/caches and clears public
records. Its cost is proportional to private model/cache size. Counts become
zero on success. Clear has no DCM-failure
rollback guarantee.

```cpp
const std::array<EntityCreation, 2> input{{
    {Line2{{0, 0}, {5, 1}}, false},
    {Circle2{{8, 3}, 2}, true},
}};
auto added = sketch.addEntities(input);
if (added) {
    // Several edits/constraints can precede this one numerical solve.
    auto solved = sketch.solve();
    if (solved && solved.value().status == SolveStatus::Converged) {
        // Consume solved geometry.
    }
}

if (added) {
    // Metadata edit without geometry resubmission or solving.
    Status changed = sketch.setConstruction(added.value()[0], true);
    if (!changed) { /* Handle changed.error(). */ }
}

Status cleared = sketch.clear(); // Same Impl; IDs stay above consumed values.
if (!cleared) { /* Inspect the current model and cleared.error(). */ }
```

For update batches, use EntityUpdate{id, geometry, std::nullopt} to preserve
construction or an explicit bool, call updateEntities(updates), then inspect its
Status before the one explicit solve.

## Constraint API

ConstraintDefinition contains type, refs, optional dimension value, and optional
fixedPosition. References must exist and match these forms. Two-reference
constraints require distinct GeometryRefs even when coordinates coincide. Only
dimensional types accept value; only Fix requires finite fixedPosition.
Unexpected/missing fields are InvalidArgument.

| Constraint | References and values |
| --- | --- |
| Coincident | Two point-like refs |
| Horizontal, Vertical | One whole line |
| Parallel, Perpendicular | Two whole lines |
| Distance | Two point-like refs; finite nonnegative length including zero |
| HorizontalDistance, VerticalDistance | Two point-like refs; second-minus-first X/Y coordinate equals a finite nonnegative value, including zero |
| Length | One whole line; finite positive length |
| Angle | Two whole directed lines; unsigned finite angle in `[0, pi]` radians |
| Equal | Two whole lines, or two whole circular curves (Circle/Arc) |
| Radius, Diameter | One whole circular curve; finite positive length |
| Tangent | Whole line/circular curve, or two circular curves; external tangency contract currently unsupported by DCM |
| Fix | One point-like ref with explicit finite fixedPosition; target survives edits, failed solves, and transfer |

| Function | Result, errors, side effects, and cost |
| --- | --- |
| `supportsConstraint(definition) const` | Status; validates fields/refs, then exact support. InvalidArgument/NotFound/Unsupported/native query failures. At most two entity reads; no mutation |
| `addConstraint(definition)` | Result of ConstraintId; preflights/capacity-checks, allocates ID, installs equations/private resources. No solve; allocated ID remains consumed after DCM failure |
| `addCoincident(a, b, placement = CoincidentPlacement::Midpoint)` | Result of ConstraintId; prepares the selected coincidence groups, then inserts Coincident. No numerical solve. Preserve delegates to ordinary addConstraint |
| `updateConstraint(id, definition)` | Status; existing ID, valid supported replacement, and update capability required. Retains public ID; may replace only translated equations/anchors, no numerical solve |
| `removeConstraint(id)` | Status; requires existing positive ID/removal support; removes equations/private anchors without numerical solve |
| `constraint(id) const` | Result of SketchConstraint with owning definition. Nonpositive ID InvalidArgument, absent ID NotFound. O(log(M + 1)) plus bounded definition copy |
| `constraints() const` | Result of vector of SketchConstraint, ascending IDs, empty success. O(M) copy/result space, no solve |

Constraint mutations can update native aliases/fixed state, with geometry
projection or explicit solving enforcing equations. Mutation cost depends on
graph maintenance and translated resources. Unsupported preflight is unchanged
state; BackendFailure has no ordinary-edit rollback. There are no batch
constraint edits or filtered constraint queries in this revision.

### Coincidence placement

`addCoincident` is the explicit operation for choosing an initial configuration
before inserting Coincident. `addConstraint` retains its existing insertion
semantics. Both references must be distinct point-like elements, including curve
endpoints and centers. `CoincidentPlacement::Preserve` performs ordinary
insertion; `Midpoint` is the default and also requires point editing support.
Unknown policies, invalid/missing references, unsupported operations, nonfinite
selected coordinates, and public constraint-ID exhaustion are checked before
coordinate edits or ID allocation.

Midpoint builds temporary groups from public Coincident constraints, including
transitive links. For two free groups it proposes the selected points' midpoint
for every member of both groups. If one group contains Fix, it seeds only the
free group at the fixedPosition target. If several Fix constraints are present
in that group, its first target in ascending constraint-ID order supplies the
guess; conflicting anchors remain unchanged for explicit solve diagnostics.
When both groups contain Fix, or both selected references already belong to one
group, insertion proceeds without preparing coordinates. The midpoint computation
avoids overflow for finite inputs.

Preparation uses an internal point-guess batch, not dragPoints: it never
invokes numerical solving, changes metadata, rebuilds Impl, or creates
temporary public constraints. DCM writes through native updatePoints in GLOBAL
mode. Other equations are deferred until an explicit `solve()`.
Intermediate curves can therefore be noncircular or
degenerate, and final coordinates need not equal the initial midpoint. A returned
constraint ID confirms insertion, not convergence. Existing native hard projection
rules still apply during insertion, including when both groups are fixed.

This is an ordinary compound edit without rollback: BackendFailure after
preparation begins can leave proposed coordinates or a partially inserted
constraint. Preflight failures do not consume IDs; an ID allocated for failed
insertion remains consumed. Group preparation scans/copies the public constraints
and uses O(M) temporary graph space, with O(M log(M + 1)) map work plus updates for
the affected group members. No persistent group cache or coordinate mirror is
introduced.

CursorTool uses Midpoint for both the Num4 shortcut and Alt-click coincidence.
Clipboard insertion continues to use ordinary addConstraint so that copied
coordinates are not reseeded by this placement policy.

## Solve and drag

solve() returns Result of SolveDiagnostics. Unsupported capability fails before
mutation. DCM errors may produce BackendFailure. The current implementation calls
DCM's solve but unconditionally sets a successful result's status to
SolveStatus::Converged under an existing TODO; it does not verify convergence.
Solving retains public topology/IDs but can alter unconstrained coordinates.
Failed attempts can retain their iterate, including invalid/degenerate geometry;
queries expose it.

drag(const DragRequest&) performs one synchronous best-effort step on a
point-like reference toward finite target. Nonfinite target, nonpositive ID, or
incompatible sub-element is InvalidArgument; absent entity NotFound; disabled
drag Unsupported. Results/errors match solve. Coordinates change without public
ID/topology changes or persistent constraints, and failed iterates can remain.
Target is a preference, not a guaranteed coordinate. There is no gesture session,
cancellation, or history in Core. Do not call solve after drag: drag already
solves with the selected coordinates preferred, while an ordinary solve has no
drag preference. Check both the Result and diagnostics.status.

dragPoints(std::span<const DragRequest>) prevalidates all requests before mutation
and applies all point targets in one DCM batch. Identical GeometryRef targets
are rejected with InvalidArgument; distinct coincident references remain distinct
and can compete (DCM uses the last proposal to an unfixed coincidence group).
Empty input is a no-op with default diagnostics; it does not solve a dirty sketch.
Whole curves must be expanded to their point elements by the caller. This is
best-effort simultaneous point dragging, not a rigid group transformation.

Optional degreesOfFreedom, conflictingConstraints, and redundantConstraints
mean unavailable when absent, not zero DOF or an empty diagnosis. Diagnostics use
public constraint IDs. Failure to converge alone is not proof of conflicts.
Numerical convergence, graph preparation, and equation structure determine solve
and drag cost; there is no fixed linear-time bound.

DCM uses its stable-address manager incrementally. Bare DCM arcs require a
private equal-radius equation. Whole-sketch DOF/conflict/redundancy diagnosis is
not exposed because it omits unconstrained/eliminated variables. Native drag
discards convergence; solve can report success for inconsistent fixed systems.
Impl uses updatePoints in DRAG mode for the complete target batch, then restores
GLOBAL mode without another solve, including on exceptions. Like solve(),
dragPoints() currently reports Converged unconditionally after a successful
native call. The existing residual/geometry validation helper is not used by
either result path. Consumers still check Result and diagnostics.status, but the
current Converged value cannot establish that the constraints are satisfied.
DCM's native angle equations also use radians.

## Snapshots and state replacement

| Function | Behavior and failure contract |
| --- | --- |
| `snapshot() const` | Result of SketchSnapshot: detached current entities/constraints and both watermarks including deleted IDs. No solve; geometry query cost plus O(M) copy, O(N + M) result space |
| `replaceState(snapshot)` | Status; stages a fresh DCM Impl, validates/inserts state, and swaps only after full success. No implicit solve |
| `switchBackend(kind)` | Compatibility method; Dcm is no-op success, unknown enum InvalidArgument. Never edits the model or solves |

Watermarks must be nonnegative and cover every positive unique ID. Geometry must
be finite/nondegenerate; unequal arc radii are accepted during transfer. Constraint
definitions/references and DCM support are validated. InvalidArgument,
NotFound, Unsupported, or reported BackendFailure preserves the original model
and Impl; the candidate is discarded. Destination allocator watermarks become the
maximum of current/imported watermarks. Import can reinstall old IDs explicitly
supplied by the snapshot: intentional state restoration, not recycled allocation.
Owners must manage lineage when importing unrelated state.

Only replacement temporarily constructs another DCM Impl. It uses
O(N + M) staging space, geometry reads, and native insertion costs, not guaranteed
linear-time reconstruction. No numerical caches, document context, or persistence
schema transfer. Nonfinite/degenerate failed iterates can be rejected without
destroying the source.

## DCM capabilities

| Operation | DCM |
| --- | --- |
| Point/Line/Circle/Arc CRUD; metadata, counts, clear | Supported |
| Geometry batches; point queries/scopes | Supported |
| Coincident, Horizontal, Vertical, Parallel, Perpendicular | Supported |
| Distance, HorizontalDistance, VerticalDistance, Length, Angle, point-like Fix | Supported |
| Equal line lengths/circular radii; Radius/Diameter | Unsupported |
| Whole-curve Tangent | Unsupported |
| Constraint update/remove; solve; point-like drag | Supported; see current convergence-reporting limitation above |
| Snapshot/replacement | Supported for valid DCM definitions |
| Whole-sketch DOF, diagnosed conflicts/redundancy | Unavailable |
| Unsolved Fix/Coincident update projection | Native hard projection |

Sketch does not support arbitrary whole-curve Fix, rigid whole-entity/group drag,
point-on-curve coincidence, or edit transactions. DCM limits
constraints to translations with established semantics.

The ToolBar exposes Distance, H/V Distance, and Angle through App's DimensionTool.
H/V Distance selects two point-like references, picks X when `abs(dx) >= abs(dy)`
and Y otherwise, and orders the references along the positive chosen axis. The
stored constraint retains that axis across subsequent geometry edits. Angle
selects two whole directed lines; the UI accepts degrees and App converts to
radians before preparing the constraint.

## Architecture assessment and follow-up proposals

Computational interpretation and direct DCM ownership stay in Core.
Filtering/counts use metadata and avoid coordinate reads for nonmatches.
Shared reference rules serve constraint/drag validation and point queries;
Impl alone translates to DCM handles. DCM uses native mixed batches and direct
add/remove loops. No duplicate live geometry, persistent point mirror, type
index, UI policy, or dependency was added. Existing
App consumers remain source-compatible, including explicit bool metadata updates.

Deliberate limitations remain: query copies/scans, repeated removal constraint
scans, and no DCM-failure rollback. Public capacity is preflighted; native
capacity/errors can fail after editing begins. The current solve/drag TODO leaves
convergence reporting incomplete. The API promises neither exact edit targets
nor thread-safe shared-instance access.

Useful future APIs include constraint queries by type/reference and batch
constraint edits with explicit failure semantics. Measured App demand may justify
revisions or geometry deltas. DCM-failure injection would improve partial-edit
coverage; transactions need an explicit recovery design. Trustworthy DCM diagnosis
remains future work. Rigid group drag and cancellation rollback need
additional solver semantics and App session policy. Versioned persistence/document identity
remain outside the in-memory API.

SketchGTEST contains CRUD, diagnostics, drag, snapshot, and state-replacement
cases for DCM, plus compatibility selection checks. Behavioral tests, the
application, full builds, and CI were not run for this refactor; runtime behavior
is unverified.
