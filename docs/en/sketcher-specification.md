# OurPaint Sketcher Specification
## Revision 0.2

**Status:** proposed long-term capability and semantic specification.  
**Research date:** 2026-10-03.  
**Revision review:** 2026-10-04.  
**Implementation status:** a target map, not a claim of implemented or experimentally verified behavior.

## 1. Purpose, scope, and long-term philosophy

A professional Sketcher expresses geometric design intent, realizes i  t numerically, supports controlled direct editing, explains failures, and supplies dependable geometry and profiles to other features. This document maps the full design space of **2D parametric sketching**. It is intended to remain useful after OurPaint replaces either solver, changes its architecture, or grows into a larger research and development project.


The capability model is independent of team size, thesis scope, M0 scheduling, SolveSpace, OurPaintDCM, and the first Solid implementation. Cost affects staging, not mathematical taxonomy. An advanced capability must have a conceptual role and explicit semantics; its presence in one vendor's toolbar alone is insufficient justification.

Three kinds of material are deliberately separated:

- **Part A — Long-term capability model:** what geometry, intent, behavior, editing, interaction, diagnostics, and derived products exist.
- **Part B — Architecture implications:** alternative ownership models and recommended boundaries; these are ADR candidates, not universal truths about Sketchers.
- **Part C — Implementation staging and acceptance:** coherent delivery slices, benchmarks to establish, and research decisions. A late stage does not make a concept less fundamental.

The scope includes authored and referenced planar geometry, expressions, constraints, solving, freeform editing, planar arrangements, profiles, transactions, and downstream interfaces. File exchange formats, CAM, assemblies, rendering implementation, Class-A surfaces, and full B-Rep persistent naming are outside it. Their consumers/providers may impose interface obligations; they do not define Sketch semantics. A 3D Sketcher would share selected services but has different topology and placement semantics.

### One-page capability map

```text
2D parametric Sketcher
├── Persistent design definition
│   ├── Authored geometry: analytic, conic, polynomial/rational freeform
│   ├── Semantic references: endpoints, landmarks, handles, sites, intervals
│   ├── Relations: incidence, orientation, metric, symmetry, continuity
│   ├── Dimensions, named parameters, expressions, equations
│   ├── Recipes, rigid groups, blocks, patterns, suppression/configurations
│   └── Source bindings, roles, identity, history, versioned persistence
├── Evaluated realizations
│   ├── Parameter/reference evaluation and intrinsic geometry validation
│   ├── Constraint solving, DOF analysis, branch intent, interactive sessions
│   └── Accepted state, trial state, residuals, qualified diagnostics
├── Geometry and editing services
│   ├── Curve evaluation/jets, inversion, extrema, lengths, intersections
│   ├── Restriction, transformation, extension, offsets, approximation
│   └── Transactional edits + intent/identity/reference migration
├── Derived planar interpretation
│   ├── Intersection/overlap events → fragments → embedded arrangement
│   ├── Vertices, half-edges, wires, faces/cells, nesting and contact
│   └── Selectable regions/unions → validated profile snapshots
├── Interaction and explanation
│   ├── Acquisition/snapping → temporary inference → persistent intent
│   ├── Creation recipes, direct numeric entry, drag modes, repeat workflows
│   ├── Selection, navigator, glyphs, DOF/curvature/profile visualization
│   └── Diagnostics, proposed repairs, alternative solutions, cancellation
└── Document and downstream boundaries
    ├── Reference provider: sketches, datums, future model/B-Rep sources
    ├── Dependency scheduling, atomic commits, undo/redo, invalidation
    └── Consumers: points, references, open wires, closed profiles
```

Neither a solver nor a curve kernel encompasses this map. Computational geometry answers geometric queries; solving realizes relations; editing changes intent and identity; topology interprets the realized locus; interaction communicates and selects intent.

### Major changes from revision 0.1

- Expanded conics and freeform geometry; separated curve type, representation, trim, authoring, and role.
- Added creation workflows, a master capability inventory, advanced intent tools, and a substantial spline lifecycle model.
- Separated long-term contracts from architecture recommendations and delivery stages; compared backend-authoritative, neutral, and hybrid ownership.
- Corrected continuity, DOF/gauge, overlap, point-contact, query-quality, and reference-persistence semantics.
- Added evidence-qualified diagnostics, precise external-provider obligations, academic references, and technical research frontiers.
- Replaced arbitrary numerical acceptance constants with a benchmark-definition obligation. Existing IDs are retained; changed meanings/classifications are recorded in the change ledger.

### Reading and requirement conventions

**Principles** express product direction. **Capabilities** identify meaningful functionality, including advanced forms. **Contracts** state testable semantics whenever a form is advertised. **Numerical obligations** identify required completeness/error/termination behavior within a declared domain. **UX contracts** describe observable interaction. **ADR candidates** recommend architecture. **Research questions** remain unresolved. These categories are not interchangeable.

`SK-*` identifiers remain stable. A retained ID refers to its existing semantic topic; substantive changes and reclassifications are recorded in section 31. New topics receive new IDs. Tables with ID rows define contracts too. **Shall** is normative for an advertised capability or a selected delivery gate, not a demand to implement the entire target at once. **Should** is a recommendation. Descriptive mathematics and candidate algorithms are not requirements merely because they occur near a requirement.

Stage tags occur in the inventory and staging section, not in the geometry/constraint taxonomy. The feature completion rubric governs every delivered family. Unsupported forms must be explicit. Proposed acceptance scenarios are not execution results.

### Research method and limits

Evidence covers Siemens NX, SOLIDWORKS, CATIA, PTC Creo, Inventor, Fusion, Onshape, FreeCAD, and SolveSpace. Official help, vendor technical articles, source/API material, and scholarly references take priority. User documentation establishes observable capability, rarely proprietary solver/topology internals. No comparative product execution or performance benchmark was performed.

Sources span named releases and rolling help. CATIA includes a Dassault-hosted feature note and vendor-authored help on a third-party mirror with uncertain release/access; this evidence is weaker than current official documentation. Absence of evidence means **not verified**, not absent. Scholarly surveys identify concepts and research directions, not guarantees that one algorithm works for every sketch.

**Naming:** OurPaint's DCM adapter integrates **OurPaintDCM**. Siemens **D-Cubed 2D DCM** is a separate commercial component, cited as design-space evidence; its features must not be attributed to OurPaintDCM. Source summaries and original proposals are distinguished throughout.

## Part A — Long-term capability model

## 2. Master capability inventory

This is a coverage index, not an implementation-status table. **F** = Foundation (M0); **C** = Core mechanical (M1); **M** = Mature mechanical; **A** = Advanced freeform (M2 spans M/A); **R** = Industrial/research extensions. Stages are provisional navigation; they do not remove late capabilities from the target. “Typical support” summarizes reviewed product documentation, not exhaustive release certification.

| Area | Capability | Fundamental concept | Typical CAD support | OurPaint target stage           | Notes |
|---|---|---|---|---------------------------------|---|
| Definition | Identity, semantic sites, roles, revisions | Intent separate from coordinates | Universal concept; internal models vary | F                               | No solver/UI handles in domain semantics |
| Geometry | Points, finite lines, circles/arcs | Analytic loci and finite domains | Broad | F/C                             | Oriented arc/seam contract |
| Geometry | Infinite lines, rays | Unbounded supports and domains | Axes common; authored rays vary | F references; M authored        | No viewport clipping in model meaning |
| Geometry | Ellipse and trimmed ellipse | Centered conics and landmarks | Broad; editing depth varies | M/A                             | Semiaxis/focus semantics |
| Geometry | Parabola, hyperbola, general conic | Quadratic locus, branches, intrinsic classification | Selected conic tools | A                               | Rational storage does not erase conic intent |
| Geometry | Polynomial/rational Bézier | Finite polynomial/rational span | Many spline/style tools | A                               | Representation-specific control editing |
| Geometry | Polynomial B-spline/NURBS | Piecewise bases, knots, weights | Broad curve use; authoring varies | A                               | No single “supportsSpline” flag |
| Geometry | Closed/periodic and composite chains | Seam continuity and composition | Broad with different restrictions | C chains; A periodic            | Properties/composition, not roles |
| Creation | Primitive variants and chained creation | Placement recipes over entities | Broad | C                               | Center/three-point/tangent modes |
| Creation | Rectangles, slots, polygons | Generated relations and dimensions | Broad | C/M                             | Optional associative recipe; no new primitive class |
| Creation | Fit/control splines and conic recipes | Authored conditions → realization | Product-specific | A                               | Fitting policy is persistent intent |
| Relations | Incidence, alignment, orientation | Equality on sites/supports | Broad | C                               | Finite/support and orientation explicit |
| Relations | Metric, angular, equality, symmetry | Measurements and mappings | Broad | C/M                             | Named measures and site mappings |
| Relations | Tangency/contact, G0/G1/G2/G3 | Regular differential geometry | G1 broad; higher forms selective | C G1; A/R higher                | Endpoint versus arbitrary witnesses |
| Relations | C1/C2, derivative/weight/knot conditions | Parameterized representation relations | Advanced/selective | A/R                             | Parameter scaling and gauges matter |
| Relations | Fix, prescribed source, rigid groups | Absolute pose versus relative shape | Broad fix; groups/blocks selective | C/M                             | Different meanings, not synonyms |
| Intent | Associative recipes, patterns, block instances | Dependency and transform relationships | Many products | M                               | Independent copies remain useful |
| Dimensions | Driving/driven, signed/reflex, annotation | Equation versus observation | Broad | C                               | Invalid/stale measurement states |
| Parameters | Units, names, scopes, expressions | Typed dependency graph | Broad | C                               | Document scheduling boundary |
| Dimensions | Weak/generated completion | Replaceable scaffold intent | Creo distinctive | M                               | Neither driven nor solver-soft |
| Measures | Curved length, ratios, curvature, area | Integral/local/derived measures | Selected forms | M/A/R                           | Driving has extra differentiability obligations |
| Equations | Coupled equations, equation-driven curves | Simultaneous intent or generated locus | Selective | A/R                             | Different from acyclic expressions |
| Intent | Auto-dimension/full-constrain/recognition | Reviewable proposed design rules | Many industrial tools | M/R                             | Does not uniquely recover intended design |
| Intent | Priorities, inequalities, suppression/configurations | Activation and preference policies | Product/component-specific | M/R                             | Hard failures cannot become silent soft solves |
| Solver | Incremental nonlinear solving and branch choice | Valid realization near seed | Broad | F/C                             | No promise to enumerate all solutions |
| Solver | Physical DOF, redundancy/conflict analysis | Local rank, rigidity, inconsistency | Broad; certainty varies | C/R                             | Exclude representation gauges/helper freedom |
| Interaction | Drag endpoint/body/handle/rigid set | Intent-conditioned solve request | Broad | C/A                             | Branch/limit feedback and cancellation |
| Interaction | Temporary relaxation/alternative solutions | Explicit exploration of changed intent | Selective | M/R                             | Preview, audit, restore |
| Geometry queries | Jets, inversion, extrema, bounds | Type-specific mathematics via services | Kernels; UI exposure varies | C/A                             | Multiplicity, sidedness, error/completion |
| Geometry queries | Closest point, length, area, classification | Optimization/integration/domain query | Broad kernel capability | C/A                             | Global/local distinctions |
| Geometry queries | Intersections, self-intersections, overlaps | Events/intervals with parameter provenance | Essential; hard freeform cases vary | C analytic; A freeform          | Tangencies and unresolved intervals explicit |
| Geometry queries | Restriction/transform/projection/extension | Locus construction and mappings | Broad; forms vary | C/A                             | Exactness and source interval maps |
| Geometry queries | Offsets, approximation, bounded tessellation | Derived loci and error policy | Broad analytic; general forms difficult | C restricted; A/R general       | Topology cleanup is separate |
| Freeform | Knots, degree, weights, local/global edits | Representation lifecycle | Advanced tools vary substantially | A/R                             | Shape-preserving versus approximating operations |
| Freeform | Fairing/smoothing objectives | Shape optimization with protected intent | Advanced/selective | A/R                             | Separate from G/C continuity equations |
| Freeform UX | Handles, curvature combs, continuity analysis | Shape inspection and direct manipulation | Many advanced sketch tools | A                               | Display analysis does not impose constraints |
| Editing | Split/trim/power-trim/knife/extend | Interval edits plus semantic migration | Broad | C/M/A                           | Gesture mode and model edit differ |
| Editing | Join/merge/replace/delete | Many-to-one/substitution/dependency edits | Broad with varied preservation | C/M/A                           | No universal migration of all intent |
| Editing | Fillet/chamfer/offset | Construction with contact and cleanup | Broad analytic | C/A                             | Multiple solutions/failure domains |
| Editing | Move/rotate/scale/mirror/copy/pattern | Transform, solve, or recipe operation | Broad | C/M                             | Current shape versus future edit behavior |
| Topology | Arrangement fragments, half-edges, faces | Derived embedding of realized curves | Internal algorithms seldom published | C/A                             | Nondestructive to authored geometry |
| Topology | Duplicates/overlap/touch/sliver diagnostics | Coverage, contact and resolution policy | Mature repair workflows | C/R                             | Valid locus may contain authored duplicates |
| Profiles | Cells, holes, unions, open wires | Selectable areas and boundary snapshots | Broad modern workflows | C                               | Consumer-admissibility policy separate |
| Profiles | Selection persistence across split/merge | Provenance and correspondence | Internal strategies vary | C explicit ambiguity; M/R remap | Never substitute a face index blindly |
| References | Projection/inclusion/section; live/frozen/detached | Binding versus geometric operation | Broad; type-change restrictions vary | C/M/A                           | Provider supplies source correspondence |
| Inference | Snaps, guides, candidates, cycling, auto-relations | Acquisition → proposed intent → commit | Broad; distinctive product policies | C/M                             | Pixel thresholds are not model tolerances |
| UX | Numeric/HUD entry, repeat, navigator, filtering | Efficient and inspectable interaction | Broad mature systems | C/M                             | Non-color status and accessible controls |
| Diagnostics | Qualified facts, suspected causes, repair plans | Evidence-bearing explanation | Mature systems; certainty varies | C/R                             | Failure is not proof of infeasibility |
| Robustness | Validity, scale, tolerance/error budgets | Resolution and admissibility | Universal obligation, not uniform guarantees | F onward                        | No silent healing or false completeness |
| Persistence | Versioned intent, seeds, provenance, undo | Lifecycle across sessions/backends | Universal product need | F/C                             | Derived/native caches need not be serialized |
| Integration | Solver, reference, geometry, profile capability queries | End-to-end semantic support | Component APIs vary | F onward                        | Capability ownership spans more than solver |

## 3. Definition of a Sketcher

A sketch is a persistent design definition in a local Euclidean plane: entities, semantic references, constraints, parameter expressions, dimensions, and dependencies. A Sketcher is the system that creates, solves, edits, interprets, and presents that definition.

Three distinct products result from it:

1. **Design definition:** what the user meant, including unresolved or temporarily invalid edits.
2. **Evaluated geometry:** a validated realization of that definition at a particular revision.
3. **Derived topology and profiles:** how eligible curves partition the plane at that revision.

A fully constrained bow-tie can be unsuitable as one simple solid profile. An under-constrained rectangle can already define a perfectly usable region. Numerical convergence alone proves neither topology validity nor fulfillment of design intent.

**SK-SYS-001 — Independent states [Contract].** The system shall track definition validity, solve status, geometry validity, reference status, and topology/profile status separately, with revision identifiers. It shall never reduce these to one `valid` flag.

**SK-SYS-002 — Headless domain [Contract].** Geometry, constraints, expressions, solving, editing plans, and topology shall be usable without Qt, a renderer, a window, or an operating-system event loop. The application supplies transactions, document scheduling, and interaction context.

**SK-SYS-003 — A plane, not a camera [Contract].** All sketch equations use local 2D coordinates. A containing sketch feature owns support-plane placement and document identity. View rotation shall not redefine horizontal, vertical, or dimensional values.

**SK-SYS-004 — Multiple consumers [Contract].** Consumers shall be able to request points, construction references, open wires, or closed region profiles independently. Extrusion readiness is a property of a selected profile, not necessarily the whole sketch.

## 4. Product principles

**SK-PRN-001 — Intent over accidental appearance [Principle].** Coordinate coincidence, an inferred alignment, a persistent coincidence relation, and a derived topological vertex should remain distinguishable. None silently implies the others.

**SK-PRN-002 — Predictability over equation count [Principle].** A successful solve should preserve branch intent and accepted hard constraints. A geometrically distant solution is not acceptable merely because its residual is small.

**SK-PRN-003 — Explainable automation [Principle].** Automatic constraints, repair suggestions, and reference substitutions should identify what they will change. The user should be able to inspect and undo the resulting change.

**SK-PRN-004 — Progressive precision [Principle].** Rough, under-constrained sketches should remain useful. Fully constraining a sketch is encouraged for predictable reuse but is not required for editing or valid profile consumption.

**SK-PRN-005 — Honest support [Principle].** Unsupported operations should return a specific reason before persistent mutation. Approximation, snapshot copies, reduced diagnostics, and incomplete searches should be explicit.

**SK-PRN-006 — Orthogonal concepts [Principle].** Prefer orthogonal geometry families, semantic constraints, and reusable services. Representation diversity is justified by semantics, accuracy, and editing behavior, not by command count. Rectangles, slots, polygons, and most fillets are creation recipes over primitives, not reasons for new fundamental curve classes.

**SK-PRN-007 — Feature completeness [Principle].** A supported entity or operation needs creation, selection, modification, dimensions where meaningful, serialization, undo, diagnostics, and downstream-consumption behavior. A toolbar command alone does not constitute support.

Essential mathematics includes valid representations, differential evaluation, robust intersections, constraint semantics, and planar arrangements. UX conveniences include chaining tools, gestures, and alignment guides. General freeform offsets, G3 joins, and simultaneous profile-area constraints are advanced work. Photorealism, assembly mating, 3D sketching, CAM, and a Class-A surface-design suite are outside this specification.

## 5. Geometry model

### 5.1 Classification axes

Do not put all geometric concepts into one inheritance taxonomy. At least seven axes are independent:

| Axis | Meaning / examples |
|---|---|
| Mathematical locus | Point; line; circle; ellipse/parabola/hyperbola; polynomial or rational freeform |
| Representation | Analytic parameters; implicit quadratic; polynomial/rational Bézier; B-spline/NURBS; procedural evaluator; bounded approximation |
| Domain | Unbounded support, half-line, finite interval, full periodic traversal, several disconnected branches |
| Authoring definition | Coefficients/control points; interpolation/fit conditions; equation; associative construction recipe |
| Identity / origin | Authored entity; generated result; external source realization; derived topology restriction |
| Role | Profile-eligible boundary; construction; axis/centerline; datum point |
| State / presentation | Editable, prescribed/read-only, suppressed, unresolved, hidden, selected |

A **support curve** is the underlying locus/evaluator beyond a finite restriction where such extension is defined. A **trimmed curve** is a support plus domain and traversal. An **authored entity** carries persistent identity and intent; one entity can evaluate to several intervals/components. A **derived curve** has provenance and generation policy. Construction and reference are roles/origins, not curve families. A line used as a centerline remains linear geometry.

### 5.2 Complete geometry taxonomy

Generic DOFs below are physical placement/shape freedoms at a regular nondegenerate configuration, with representation structure fixed. Raw coefficient counts can include gauges. Trimming adds domain freedoms only if the endpoints are independently editable.

| Concept | Definition and semantic landmarks | Typical physical freedoms / cautions |
|---|---|---|
| Point | Position `(x,y)` | 2; may be authored datum or a semantic site on another entity |
| Finite line segment | Two distinct endpoints and oriented interval | 4; endpoint identities differ from support identity |
| Infinite line | Direction and normal displacement | 2; no finite length or endpoint; can partition the plane but alone bounds no area |
| Ray | Origin and oriented direction, half-infinite domain | 3; clipping for display is not finite restriction in the model |
| Circle | Center, positive radius, full locus | 3; arbitrary parameter seam is not an authored endpoint |
| Circular arc | Circular support plus start/sweep and traversal | 5; full traversal, opening, and zero-length restriction must be distinct |
| Ellipse | Center, axis frame, positive semiaxes; foci and quadrants | 5; circle limit makes major-axis orientation nonunique |
| Elliptical arc | Ellipse support plus finite interval/orientation | 7 in the generic free-end case |
| Parabola | Vertex, axis, focal parameter; focus/directrix | 4 for a nondegenerate Euclidean parabola; unbounded support, finite restrictions useful |
| Hyperbola | Center, axis frame, two semiaxes; foci/asymptotes | 5; two branches, no implicit branch hopping |
| Trimmed parabolic/hyperbolic/conic arc | Classified support plus branch, finite interval and traversal | Adds interval freedoms only when independently authored; not a separate locus family |
| General conic | `Ax²+Bxy+Cy²+Dx+Ey+F=0`, classification and selected real component/domain | 5 generic freedoms after homogeneous coefficient scale; parabolic subclass has 4; degeneracy can yield line pairs, point, or empty locus |
| Polynomial Bézier | Degree `p`, ordered control coefficients on a finite domain | `2(p+1)` coefficient freedoms before conditions; representation may describe a lower-degree locus |
| Rational Bézier | Homogeneous control data / control points and weights | Single rational span; common weight scaling is a gauge |
| Polynomial B-spline | Degree, nonuniform knots/multiplicities, control coefficients | Piecewise polynomial with local basis support; knots may be fixed authoring data or explicitly editable parameters |
| Rational B-spline / NURBS | B-spline basis and homogeneous weighted coefficients | Exact conic representations possible; denominator and weight policy explicit |
| Periodic / closed curve | Domain and seam properties of any applicable representation | Closed means endpoint positions agree; periodic representation supplies periodic continuation with declared continuity |
| Composite curve / chain | Ordered oriented source intervals, join conditions, optional shared parameterization | Not automatically one smooth primitive; mixed types, corners, gaps and branches must be explicit |
| Equation-defined / generated curve | `C(t)` or explicit graph over a declared domain; persistent expression/recipe | May evaluate analytically, procedurally or approximately; dependencies and singularities are part of its definition |

Parabola/hyperbola and general conics belong to the target even if staged late. A degenerate conic need not have an ordinary creation tool; its classification is necessary for diagnosis, type transitions, imported/reference records, and algorithm safety.

### 5.3 Conic semantics and representation choice

Rational quadratic spans can represent conic arcs exactly; multiple spans can represent a circle or ellipse. “Exact representation” means algebraically locus-preserving before floating-point rounding, not that all computation uses exact arithmetic. [NURBS technical reference][A2]

Two defensible designs are analytic conic records with rational evaluation/conversion, or conic-semantic definitions backed by rational spans. Generic NURBS alone loses authored center/axis/focus/directrix/asymptote, eccentricity/rho, and branch meanings unless these are separately retained. Recognizing a generic spline as a conic is an optional qualified operation; it must not silently change authored intent. The storage choice remains an ADR candidate.

**SK-GEO-012 — Conic identity [Contract].** An advertised conic authoring form shall preserve its named landmarks, shape-parameter convention, branch, and finite/support domain through ordinary solving and exact representation conversion. A degeneration or conic-type change shall report which references cease to be meaningful.

**SK-GEO-013 — Domain and component identity [Contract].** Unbounded and multibranch geometry shall expose its actual domain/components. View clipping, picking windows, and numerical search bounds shall not silently redefine the authored entity. Restricted searches shall report their bounds.

**SK-GEO-014 — Composite semantics [Contract].** A chain shall identify ordered member intervals and traversal, continuity/connection conditions, and whether the chain is an independent grouping or an associative definition. Whole-chain selection shall not imply that all members share one support or one solver entity.

**SK-GEO-015 — Rational admissibility [Contract].** Rational forms shall declare weight/denominator admissibility and regular intervals. Positive weights are a useful supported policy, not the definition of all rational curves. General signed/zero homogeneous data and denominator zeros require explicit advanced support and diagnostics; unsupported projective behavior shall not enter ordinary finite profiles.


**SK-GEO-001 — Neutral entities [ADR candidate].** OurPaint should persist neutral domain values and stable IDs, excluding solver handles, native parameter addresses, Qt points, and renderer objects from public geometry records. This is the recommended ownership design evaluated in Part B; neutral public semantics are required regardless of private storage choice.

**SK-GEO-002 — Intrinsic validity [Contract].** Accepted evaluated geometry shall have finite values and satisfy its declared representation invariants. Invalid authored or imported definitions may be preserved for diagnosis/repair, but shall not masquerade as valid evaluated geometry. A trial with unequal circular-arc endpoint radii shall not be admitted as an accepted circular arc or profile boundary.

**SK-GEO-003 — Arc semantics [Contract].** Arcs shall distinguish full traversal from nonzero partial and empty restrictions, transform traversal/sweep orientation consistently through reversal/reflection, and specify seam behavior. A zero sweep shall not ambiguously mean a full circle. Public meaning shall not depend on backend endpoint-order conventions; opening a full traversal shall preserve locus and endpoint identity policy explicitly.

**SK-GEO-004 — Orthogonal roles [Contract].** Profile participation shall be explicit. Construction curves and axes shall not partition profiles by default. Referenced curves may participate only according to a saved inclusion policy. Visibility shall not change topology.

**SK-GEO-005 — Semantic sub-elements [Contract].** References shall distinguish endpoint, center, ellipse axis/focus, spline control point, fit point, knot location, tangent handle, and point-at-parameter. Only applicable forms shall be offered. A control point is not presumed to lie on its curve.

**SK-GEO-006 — Stable identity [Contract].** Entity and constraint IDs shall remain stable under ordinary numerical movement. Coincident points shall retain separate identities unless an explicit model edit merges them. Deleted IDs shall not be reused within a document lineage.

**SK-GEO-007 — Spline definition [Contract].** Degree, knots, multiplicities, periodicity, weights and their authoring/variable policies shall be explicit. Ordinary coefficient solves preserve representation structure unless a declared advanced formulation includes admissible knot/weight variables. Degree changes and discrete knot-structure changes are representation edits or explicit outer authoring procedures with reference-remapping results; they shall not occur as hidden solver side effects.

**SK-GEO-008 — Fit versus control representation [Contract].** Interpolation/fit authoring shall store its conditions, parameterization/fitting policy, and association separately from evaluated coefficients. Refitting may be the declared realization policy, including after constraint edits. Conversion or policy change shall disclose shape, handle, conditioning and reference consequences; unannounced refitting that changes intent is prohibited.

**SK-GEO-009 — Closed versus periodic [Contract].** Endpoint coincidence shall not imply derivative continuity at a seam. Closed and periodic curves shall be distinguished; the guaranteed continuity shall be queryable.

**SK-GEO-010 — Exact and approximate provenance [Contract].** Geometry shall record representation family independently from provenance and accuracy: authored analytic/freeform data, exact locus-preserving conversion, generated evaluation or bounded approximation. A spline is not inherently approximate; rational splines can represent conics exactly. Exact conversion does not imply exact arithmetic or exact topological closure after rounding.

**SK-GEO-011 — Unsupported but preserved [Contract].** Loading or switching to a backend that cannot solve an entity shall not delete or tessellate that entity. A recognized neutral entity can remain viewable and preserved with an explicit unsolved/read-only status. Unknown future records require preservation or an explicit unsupported-version refusal, never silent omission.

## 6. Creation tools and authoring workflows

Creation is an interaction family distinct from the mathematical taxonomy. A three-point circle creates a circle; a center rectangle creates a segment recipe; a linked pattern creates an associative dependency. A one-time construction may record provenance without remaining a live recipe. The UI command count must not dictate entity class count.

| Workflow | Important variants | Fundamental result / intent | Classification |
|---|---|---|---|
| Point / linear geometry | Datum point, segment, infinite line, ray, centerline | Point or linear support/domain, chosen role | Fundamental placement; centerline is a role |
| Chained drawing | Polyline, mixed line/arc chain, tangent continuation, close-to-start | Primitive members plus chosen incidence/orientation | Creation recipe + continue UX |
| Rectangle | Corner, center, oriented corner/center, square | Four segments, incidence/parallel/perpendicular; center/size intent | Recipe; optional persistent associative form |
| Parallelogram | Corner/side/direction, centered form | Segment relations without automatic right angles | Recipe |
| Polygon | Inscribed/circumscribed regular, edge-defined regular, arbitrary | Segments, count, center/radius or side intent | Recipe; regularity may remain associative |
| Circle | Center-radius, diameter endpoints, three points, tangent constructions | Circular support and selected placement conditions | Primitive variants; construction witnesses optional |
| Circular arc | Center-start-sweep, center endpoints, three points, tangent arc | Circular support, interval, orientation/branch | Primitive variants |
| Slot | Center-to-center, overall-length, centered, three-point arc, center-arc | Lines/arcs, tangencies, equal radii, construction centers and width | Recipe; no separate slot curve type |
| Ellipse / elliptical arc | Center/axes, foci/point where supported, interval | Elliptic conic with named landmarks | Primitive variants |
| Conic | Endpoints/tangents/rho, vertex/focus/directrix, hyperbola branch/asymptotes | Conic locus plus semantically meaningful construction data | Primitive or associative construction |
| Bézier / control spline | Degree and poles, rational weight controls, periodic option | Representation coefficients and structure | Fundamental authoring policy |
| Fit/interpolation spline | Fit sites, interpolation/approximation, parameterization, end conditions | Authored conditions and realized spline | Authoring definition; not poles disguised as fit points |
| Reference creation | Project, include/copy in plane, plane section, generated intersection | Source binding and evaluated planar curves | Associative feature or explicit frozen/detached copy |
| Equation curve | Explicit graph or parametric expression, finite domain | Procedural/generated definition, approximation if necessary | Associative authoring form |
| Reusable / derived creation | Block insertion, mirror/pattern, offset/fillet recipe | Source-member maps, transforms, generated geometry | Associative feature or independent recipe |

SOLIDWORKS documents rectangle/parallelogram variants; Fusion documents distinct slot and conic workflows. Their variety is evidence for creation policies over reusable geometry, rather than separate mathematical primitives. [Rectangle workflows][SW6], [slot workflows][FU4], [conic workflow][FU5]

**SK-CRT-001 — Recipe result [Contract].** Creation shall expose generated entities, constraints, dimensions, construction aids and provenance, and whether source association persists. Editing a generated condition shall have an explicit effect on the recipe: preserved, modified, detached or invalid.

**SK-CRT-002 — Placement modes [UX contract].** Multi-step creation shall preview the next geometric result and selected branch, support direct numeric entry and inference controls, and distinguish cancel-current-element from finish/cancel-tool. Committed chain members and provisional members shall have defined undo behavior.

**SK-CRT-003 — Creation versus solving [Contract].** Choosing a placement recipe shall not imply that every input becomes a permanent relation. The user/tool policy shall identify which values or witnesses are retained as driving intent, which are inferred, and which only seed the initial shape.

**SK-CRT-004 — Equation-defined geometry [Contract].** Equation curves shall retain expressions, units, declared domain and dependencies; distinguish exact/procedural evaluation from approximation; and diagnose nonfinite values, discontinuities, singularities and invalid domains. Solving arbitrary expression coefficients is a separate capability from evaluating a generated curve. [Industrial precedent][SW7]

## 7. Spline and freeform capability family

“Spline support” covers representation, authoring, solving, editing, queries, analysis, and profile participation. A backend able to position cubic poles is not thereby able to fit points, edit weights, preserve G2, find all intersections, or offset NURBS.

### 7.1 Representation and authoring

| Property | Required conceptual distinction | Editing / solver consequences |
|---|---|---|
| Degree / order | Polynomial degree `p`; order `p+1`; actual locus may have lower degree | Degree elevation can preserve shape; reduction usually approximates |
| Controls / weights | Euclidean poles with weights or homogeneous coefficients | Poles need not lie on curve; common weight scale is a gauge |
| Knots / multiplicity | Nonuniform knot values, repeated knots, continuity spans | Discrete structure and continuous knot positions are distinct edits |
| Clamping / end conditions | Knot policy and endpoint evaluation behavior | Endpoint is not always first/last pole; handles depend on representation |
| Fit versus interpolation | Exact passage through sites versus error/objective fit | Fit parameters/parameterization may be fixed, solved or regenerated under declared policy |
| Local versus global | Basis support versus a solve/fitting objective | Moving one pole can be local in evaluation yet trigger global constrained motion |
| Closed versus periodic | Positional closure versus periodic continuation/seam continuity | Opening, seam relocation and endpoint identity need separate policies |
| Rational conics | Exact rational representation plus optional conic semantics | Conic shape/landmark intent must survive conversions where promised |
| Handle semantics | Direction, derivative magnitude, curvature, curvature rate, pole distance | A visible handle length is not universally a physical derivative magnitude |

At an interior knot with degree `p` and multiplicity `m`, the usual basis guarantee is `C^(p-m)` where applicable; special coefficients may make the actual curve smoother. Shape-preserving knot insertion does not reduce actual smoothness of the unchanged locus; it reduces what is guaranteed after unconstrained coefficient edits. Local basis support does not guarantee locality of the constrained solve. [B-spline algorithms][A1]

### 7.2 Structural and shape editing

| Capability | Exact / shape-preserving contract | Change-of-intent or approximation contract |
|---|---|---|
| Subdivision / trim / restriction | Preserve locus and source interval map | Independent children versus retained parent association explicit |
| Knot insertion / Bézier extraction | Preserve curve to numerical representation error | New controls/knots change available authoring freedom and references |
| Knot removal | Exact only when coefficients admit it | Otherwise error-bounded simplification with preview and removed-reference disposition |
| Degree elevation | Preserve polynomial/rational curve with equivalent data | Additional coefficients do not automatically represent independent physical freedoms |
| Degree reduction | Exact if the curve is representable at lower degree | Otherwise approximation with global/local error criterion |
| Knot relocation / reparameterization | Exact transformations when supported | Arbitrary knot changes generally alter shape or fitting policy |
| Weight editing | Preserve declared denominator/regularity policy | May change shape, parameterization, conic status and conditioning |
| Extension | Declared continuation: terminal span, tangent line/arc, refit, extrapolated fit | No universal spline support extension; strategy and interval bounds visible |
| Join | Composite chain, exact concatenation with compatible structure, or a shared-support restriction | Refit into one curve is distinct and error-bearing |
| Continuity enforcement | Satisfy selected G/C conditions at regular sites | Consumes freedom; may change shape/handles; failure and alternatives explicit |
| Fair / smooth | Exact representation simplification only when the locus is unchanged | Otherwise optimize a declared fairness objective with protected sites/conditions and error/displacement preview |
| Close / periodicize / open | Locus-preserving seam changes where possible | Enforcing seam continuity may alter locus; geometric closure and endpoint identity independent |

### 7.3 Freeform semantic contracts

**SK-SPL-001 — Authoring-policy completeness [Contract].** An editable freeform form shall specify degree/knots/weights, fit or control conditions, endpoint/seam policy, editable variables, admissible structure, and the meaning of handles. Structure changes shall remap semantic references rather than relying on nearest control-point indices.

**SK-SPL-002 — Shape-preserving operations [Contract].** Advertised exact subdivision, insertion, extraction, elevation and conversion shall reproduce source geometry within documented numerical representation error and return parameter/reference correspondence. Their exactness shall not imply bitwise identical evaluation or unchanged control identities.

**SK-SPL-003 — Approximation operations [Numerical obligation].** Reduction, removal, fitting, replacement and conversion that alter the locus shall state the metric/domain of the error claim, achieved bound or estimate, and effects on constraints and semantic sites. A sampled maximum shall be labeled an estimate unless a bound has been established.

**SK-SPL-004 — Local editing [UX contract].** Whole-curve, pole, fit-site, endpoint, derivative-handle and local-span editing shall be distinguishable. A locality/end-protection mode shall identify the protected conditions and affected neighborhood; if relations couple beyond it, preview the effect or report the limit. Do not promise locality from basis support alone.

**SK-SPL-005 — Analysis overlays [UX contract].** Control polygons, knot/site markers, tangent/curvature handles, curvature combs, inflections and seam-continuity inspection shall report the evaluated revision and available derivative regularity. Comb density/scale are display controls; a comb is not a constraint or proof that no curvature extremum was missed.

**SK-SPL-006 — Advanced derivative controls [Contract].** Tangent magnitude, second/higher derivatives, curvature and curvature-rate conditions shall identify parameter scaling or arc-length meaning and regularity prerequisites. Weight/knot conditions shall distinguish geometric intent from representation constraints and eliminate or diagnose gauges.

Reviewed products expose different portions: SOLIDWORKS Style Spline includes rational weight controls; Inventor offers fit/control workflows and combs; Fusion exposes degree and curvature handles; Onshape allows handle constraints/dimensions. Their tools do not imply universal rational, degree, or contact support. [Style spline][SW8], [Inventor spline workflows][IN5], [Fusion spline workflows][FU6], [Onshape spline handles][ON5]

**SK-SPL-007 — Fairing goals [Contract].** Fairing/smoothing shall distinguish an objective such as curvature variation or bending energy from hard incidence/continuity conditions. The affected interval, protected intent, achieved objective and shape displacement/error shall be explicit. An attractive comb or lower objective shall not imply unique optimal shape or preservation of all design intent.

Degree/knots/weights, continuity and shape modification have a large established mathematical literature; *The NURBS Book* is a reference text, while the accessible MIT hyperbook supplies specific algorithmic/differential context. No single fitting or intersection algorithm is selected here. [Reference bibliography][A3], [Shape Interrogation][A4]

## 8. Generic curve capabilities

A generic curve abstraction is useful, but one virtual interface containing every geometric operation is not. Evaluation belongs to the representation; pairwise intersection belongs to a dispatcher/service; editing belongs to a transactional operation; projection needs source and plane context. Offset is generally a one-to-many construction, not `Curve::offset() -> Curve`.

**SK-CUR-001 — Curve view [Contract].** A read-only curve view shall expose kind, domain, orientation, closed/periodic flags, period where applicable, continuity intervals, and revision. Finite trimmed domains shall be distinguishable from their underlying support curves.

| Capability | Recommended contract | Ownership / implementation |
|---|---|---|
| Evaluate | `value(u)` with checked domain and sided evaluation at discontinuities | Type-specific mathematics behind a common query |
| Derivatives | `jet(u, order, side)`; return available order, undefined/degenerate status | Analytic by type; do not require finite differences as the contract |
| Tangent/normal/curvature | Derived from a regular jet, with orientation and units | Generic differential service; undefined when speed is zero |
| Domain | Finite interval, periodic interval, or unbounded support | Representation; never force all native parameters to `[0,1]` |
| Bounding box | Conservative bounds over an interval, plus error policy | Type-specific analytic extrema or conservative subdivision |
| Closest point | All relevant minima or a documented global/local result, parameter, distance, ambiguity and quality | Geometry query service; endpoints included |
| Point inversion | All parameters within tolerance, or nearest parameter under an explicit policy | Geometry service; self-intersections can yield multiple answers |
| Curve intersections | Points and overlap intervals, parameter pairs, classifications, quality | Pairwise geometry service |
| Length / parameter by length | Numerical error bound, monotonic interval and convergence status | Exact analytic cases; quadrature/root isolation for freeform |
| Restrict / split | Exact subcurve(s) and parameter maps when representable | Geometry construction service; persistence handled by editing |
| Reverse | Oriented curve/view plus parameter mapping | Geometry service; semantic references handled by editing |
| Affine transform | Type-preserving where valid; otherwise explicit type change | Geometry service; nonuniformly scaled circles become ellipses |
| Projection onto sketch plane | Result set, degeneracies, source map, exactness | Reference/projection service using geometry algorithms |
| Offset | Signed side, distance, join/cap policy, regularity domain; zero or more components | Offset construction + topology cleanup |
| Approximation / tessellation | Error policy and achieved error; preserve source parameter map | Shared geometry service; display tessellation is not model geometry |

**SK-CUR-002 — Parameter meaning [Contract].** Curve parameters shall not be treated as distances. Reparameterization shall provide an explicit source-to-result map. The map may be piecewise, reverse orientation, or be unavailable after approximation; clients shall handle these outcomes.

**SK-CUR-003 — Differential regularity [Contract].** Curvature shall use the oriented planar definition `cross(C', C'') / |C'|^3` for regular curves. Derivative order, knot continuity, and degenerate tangents shall be checked before evaluation. A zero-speed point shall produce a diagnosis rather than NaN propagation.

**SK-CUR-004 — Quality-bearing results [Contract].** Queries shall independently report execution status, completed/unresolved domain coverage, accuracy/bounds or estimates, isolated multiplicity/continuous solution intervals, and ambiguity. A completed approximate result can be valid; an accurate isolated root does not prove a complete search. Local closest-point results shall not be labeled global minima, and exhausted searches shall not return authoritative empty sets.

**SK-CUR-005 — Splitting [Contract].** Every supported finite curve shall support splitting at a valid interior parameter or explicitly report that form unsupported. The ordered children shall reproduce the original curve within the operation's declared error, with no gap and a source-parameter map. Exact restriction is required for native Bézier/B-spline/NURBS curves; tolerance refers to floating-point evaluation, not permission to refit.

**SK-CUR-006 — No false universal operations [Contract].** The API shall not assume every curve has a center, radius, finite bounding box, unique closest point, natural extension, or same-type offset. Point queries and curve queries shall remain distinct.

Polynomial Bézier subdivision uses de Casteljau; rational subdivision operates in homogeneous coordinates. B-spline restriction uses knot insertion/extraction or an equivalent exact trimmed representation. Analytic circle/ellipse arcs can share a support curve with an interval. Implementation choices must preserve endpoint and parameter semantics. OCCT exposes knot insertion, segmentation, derivative continuity, and rational/periodic spline distinctions, illustrating the depth hidden behind a generic curve handle. [OCCT spline reference][G2]

### Small evaluators and query services

The common abstraction should describe a curve and allow checked evaluation/jets over declared spans. Algorithms consuming it must query prerequisites and choose appropriate analytic, polynomial/rational or bounded procedural methods. Type-specific representations can support exact conversion/construction without requiring every operation as a virtual member. Open multimethod/variant dispatch, traits, function tables or service interfaces are implementation choices.

Additional generic query concepts are **regularity/continuity spans**, critical points/extrema, inflections and stationary curvature, arc-length inversion, oriented area contribution, point/region classification, support/interval equivalence and distance candidates. The next section on computational geometry specifies their result semantics. Generic mathematical meaning does not imply one generic implementation or support for arbitrary procedural functions.

## 9. Constraint taxonomy

### 9.1 Semantic rules

**SK-CON-001 — Typed meanings [Contract].** Constraint definitions shall identify semantic reference roles, values, branch options, activation, origin (manual/inferred/generated), and parent operation where applicable. Invalid combinations shall be rejected before solver translation.

**SK-CON-002 — Support versus trimmed domain [Contract].** Point-on-curve and tangency shall state whether they apply to the finite trimmed entity or its unbounded/full support. The UI shall expose this distinction when it changes the result. There shall be no implicit conversion of endpoint contact into remote support tangency.

This matters in practice: SOLIDWORKS documents that relations to lines and arc segments can act on their infinite/full supports. That is useful but can surprise a user expecting physical contact. OurPaint should preserve both meanings explicitly. [SOLIDWORKS relation semantics][SW1]

**SK-CON-003 — Composite constraints [Contract].** A semantic relation may translate to several equations or helper entities. The public constraint remains one identifiable object, with diagnostics mapped back from all generated equations. Public counts shall exclude intrinsic validity equations.

### 9.2 Geometric relations

| ID | Relation | Semantic contract | Implementation qualification |
|---|---|---|---|
| SK-CON-010 | Coincident | Two point references have equal positions; identities remain separate | Independent of owning curve types |
| SK-CON-011 | Incidence / point-on-curve | Point at a named fixed site or with a free witness on a support/domain | Witness bounds and multiplicity explicit |
| SK-CON-012 | Horizontal / vertical | Point-pair alignment or linear direction relative to sketch axes | Axis/tangent variants explicitly named |
| SK-CON-013 | Parallel / perpendicular | Relation between valid direction references: supports, axes, tangents or handles | Tangent sites can differ; contact is not implied |
| SK-CON-014 | Collinear | Linear supports coincide, without requiring finite overlap | Incidence plus orientation |
| SK-CON-015 | Tangency / contact | Regular curves contact at selected endpoint or arbitrary witnesses with collinear tangents | Finite/support, internal/external and orientation branch explicit |
| SK-CON-016 | Endpoint G1 join | G0 positional join plus matching oriented unit tangent through the join | Does not accept a reversed-tangent cusp as smooth continuation |
| SK-CON-017 | Concentric / coradial | Named centers coincide; coradial additionally equates circular radii | Central conics have centers; parabolas do not |
| SK-CON-018 | Midpoint / bisector site | Segment affine midpoint or a named half-parameter, half-sweep or half-length site | These measures differ on general curves |
| SK-CON-019 | Equality / shape correspondence | Named lengths/radii/semiaxes/measures equal; congruence or similarity uses an explicit transform/map | No unspecified whole-curve equality |
| SK-CON-020 | Symmetry | Point, curve or matched set reflected about an axis or central point | Site/control/interval correspondence explicit |
| SK-CON-021 | Fix / prescription | Selected coefficients, sites, geometric locus or complete pose prescribed | Fix locus and fix representation need not coincide |
| SK-CON-022 | Rigid group / block | Preserve internal shape while allowing rigid placement; block adds reusable definition/instance identity | Generic planar placement has up to three physical freedoms; shape stabilizers excluded |
| SK-CON-023 | Equal curvature | Equal signed curvature at two oriented regular sites without automatic contact | Zero curvature allowed; singular sites excluded |
| SK-CON-024 | G2 join | Oriented G1 join plus equal signed curvature | Independent of parameter speeds |
| SK-CON-025 | Spline/site conditions | Fit-site incidence, tangent direction/magnitude, derivative or curvature conditions at named sites | Site and representation policies explicit |
| SK-CON-026 | Conic landmarks | Center/vertex, axes, foci, directrix, asymptotes and selected shape parameters where meaningful | Undefined/degenerate landmarks diagnosed |
| SK-CON-027 | Pattern / mirror relationship | Instances follow a saved transform and source-member mapping | Associative recipe differs from independent copies |
| SK-CON-028 | Higher continuity | Planar G3 matches oriented dκ/ds after G2; C1/C2/C3 match parameter jets under a declared map | Requires sufficient regularity; parameter equality is not geometric equality |

Parallel tangents alone do not establish a contact point. Equal curvature alone does not establish G2 continuity. G1 permits different parameter speeds; C1 equates derivatives under the chosen parameterization. A line can meet a spline with G2 if the spline endpoint curvature is zero. A nonzero-radius circle cannot meet a line with G2. Two circular arcs with the same oriented tangent and curvature at a shared point lie on the same circle locally.

For general spline tangency, witness parameters may be solver unknowns. Bounds, selection proximity, regularity, and branch intent are essential; merely imposing a vanishing tangent cross product admits remote and unwanted contacts. High-order continuity consumes shape freedom and becomes poorly conditioned near zero derivatives. It belongs to the long-term model; staging is a separate decision.

Fusion documents a spline curvature relation; Inventor distinguishes endpoint Smooth (G2); Creo exposes equal curvature; Siemens D-Cubed documents higher-order spline controls. These demonstrate product demand, not equivalent APIs or universal support for every pairing. [Fusion constraints][FU1], [Inventor constraints][IN1], [Creo constraints][CR1], [D-Cubed spline controls][DC1]

### 9.3 Dimensional and generated relations

**SK-CON-030 — Dimensional families [Contract].** The basic solver contract shall support point distance, signed horizontal/vertical separation, segment length, oriented or explicitly unsigned line angle, radius, and diameter. Point-to-line normal distance and parallel-line separation shall specify support and sign semantics.

**SK-CON-031 — Advanced measures [Contract].** Arc sweep/length, ellipse/conic shape parameters, curve length, tangent angle, curvature/radius of curvature, distance/length ratios and differences, minimum separation, and region area/perimeter shall have named meanings and independently discoverable support. Minimum witnesses and region boundaries can change branches/topology, creating nonsmooth driving equations; measured availability does not establish driving support.

**SK-CON-032 — Roles are independent [Contract].** A dimension can be driving, measured/driven, or temporarily inactive. A construction entity can have driving constraints. An external reference can be prescribed without a user-visible Fix constraint. These roles shall not be collapsed into a single “reference constraint” enum.

**SK-CON-033 — Generated provenance [Contract].** Constraints created by rectangle, fillet, trim, mirror, or pattern commands shall identify their generating operation and remain inspectable. Removing a generated relation shall either make the operation independent or visibly mark it modified; it shall not reappear mysteriously.

### 9.4 Neutral residuals and independent validation

**SK-CON-040 — Semantic residual oracle [Contract].** OurPaint shall be able to evaluate whether a candidate satisfies its public constraint meanings without accepting the backend's private equation conventions. This need not duplicate the solver: evaluate geometric residuals and branch/domain conditions, not a second nonlinear optimization. Unsupported validation of a newly added constraint family prevents certifying that family.

The following regular-case formulations illustrate the required semantics, not mandatory solver equations. Let `p,q` be points, `d` a nonzero line direction, `n` its chosen unit normal, `c,r` a circle center/radius, and `R_axis` reflection across a line.

| Constraint | Validation invariant | Branch / degeneracy condition |
|---|---|---|
| Point coincidence | `p-q = 0` | Separate identities remain; two positional components |
| Point on line support | `dot(p-a,n) = 0` | Segment membership additionally checks a bounded witness parameter |
| Point on circle | `length(p-c)-r = 0` | Arc membership additionally checks oriented sweep |
| Parallel / perpendicular | Normalized direction cross product / dot product is zero | Nonzero directions; parallel direction sign is independent intent when needed |
| Collinear | Parallel supports plus zero normal separation | Do not add fictitious finite overlap requirements |
| H/V distance | `q.x-p.x = dx` / `q.y-p.y = dy` | Signed value convention saved |
| Point distance | `length(q-p) = D` | `D >= 0`; zero distance is singular for a norm derivative and should use coincidence semantics where appropriate |
| Oriented angle | Wrapped `atan2(cross(d1,d2), dot(d1,d2))` equals target | Saved winding/quadrant; avoid relying only on cosine, which admits reflected angles |
| Circle–line tangency | Chosen signed center-to-line distance equals `+r` or `-r` | Validate contact location on finite domains when required |
| Circle–circle tangency | Center distance equals `r1+r2` or `abs(r1-r2)` | Internal/external and containment branch saved; concentric equal circles are overlap, not isolated tangency |
| Segment midpoint | `p = (a+b)/2` | Does not define a general curve's arclength midpoint |
| Point symmetry | `q = R_axis(p)` | Axis must be regular; two reflected points do not imply complete-curve symmetry |
| General contact | `C1(u)=C2(v)` and collinear regular tangents | Finite witness bounds and oriented contact branch; do not accept zero tangents as tangent |

Numerically convenient squared distances or cross-product equations may be used inside a backend, but residual reporting shall return meaningful distance/angular errors. Validation shall test the intended branch and domain in addition to the equality residual. A composite relation's scalar equation count is not its guaranteed rank reduction.

### Mathematical relation families beyond toolbar names

| Family | Capability space | Semantic qualification |
|---|---|---|
| Incidence | Point-point, point-site, point-on-support/domain, site-on-site, selected contact | Fixed versus free witnesses; finite bounds; branch/site identity |
| Alignment / orientation | Collinear, parallel, perpendicular, horizontal/vertical, axis/normal/tangent alignment | Direction references may be at distinct locations; contact is separate |
| Metric / angular | Distances, signed components, angles, lengths, curvature, integral measures | Named measure and units; oriented/reflex/winding convention |
| Equality | Equal measures, ratios/differences; congruent/similar shapes | Scalar equality differs from full shape equivalence |
| Symmetry | Point pair, axis/central symmetry, symmetric support/set, recognition | Matching map and orientation; recognition is a proposal |
| Contact / support equivalence | Endpoint/arbitrary tangency; shared support; curve-on-curve interval correspondence | A few sampled incidences cannot prove whole support equality |
| Continuity | G0/G1/G2/G3 and C0/C1/C2/C3 under a parameter map | Regularity and traversal semantics required |
| Prescription | Fix site, pose, coefficients, locus; external prescribed realization | Internal shape versus absolute placement; gauge elimination |
| Group / recipe | Rigid group, block instances, mirror, arrays, generated construction | Feature dependencies may realize intent outside numerical solver |
| Conic-specific | Axis/focal/directrix/asymptote relations, eccentricity/rho/semiaxes | Parameter conventions and classification explicit |
| Representation-specific | Pole alignment/equality, handle length, fit/knot conditions, weight ratios | May constrain representation rather than unique geometric shape |
| Admissibility / preference | Domain bounds, positive size, inequality, priorities, weak completion | Equality, inequality and soft objectives are distinct |

For regularly oriented planar joins: **G0** is positional contact; **G1** matches unit tangent; **G2** also matches signed curvature; **G3** also matches `dκ/ds`. **C0/C1/C2/C3** compare position and successive parameter derivatives after a specified correspondence. G-continuity permits reparameterization; C-continuity depends on it. A planar torsion is zero where defined, so the vendor command name “torsion continuity” must not become OurPaint's neutral terminology. [Geometric continuity reference][A5], [documented industrial continuity command][SW9]

**SK-CON-041 — Support equivalence [Contract].** Curve-on-curve relations shall state shared support, exact representation correspondence, congruence, or generated dependence and domain coverage. Isolated point/contact equations shall not be presented as proof of complete coincidence.

**SK-CON-042 — Strength and activation [Contract].** Hard authored conditions, replaceable weak/completion conditions, soft objectives, driven observations, saved suppression and temporary relaxation shall be separate semantics. The solver/recipe policy shall identify which conditions contribute to accepted-design DOF and which stabilize a temporary realization.

**SK-CON-043 — Group and instance identity [Contract].** Rigid grouping shall preserve selected internal shape independently of pose. Reusable blocks shall additionally expose definition/instance maps, insertion frame, internal/external references, update and explode policies. Flexible nested structures are a distinct advanced form, not implicit in rigid grouping. [Block precedent][IN6]

**SK-CON-044 — Admissibility [Contract].** Positive radii, finite-domain bounds, branch conditions and regularity prerequisites shall be checked even when represented outside backend equations. General inequality/priority solving may be unsupported, but equality satisfaction shall not bypass advertised domain semantics.

### Advanced intent generation and management

Recognition, auto-dimensioning, automatic full-constraining, symmetry/pattern discovery and design-intent repair are proposal systems. They may use rules, numerical analysis, optimization or learning; no AI mechanism is required. Several valid parameterizations can match the same current shape and lead to different future edits.

**SK-AUT-001 — Proposed intent [Contract].** Recognition and auto-constrain tools shall expose scope, permitted relation/measure families, datum choice, assumptions, generated provenance and alternative proposals where available. Adding constraints and modifying geometry shall be separately controllable; geometric changes shall show displacement and tolerance policy before commitment.

**SK-AUT-002 — Full-constrain semantics [Contract].** An automatic completion shall report whether the chosen formulation removes physical freedoms at the analyzed configuration. It shall not claim to have recovered unique user intent, proven global uniqueness, or removed singular ambiguity. Generated completion conditions shall remain editable/removable or strengthenable.

**SK-AUT-003 — Suppression and configurations [Contract].** Saved inactive constraints, configuration-conditioned values/activation, temporary gesture relaxation, hidden geometry and source suppression shall remain distinguishable. Configuration evaluation shall be transactional and report newly invalid dependencies. Document configuration ownership is outside the local solver.

SOLIDWORKS Full Define exposes relation choices and dimension schemes; Fusion AutoConstrain exposes alternatives and optional geometry changes. These support a reviewable intent-proposal model. Creo's current weak **dimensions** supply replaceable completion; its documentation describes weak geometric constraints as legacy behavior before Wildfire 4.0, so that historical policy must not be generalized to modern Creo. [Full Define][SW10], [AutoConstrain][FU7], [Creo constraint terminology][CR6]

## 10. Geometry × constraint compatibility matrix

These tables specify mathematical/product compatibility, not current backend availability. `P` is any valid point-like reference, `L` a line direction/support, `C` a circle/circular arc, `E` an ellipse/elliptical arc, and `S` a regular Bézier/B-spline/NURBS curve. A conic represented as a spline retains conic semantics only when explicitly declared. Endpoints and centers accessed through `P` are explicit semantic references. Construction and reference roles do not change compatibility; editability may prevent movement.

**SK-MAT-001 — Common compatibility [Contract].** Validation and capability preflight shall use these meanings, including the qualifications below.

| Relation | P–P | P–L | P–C | P–E | P–S | L–L | L–C | L–E | L–S | C–C | C–E | C–S | E–E | E–S | S–S |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Coincident points | Yes | — | — | — | — | — | — | — | — | — | — | — | — | — | — |
| Point on support/domain | — | Yes | Yes | Yes | Yes | — | — | — | — | — | — | — | — | — | — |
| H/V point alignment | Yes | — | — | — | — | — | — | — | — | — | — | — | — | — | — |
| Parallel/perpendicular supports | — | — | — | — | — | Yes | — | — | — | — | — | — | — | — | — |
| Collinear | — | — | — | — | — | Yes | — | — | — | — | — | — | — | — | — |
| Tangent contact | — | — | — | — | — | Collinear contact | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes |
| Concentric | — | — | — | — | — | — | — | — | — | Yes | Centers | — | Centers | — | — |
| Equal size default | — | — | — | — | — | Lengths | — | — | — | Radii | — | — | Semiaxis pair | — | — |
| Distance default | Euclidean | Normal | Named mode | Named mode | Named mode | Parallel only | Named mode | Named mode | Named mode | Named mode | Named mode | Named mode | Named mode | Named mode | Named mode |
| Angle default | — | — | — | — | — | Yes | — | — | — | — | — | — | — | — | — |
| Equal curvature at locations | — | — | — | — | — | Trivial zero | Impossible finite radius | Impossible regular ellipse | Zero at S | Signed radii | Local | Local | Local | Local | Local |

“Named mode” means a specific measure such as center distance, radial gap, or normal distance at witnesses; it is not permission for one ambiguous generic Distance constraint. A nonparallel pair of infinite lines has minimum distance zero and therefore is not a useful separation dimension. A tangent relation on full curves must additionally determine whether contact lies within each trimmed domain.

| Single-entity or multi-reference rule | Valid references | Qualifications |
|---|---|---|
| H/V direction | Segment/infinite line; ellipse axis; spline endpoint tangent | The tangent/axis variant is distinct from constraining the whole curve |
| Midpoint | P + finite segment; P + arc/curve with named measure | Parameter midpoint and half-arclength differ for general curves |
| Radius / diameter | C | Spline curvature radius is a separate local measure; E has semiaxes |
| Length | Finite segment, arc, E interval, S interval | Curved lengths need numerical integration and derivative support for driving |
| Fix | Any native supported entity or P | Fixing a center does not fix its radius; whole-curve fix must define weights/knots policy |
| Symmetry | Two P + axis; matching entity pairs + axis | Mapping of endpoints/control structure explicit; pair compatibility alone is insufficient |
| Endpoint G1/G2 | Two endpoint-bearing regular curves | Requires coincidence; full circles need a chosen parametric contact, not a fictitious endpoint |
| Spline handle constraint | Named control/fit/derivative reference | A control polygon direction equals endpoint tangent only under stated representation conditions |
| Equal length of arbitrary curves | Two finite intervals | Integral measure relation, distinct from Equal size |
| Intersection point | P + two supports | Composite of two point-on relations plus witness/branch selection |

**SK-MAT-002 — Conditional support [Contract].** A constraint query shall report which prerequisite is absent: geometry type, endpoint/contact selection, continuity, finite domain, backend equation, or diagnostic capability. “Spline supported” shall not imply spline–circle tangency or editable rational weights.

### Conic and semantic-reference supplement

`Q` denotes a classified regular parabola/hyperbola/general conic site or support. Circle/ellipse remain separately named because their measures/landmarks differ. Compatibility means mathematical applicability, not that every backend implements the relation.

| Relation | P–Q | L–Q | C/E–Q | Q–Q | Q–S | Qualification |
|---|---|---|---|---|---|---|
| Incidence | Yes | Via endpoint/site | Via selected sites | Via selected sites | Via selected sites | Support/finite domain and selected branch |
| Tangency | — | Yes | Yes | Yes | Yes | Regular witnesses; endpoint and arbitrary-contact forms distinct |
| Concentric | — | — | Central Q only | Central Q only | Only declared center semantics | Parabola has vertex/focus, no center |
| Equal curvature / G2 / G3 | — | Conditional zero curvature at a regular site | Conditional | Conditional | Conditional | Evaluate sites, signed orientation and derivative order; no universal feasibility |
| Named distance / angle | Site/landmark measure | Support/site measure | Named sites/witnesses | Named sites/witnesses | Named sites/witnesses | Do not infer one default “distance between curves” |
| Equal shape parameters | — | — | Matching named quantity only | Matching shape semantics | Declared conic correspondence only | Ellipse axis, eccentricity and rho are not interchangeable |

| Reference facet | Relations / measures that apply | Preconditions and exclusions |
|---|---|---|
| Position | Coincidence, point alignment, distance, symmetry, fixation | Authored point or valid curve landmark/site |
| Direction | Parallel/perpendicular, angle, H/V, symmetry | Nonzero support/axis/tangent/handle direction |
| Center / focal / conic landmark | Incidence, distance, alignment, conic conditions | Landmark defined for current type/classification |
| Regular curve site | Contact, tangent-angle, curvature, G-continuity | Endpoint/interior/witness distinction; available regular jet |
| Parameter jet | C-continuity, derivative magnitude/direction | Parameter map/scaling and sided derivative defined |
| Finite interval | Length, midpoint variants, interval correspondence | Domain and traversal; integral quality for curved measures |
| Coefficient / knot / weight | Fix, equality, ratios, authoring conditions | Representation-specific, nonphysical gauges eliminated |
| Matched entity/group set | Congruence, symmetry, pattern, rigid shape | Explicit member/site maps and transform meaning |
| Prescribed external realization | Observation and constraints moving native geometry | Local solve cannot alter upstream source without separate coupling capability |

A regular ellipse has nonzero curvature, so line–ellipse G2 is impossible; a regular parabola/hyperbola site generally has nonzero curvature too, while a freeform inflection can meet a line with G2. Applicability and feasibility are separate: adding a compatible relation can still overconstrain available shape freedoms.

## 11. Dimensions and parameter system

A dimension has three separable parts: a geometric measurement definition, an optional driving expression/constraint, and an annotation. Annotation placement must not change mathematics.

**SK-DIM-001 — Driving versus driven [Contract].** Driving dimensions contribute equations; driven dimensions observe an accepted solution and consume no DOFs. Changing mode shall be an undoable, preflighted transaction. A conflicting new driving dimension may offer conversion to driven, but the mode change shall be visible.

**SK-DIM-002 — Units [Contract].** Persist a canonical length unit and radians for internal angles. Parse unit-bearing input, verify dimensional consistency, and separate display precision from stored precision. Changing displayed units shall not rescale geometry.

**SK-DIM-003 — Expression graph [Contract].** Support named scalar/length/angle and meaningful derived-unit parameters, arithmetic, constants and an extensible documented function library with unit/domain rules. Resolve names to stable IDs; detect undefined references, cycles in acyclic formulations, division by zero, nonfinite values and domain violations before solving. Nonsmooth/piecewise functions shall expose their limitations for driving/sensitivity.

**SK-DIM-004 — Evaluation order [Contract].** Acyclic parameter expressions shall evaluate upstream parameters, local targets, geometry, then driven measures, with dependency errors reported before solving. A driven measure shall not silently feed its own driving chain. Explicit simultaneous parameter/geometry equations are a separate supported formulation with unknowns, units, branch and failure semantics; cycles are errors in a DAG, not inherently invalid mathematics.

**SK-DIM-005 — Orientation [Contract].** Signed distances, internal/external angles, reflex angles, and radius/diameter modes shall be explicit. A displayed angle shall not unexpectedly change from `30°` to `150°` after a drag. Mirror/reverse operations shall remap orientation semantics.

**SK-DIM-006 — First dimension scaling [Contract].** Optional first-dimension scaling shall be a previewed command with a stated affected set. It shall not change externally referenced or expression-driven geometry silently.

**SK-DIM-007 — Annotation state [Contract].** Store label position, text formatting, and visibility separately from the measure. Moving a label does not solve. A driven value whose prerequisites are invalid shall show unavailable/stale status, never a plausible unlabeled old value.

**SK-DIM-008 — Parameter ownership [Contract].** Distinguish document user parameters, dimension target values, geometry coefficients, and backend unknowns. A solver's numbered parameter shall not become a public parameter ID.

**SK-DIM-009 — Tolerances are not constraints [Contract].** Display tolerances such as `10 ± 0.1 mm` shall not silently create solver inequalities or change model accuracy. Nominal-value design and tolerance analysis are separate concerns.

Creo's weak dimensions are replaceable system-generated conditions that can be strengthened; editing them can invoke first-sketch scaling. This is a distinct completion/strengthening interaction policy; uncompleted hard-design freedom must remain visible. [Creo dimension behavior][CR2] Onshape documents driving/driven modes and automatic driven conversion for an otherwise overdefining dimension. [Onshape dimensions][ON4]

### Full dimensional and parameter design space

| Concept | Meaning | Additional contract / difficulty |
|---|---|---|
| Driving dimension | Equation on selected geometric measure | Target validity, branch and derivative support |
| Driven/reference measure | Observation of accepted realization | No DOF removal; stale/unavailable state explicit |
| Weak/completion dimension | Replaceable generated scaffold, optionally exact within current solve | Removal/strengthening policy; hard-design freedom reported separately |
| Soft dimension / priority | Preference objective or ordered condition | Explicit residual/priority; no silent weakening of hard equations |
| Locked/manual/generated | Provenance and editing policy | Origin is independent of mathematical strength |
| Signed component / distance | Directed separation along named frame/normal | Negative values and endpoint order meaningful |
| Angular forms | Included, oriented, reflex, sweep, tangent/axis, multi-turn where applicable | Saved wrapping/winding and branch policy |
| Circular / conic measures | Radius, diameter, semiaxes, focal/eccentricity/rho parameters | Rho convention belongs to authoring form |
| Integral measures | Arc/curve length, perimeter, area | Measurement versus driving; integration error and topology changes |
| Coupled measures | Equal length, ratio, difference, equations between dimensions | Units, zero denominators, nonlinear/cyclic formulation |
| Dimension schemes | Baseline, ordinate, chain, pattern dimensions | Annotation/parameterization workflows, not automatically new relations |
| Named parameters | Scalar/length/angle and derived-unit quantities; sketch/document scope | Name resolution to IDs; unit-aware functions and dependencies |
| Configuration values | Alternative values/activation under document configuration | Rebuild/undo and missing-reference behavior |
| Parameter exploration | Stepping/sweep, alternative branch previews, planar locus tracing | Sampled exploration, not assembly simulation or all-solution proof |

**SK-DIM-010 — Completion dimensions [Contract].** Weak/completion dimensions shall identify generated origin, replacement/strengthening policy and whether their equations are active in the current realization. Their presence shall not convert underdefined authored hard intent into “fully constrained” without an explicit distinction. Weak is not a synonym for least-squares soft, driven or inactive. [Creo dimensions][CR2]

**SK-DIM-011 — Coupled measures [Contract].** Equalities, ratios, differences and equations between measures shall identify quantities/units and dependencies. A measurement capability shall not imply its driving formulation is supported. Integral and minimum-distance driving shall disclose continuity/witness assumptions and diagnosis when these fail.

**SK-DIM-012 — Annotation and generation [Contract].** Dimension schemes and automatically generated dimensions shall preserve measure identity separately from layout, grouping and provenance. Manual, creation-inferred, recipe-generated, completion, recognized and driven origins shall remain inspectable.

**SK-DIM-013 — Parameter scope [Contract].** Local and document parameter scopes shall have deterministic name/ID resolution and unit rules. Document-global dependency cycles and configuration evaluation belong to document scheduling; a local solver shall not acquire arbitrary document mutation authority.

**SK-DIM-014 — Exploration [UX contract].** Dimension stepping/sweeps and alternative-solution previews shall retain the original accepted state, identify sampled values and branch changes, and commit or cancel explicitly. Failure at one step is neither proof that later values are infeasible nor permission to jump branches silently. [SolveSpace exploration precedent][SS1]

## 12. Solver requirements

The solver solves a numerical problem prepared from the design definition. It does not decide which curves the user meant to trim, which region to extrude, or which lost reference is the intended replacement.

**SK-SOL-001 — Request/result boundary [Contract].** A solve request shall identify the definition revision, prescribed geometry, active constraints, initial state, tolerance policy, and optional interaction preferences. A result shall identify the same revision and return candidate numerical state and structured diagnostics. Stale results shall not commit.

**SK-SOL-002 — Hard constraint validity [Contract].** Every accepted solution shall pass neutral intrinsic-geometry and hard-constraint residual checks. Each residual shall use appropriate units/scaling. A backend success code alone is insufficient.

**SK-SOL-003 — Status dimensions [Contract].** Report numerical outcome separately from constraint analysis:

| Numerical outcome | Constraint/geometry analysis, independently |
|---|---|
| Converged | Satisfied, rank/DOF estimate and confidence |
| No convergence | Suspected conflict, poor initial guess, conditioning, or unknown cause |
| Infeasible, with qualified evidence | Identified incompatible subset and diagnostic confidence |
| Invalid input / unsupported | Specific source IDs and semantic form |
| Cancelled / time budget exhausted | Last valid accepted state retained |
| Backend error | Recoverable adapter/backend failure information |

Additional flags shall cover redundancy, singular/near-singular configuration, branch change, partial analysis, and invalid candidate geometry. “Under/fully/over constrained” is a user summary of these fields, not the raw result enum.

**SK-SOL-004 — Degrees of freedom [Contract].** Report physical geometric freedoms under the stated active authored conditions, with scope and analysis method. Exclude intrinsic identities, representation gauges and nonphysical helper/witness freedom from public DOF. At regular equality configurations, qualified Jacobian nullity may describe local first-order freedom; at singularities it is not proof of finite motion or global rigidity. Generated completion and soft/drag preferences shall be accounted for separately, and active bounds may require an admissible-motion cone rather than raw equality nullity.

At a regular solution, local DOFs relate to the nullity of the constraint Jacobian after accounting for intrinsic representation identities and prescribed variables. Rank is tolerance-dependent. At singularities, first-order freedom need not represent finite motion; a zero local DOF count does not prove a unique global solution. A free shape in the plane can carry global translation and rotation freedoms as well as internal shape freedoms.

**SK-SOL-005 — Dependency versus inconsistency [Contract].** Distinguish satisfied duplicate/structurally dependent relations, configuration-local Jacobian dependence, partial dependence of multi-equation relations, incompatible conditions, and unavailable analysis. A local dependency at a singular state shall not be asserted globally redundant. Diagnosis shall state active scope, tolerance and basis/order sensitivity where relevant; dependent conditions shall not be deleted automatically.

**SK-SOL-006 — Diagnostic qualifications [Contract].** Report a conflict set as a candidate, validated inconsistent subset, irreducible subset, or minimum-cardinality set only when that property is established. Never imply that there is a unique guilty constraint or that failure to converge proves inconsistency.

**SK-SOL-007 — Branch intent [Contract].** Preserve choices such as circle tangency side, arc sweep orientation, angle quadrant, intersection witness, and linkage orientation. Use previous accepted state as the primary initial guess. Record important branch choices in the neutral model or edit request rather than relying only on opaque backend history.

**SK-SOL-008 — Transactional acceptance [Contract].** Failed numerical attempts shall not overwrite the last accepted geometry. Definition edits may remain as an explicitly invalid draft with last-good display, but draft state and accepted state shall be separately accessible. Ordinary command failure and cancellation shall restore the prior accepted transaction.

**SK-SOL-009 — Incremental performance [Contract].** Reuse mappings and factorization/structural information where supported. Dirty independent components should be solved independently. Do not require full backend reconstruction for each cursor event. Adapters may reconstruct internally when necessary, but must meet the interaction budget or report degraded support.

**SK-SOL-010 — Numerical policy [Contract].** Normalize lengths and equations consistently, manage angular and positional residuals separately, bound iterations, detect nonfinite steps, and report final residuals. Numeric controls shall be available for diagnostics without becoming mandatory user setup.

**SK-SOL-011 — Priority [Contract].** Hard design constraints outrank cursor preferences and automatic soft guidance. Optional priorities shall be lexicographic or otherwise guarantee hard satisfaction; arbitrary weights that quietly violate a dimension are unacceptable.

**SK-SOL-012 — Reproducibility [Contract].** For a specified implementation/version/configuration, seed and deterministic tie-breaking policy, equivalent repeated inputs shall have equivalent accepted semantics within documented numerical policy. Cross-backend/platform bitwise equality is not required. Record configuration when comparing failures or gesture regressions.

SolveSpace documents initial-state-dependent numerical branch choice and least-squares treatment of under-constrained motion. This supports treating initial conditions and drag objectives as part of the integration contract; it does not establish a guarantee of global convergence. [SolveSpace solving technology][SS2]

### Mathematical scope of analysis and solving

Numerical solving, graph decomposition, witness/Jacobian analysis and rigidity theory provide different information. Entity/constraint counting is not enough: geometry can introduce nonstructural dependencies, gauges or singular motion. An infinitesimally rigid state need not establish a globally unique configuration; a nonzero first-order null vector need not integrate into a finite motion at a singular point. These distinctions are mathematical obligations for honest diagnostics, not a mandate for one analysis algorithm. [Solver survey][A6], [decomposition and design intent][A7], [recent review and reference map][A8]

**SK-SOL-013 — Solution branches [Contract].** The solve request/result shall distinguish selected seed/branch, found alternatives, suspected ambiguity, and search coverage. Finding two solutions may establish nonuniqueness; finding one shall not claim uniqueness. Alternative exploration is optional, but any advertised enumeration shall state its mathematical family/domain and completeness limitations.

**SK-SOL-014 — Priority semantics [Contract].** Hard equalities/admissibility, replaceable completion equations, inequalities and soft objectives shall have declared acceptance and ordering policies. A successful preference optimum with violated hard intent shall not be reported as an accepted hard solve. Backend limitations shall be surfaced before the relevant formulation is advertised.

**SK-SOL-015 — Incremental consistency [Contract].** Incremental/session results shall satisfy the same semantic validation as a fresh solve. Cache reuse shall be revision/structure-aware; timeout/cancellation and deep diagnostics shall not publish partial invalid geometry as accepted. Performance optimization shall not change relation meaning.

Global solution enumeration, minimum conflict sets, complete rigidity classification and robust inequality systems are substantial advanced/research capabilities. They remain on the map even when a practical product uses bounded local analysis. Witness methods are candidate tools with documented limitations, not universal diagnostic oracles. [Witness-method limitations][A9]

## 13. Dragging and interactive solving

Dragging communicates intent. A cursor is a desired target under constraints, not a command to write coordinates and subsequently hope the solver repairs them.

**SK-DRG-001 — Gesture lifecycle [Contract].** Provide begin/update/commit/cancel semantics. Capture the accepted start state and selected semantic references. Cancel restores that state; commit is one undoable action. The domain supports this lifecycle without knowing mouse buttons.

**SK-DRG-002 — Stable targets [Contract].** Targets shall be measured from the gesture's original references and total cursor displacement, not accumulated from previously projected coordinates. Multiple selected targets shall be submitted together.

**SK-DRG-003 — Selection intent [Contract].** Distinguish endpoint dragging, center translation, circle radius change, whole-curve translation, and rigid-set motion. Curve selection shall not arbitrarily translate its control polygon or resize it. Show the available gesture mode.

**SK-DRG-004 — Locality and branch continuity [UX contract].** Nonsingular gesture updates shall preserve selected branch intent, accepted hard conditions and declared protected geometry. Product evaluation shall measure unrequested displacement and step discontinuity on recorded gesture fixtures, with thresholds chosen for the interaction scale. Minimal movement, continuation, trust regions or other methods are candidate policies, not a uniquely prescribed optimization. Ambiguity or a limit shall be reported instead of an unexplained remote solution.

**SK-DRG-005 — Singularities [Contract].** Near a toggle point or collapsed configuration, report limited/ambiguous motion. If a branch change is necessary, require an explicit alternate-solution or relaxation action. No universal continuous branch can be guaranteed across a singularity or disconnected solution space.

**SK-DRG-006 — Unreachable cursor [Contract].** A constrained point may stop short of the cursor. Show actual versus requested position and the limiting constraint context. Fully fixed geometry remains fixed with feedback; the interface shall not appear broken.

**SK-DRG-007 — Relaxation [Contract].** A separate mode may temporarily suppress a named set of relations/dimensions. Preview and commit shall state whether relations are restored, suppressed, removed, or have changed targets. Formula-driven dimensions and external dependencies shall not be relaxed implicitly.

**SK-DRG-008 — Scheduling [Contract].** Coalesce obsolete cursor updates, support cancellation between bounded work units, and perform a final solve on release. Never publish a late solve for an older cursor or definition revision. Fast drag tolerances may be looser than commit tolerances, but previews shall be labeled and final acceptance revalidated.

**SK-DRG-009 — Gesture reversibility evidence [UX contract].** Recorded nonsingular forward/reverse gesture fixtures shall check constraint satisfaction, branch continuity and bounded unrequested drift under the declared policy. Exact return is not a theorem for underconstrained systems with path-dependent preferences or hysteresis. Repeated gestures with the same starting state/configuration shall be compared reproducibly; commit/cancel remains an exact transaction-level obligation.

NX makes relation relaxation/ignoring explicit. Inventor exposes relax-drag settings and protection for equation-based dimensions. These are useful advanced workflows after ordinary constrained dragging is dependable. [NX relation relaxation][NX3], [Inventor relax settings][IN4]

### Drag intent and allowable freedom

An endpoint target, body translation, radius handle, fit-site move and tangent-handle adjustment are different solve requests. Whole-object dragging should preserve chosen internal shape when possible while a pole drag may intentionally change it. The UI must tell the user whether motion is constrained, limited, temporarily relaxed, approximate, ambiguous or pending.

Hard-design DOFs should remain visible even when temporary target penalties choose one motion. A generic rigid group's up-to-three planar placement freedoms differ from independent-member freedom; shape stabilizers such as a circle's rotational symmetry do not create physical DOF. A work/inference region can limit candidate search or select movement preferences; it must not ignore dependencies needed to honor hard relations. At a toggle, cusp, collapsed interval or domain boundary, the admissible continuation may disappear or become nonunique. No general rule can guarantee continuous continuation through every singularity without an explicit user choice.

**SK-DRG-010 — Intent visualization [UX contract].** Before/during dragging, the selected sub-element and protected shape/pose conditions shall be visible. Coupled free-motion visualization shall state local/heuristic status; a backend unable to expose a motion basis shall report that limitation rather than displaying arbitrary arrows as exact DOFs.

**SK-DRG-011 — Relaxation lifecycle [UX contract].** Temporary relaxation shall identify affected conditions, reason/priority, trial deviations and whether release restores, commits or requests a design change. Cancel shall restore activation and values as well as geometry. Saved suppression and configurations shall remain separate operations.

## 14. Computational geometry

This subsystem answers questions about already defined curves. It need not know dimensions, native solver IDs, or UI selection.

**SK-CG-001 — Core query set [Contract].** Supply interval bounds, point containment, closest point, curve intersections, self-intersections, length, oriented area contributions, and point-in-region queries for supported types. Each operation shall receive an explicit numerical policy and domain.

**SK-CG-002 — Broad and narrow phases [Contract].** Use conservative spatial bounds to prune pairs, then curve-aware intersection algorithms. Rendering tessellation may seed a search but shall not be authoritative for model connectivity or absence of intersection.

**SK-CG-003 — Intersection events [Contract].** Return source parameter pairs/intervals, coordinates with error evidence, multiplicity where established, and independent applicable facets: transverse/tangential alignment, crossing/touching behavior, endpoint/interior, isolated/overlap, regular/singular and complete/unresolved coverage. A tangential event can cross; one exclusive tangent/touch enum is insufficient.

**SK-CG-004 — Overlap [Contract].** Coincident supports with overlapping domains shall return interval correspondences and orientation, not an arbitrary sample point or an infinite list. Full duplicates and partial overlaps shall be distinguishable.

**SK-CG-005 — Declared completeness [Numerical obligation].** Intersection and other topology-sensitive searches shall declare domains, supported representation/degree/weight envelope, event-resolution and error policy. Within that envelope they shall isolate all relevant events or identify unresolved intervals/parameter boxes and inability to establish completeness. A small residual at found roots shall not certify that none were missed; truncated unbounded searches shall not claim full-support coverage.

**SK-CG-006 — Offset semantics [Contract].** An equidistant offset is the normal-distance locus, not a scaled copy. Return raw offset components, singularities, and cleanup status. Joining, removing loops, and selecting retained components are separate operations/policies.

**SK-CG-007 — Projection [Contract].** Projection onto a plane shall return changed curve types and collapses explicitly: a tilted circle may yield an ellipse; an edge viewed along its direction may collapse to a point. Projection and intersection with a plane are different operations.

**SK-CG-008 — Query reuse [Contract].** Picking, inference, editing, and topology should share neutral geometric queries and compatible tolerances. They may choose different quality/performance policies; a UI nearest-point result shall not be mistaken for a certified topology event.

An offset of a regular curve may develop cusps where the offset scale factor vanishes, and can self-intersect even if the source is simple. General spline/ellipse offsets usually cannot be represented exactly by a same-degree polynomial spline or an ellipse. A derived offset evaluator or bounded approximation is needed. OCCT explicitly separates offset evaluation from removal of self-intersections. [OCCT offset curve][G3]

### Query obligations and algorithmic difficulty

| Query family | Analytic / numerically solved cases | Multiplicity and topology sensitivity | Result obligation |
|---|---|---|---|
| Evaluation / jets | Analytic formulas, spline basis evaluation, procedural evaluator | Sided derivatives at knots; zero speed/denominator/singular point | Order/domain/regularity checked; no NaN propagation |
| Parameter inversion | Linear/angular inverse; roots or closest-point search | Multiple parameters at self-intersection or periodic seam | All found candidates and coverage; point-on versus nearest distinguished |
| Extrema / critical points | Derivative roots per analytic or polynomial/rational span | Flat intervals, repeated roots, nearly coincident extrema | Isolated roots/intervals or unresolved spans; include endpoints |
| Bounds | Analytic extrema, convex-hull or subdivision bounds | Loose bound okay; false negative broad-phase exclusion is not | Conservative bound, or clearly approximate estimate unsuitable for exclusion |
| Closest / curve-curve distance | Analytic cases; stationary equations plus endpoints | Multiple local minima, equal minima, support/domain distinctions | Local/global qualification; witnesses and uncertainty |
| Length / inversion | Exact line/circle formulas; quadrature/root solve | Zero-speed intervals, nonunique inverse on collapsed interval | Error/bound or estimate; monotonic domain and convergence |
| Area contribution | Oriented integral `1/2 ∫(x dy−y dx)` | Traversal, self-crossing and closure; curve alone has no region | Integral quality; topology defines region before interpreting area |
| Intersections / self-intersections | Analytic pairs; polynomial/rational isolation; bounded numerical methods | Cross/touch/repeated contact, seam duplicates, overlap intervals | Event classification, parameter pairs/maps, resolution and completeness |
| Overlap / equivalence | Analytic support equality; algebraic/representation correspondence | Continuum of solutions, retracing, partial overlap, approximate near-overlap | Intervals and contributor map; sampled equality only heuristic |
| Point classification | Point-on-boundary, winding/parity, arrangement point location | Boundary/touch/sliver resolution, multiple components | Inside/outside/boundary/uncertain under declared policy |
| Restriction / split / reverse | Analytic intervals and exact polynomial/rational conversion | Representation seams, discontinuities, one-to-many maps | Locus/parameter mapping, identity handled by editor |
| Projection / section | Affine planar projection; provider/model plane intersection | Type change, collapse, multiple components, tangential section | Exact/approximate form, source correspondence, failure domain |
| Extension | Natural analytic support; explicit freeform continuation | Several forward contacts or no contact; extrapolation instability | Strategy/search domain and branch; no arbitrary universal extension |
| Offset | Analytic lines/circles; normal evaluator and approximation | Cusps, self-intersections, collapse, reversed pieces, multiple components | Raw-locus quality distinct from cleaned boundary quality |
| Approximation / tessellation | Adaptive representation/display sampling | Chord error alone may miss small loops/tangencies | Metric and achieved bound/estimate; angle/topology objectives separate |

**SK-CG-009 — Critical-point queries [Numerical obligation].** Advertised extrema, inflection, singularity and regularity queries shall return sites/intervals with sidedness, domain coverage and evidence. Conservatively subdividing at algorithmic critical points shall not create authored sketch points.

**SK-CG-010 — Integral queries [Numerical obligation].** Length and oriented area queries shall state absolute/relative error policy and achieved bound or estimate; driving forms shall provide compatible derivatives or qualified numerical sensitivity. Region area/perimeter shall depend on a current topology selection, not an unordered collection of curve integrals.

**SK-CG-011 — Classification [Contract].** Point classification shall distinguish boundary, interior, exterior and unresolved cases, with the selected region/winding policy and tolerance. Display tessellation shall not be authoritative for model containment.

**SK-CG-012 — Approximation purpose [Contract].** Approximation shall identify whether its objective is display, editable replacement, geometric query acceleration or downstream geometry. Claimed positional error shall not imply topology preservation or derivative/curvature accuracy. Topology-sensitive use needs additional validation.

Analytic formulas are candidates, not automatically robust solutions: nearly tangent quadratics can be ill-conditioned. For freeform intersections, possible families include interval subdivision/isolation, Bézier clipping, implicitization/resultants and filtered/exact polynomial predicates. For bounded procedural curves only weaker guarantees may be available. Choosing algorithms is an explicit research decision. MIT documents missed/repeated-root behavior and tangency challenges; this specification requires honest event coverage, not one prescribed method. [Planar curve intersections][A10], [intersection methods][A11]

A **curve normal offset** and a **region distance-boundary offset** are different constructions. The latter adds corner/join rules and removes extraneous portions using topology. General offsets need not be same-degree or rational curves; exact-offset special families such as PH curves are optional research representations, not mandatory primitive classes. [Offset singularities][A12]


Additional optional planar analysis includes centroid and area moments of a resolved region. These are integral measurements over selected area, not new curve types or mandatory solver constraints; driving them adds separate coupled/topology sensitivity obligations.

## 15. Editing operations

### 15.1 Common editing algebra

An edit is a transformation of a design definition, accompanied by reference and intent migration. Geometry computation alone does not complete it.

**SK-EDT-001 — Edit plan [Contract].** Each structural operation shall prepare an inspectable plan containing created/updated/deleted entities, source-to-result interval maps, sub-element remaps, constraint actions, dimension actions, dependency effects, and diagnostics. Commit is atomic for ordinary operational failures.

**SK-EDT-002 — Constraint dispositions [Contract].** Every affected constraint shall be classified as preserved, remapped, replaced by equivalent relations, deliberately removed, or unresolved. The plan shall explain removals and unresolved cases. A relation shall not be silently copied when its meaning changes.

**SK-EDT-003 — Dimension dispositions [Contract].** Every affected driving and driven dimension shall receive the same treatment, preserving expression identity and annotation references when meaningful. A dimension to the old whole curve is not automatically a dimension to one fragment.

**SK-EDT-004 — Preview and undo [Contract].** Display prospective geometry and meaningful constraint losses before committing. One gesture is one command; failed previews do not mutate accepted state. Undo restores IDs, expressions, reference bindings, and selection intent along with geometry.

**SK-EDT-005 — Shape at commit versus future shape [Contract].** Exact splitting preserves the current locus. It does not by itself preserve the original curve family's behavior under future edits. An independent split and a linked split shall be separate policies; additional continuity or shared-support relations must be explicit.

**SK-EDT-006 — Identity under structural edits [Contract].** A split may retire the parent and allocate children, or retain its ID for one designated child, but that policy shall be deterministic and documented. In either case, publish a one-to-many lineage map; a reference to the old whole must not automatically become a reference to the retained-ID fragment. Join supplies the inverse many-to-one mapping. Retired IDs are not recycled, and unresolved semantic references are never rebound by numeric ID coincidence.

### 15.2 Operation contracts

Each row defines semantics whenever the operation/form is advertised and includes inputs, computation, topology, preservation, failure, and interaction behavior. Shared rules SK-EDT-001–005 apply to every row.

| ID / operation | Inputs and geometry computation | Topology and constraint/dimension policy | Failure cases and desired UX | Form qualification |
|---|---|---|---|---|
| SK-EDT-010 Split | Curve plus interior parameter or selected intersection; exact restriction | One entity becomes ordered children; retain outer endpoint references, create separate coincident seam endpoints. H/V and support-direction relations can propagate; old total length must not become each child's length. Offer construction-parent preservation for complex intent | Endpoint split is a no-op/error without a tiny fragment; preview seam and children; ambiguous event requires selection | Finite curves; native exact restriction |
| SK-EDT-011 Trim | Picked interval plus active cutting curves; enumerate and order intersection events | Remove interval, shorten or split entity. Preserve unchanged endpoint/center relations; remap new endpoint contact to cutter; remove or explicitly replace whole-length/equality/symmetry conditions whose meanings no longer survive | No boundary: visibly preview deletion of whole entity; tangencies/overlaps require defined event policy; read-only curves cannot be directly trimmed | Declared pair/event and domain coverage |
| SK-EDT-012 Power Trim / knife | Gesture crossing visible eligible fragments; domain hit testing then Trim or Split plans | Recompute affected fragments after each provisional cut; deduplicate already-processed intervals; one history command. “Knife split” retains both sides, unlike trim | Gesture backtracking, missed thin segments, changing intersections, dense overlaps; show cut trail and pending removals, allow cancel | Gesture over advertised trim/split forms |
| SK-EDT-013 Extend | Endpoint, boundary and extension policy; intersect extended support or new continuation | Move existing endpoint reference when appropriate; preserve support/radius relations, add contact to boundary. Conflicting fixed endpoint/length is reported, not discarded | No intersection, branch ambiguity, full-circle limit, backwards extension, unsupported spline continuation; preview chosen nearest forward event | Analytic support or declared freeform continuation |
| SK-EDT-014 Join | Ordered touching curves; test common support or create composite | Exact collinear/cocircular merge may replace entities with one; migrate surviving endpoints and support dimensions. Otherwise join into wire without erasing component identities | Gaps, branching degree >2, inconsistent orientation, incompatible dimensions; distinguish “join path,” “merge curves,” and approximate fit | Support merge, chain and concatenation distinct |
| SK-EDT-015 Offset | Curve/chain, signed distance, join/cap choice; construct parallel/equidistant locus and resolve intersections | May create many entities/components/holes or no result. Independent result and associative recipe are separate modes with separately advertised relations. Do not copy source length dimensions to offsets | Offset collapse, cusps, narrow-channel removal, overlaps, discontinuous normals, disconnected outputs; show side, all components and retained region | Raw/cleaned, independent/associative forms |
| SK-EDT-016 Fillet | Two selected branches/endpoints, radius, trim/keep-original policy; compute admissible contact-center/radius solutions; offset intersections are one candidate construction | Trim parents and insert tangent arc with endpoint contacts/radius dimension. Preserve unaffected endpoint relations. Offer original corner as construction witness for dimensions; migrate corner dimensions only with explicit witness | Too-large radius, parallel/coincident supports, multiple centers, zero tangent, short remainder; show alternatives near picked corner | Selected finite/support contact branches |
| SK-EDT-017 Chamfer | Two line branches and two setbacks or setback+angle; compute line cut points | Trim lines and create connecting segment and intended dimensional constraints. Preserve old corner as optional construction witness; do not relabel original full-edge dimension as shortened length | No usable corner, insufficient lengths, conflicting fixed endpoints; show setbacks and orientation | Named setback/angle/curve measure |
| SK-EDT-018 Move | Selected entities or point references, displacement; transform proposal then solve | No structural split. Preserve constraints/expressions; partial selection may cause allowed coupled movement. Rigid-set mode differs from independently dragging handles | Hard constraints prevent requested transform; show achieved motion and limiting relations | Constrained proposal or explicit intent transform |
| SK-EDT-019 Rotate | Set, pivot, angle; rigid transform proposal | Preserve internal geometric relations; axis-relative H/V and external dimensions may block motion. A command to rotate design intent must explicitly remap/rewrite those constraints | Fixed/external conflicts, wrong pivot, reversed angle branch; live preview with clear pivot | Constrained proposal or explicit intent transform |
| SK-EDT-020 Scale | Set, center, signed uniform factor or explicitly supported affine map | Scale lengths/radii only under an explicit target-edit policy; angles unchanged. Do not multiply a shared parameter silently. Nonuniform scale changes curve kinds and many constraints | Zero/singular scale, undeclared reflection/type change, external/read-only geometry, expression conflicts, nonuniform unsupported; preview changed dimensions | Uniform or declared affine conversion |
| SK-EDT-021 Mirror | Set and line axis; reflect geometry and orientation | Copy or associative mirror are distinct. New IDs for copies, internal relations remapped, shared parameters retained by chosen policy; sweep and signed angle semantics reflected | Coincident duplicate at axis, ambiguous external references, unsupported relation transforms; preview originals/copies and association | Independent copy or associative relationship |
| SK-EDT-022 Linear pattern | Seed, vector(s), spacing/count, include-seed policy | New stable instance/member IDs. Copy internal relations; dimensions choose shared expression or independent value. Associative count/spacing represented by recipe, not an unbounded equality graph | Invalid count/spacing, duplicates, huge instance count, external constraint explosion; preview count and total extent | Independent copies or associative recipe |
| SK-EDT-023 Circular pattern | Seed, center, count, angular spacing or total sweep | Transform members; distinguish total sweep from per-instance angle and full-ring duplicate. Persist instance identity across parameter changes when correspondence is clear | Count change removes referenced instance, duplicates at 360°, singular pivot; preview affected instances and dependencies | Instance map, total/per-step angle policy |
| SK-EDT-024 Copy | Entity/constraint subset plus placement | New IDs; remap all internal references. Crossing constraints are reported and either retained against same external target or omitted by an explicit policy. Parameters choose shared link or value copy | Missing dependencies, incompatible destination units/plane, unsupported types; preview source and target and list dropped links | Subset with declared dependency policy |

### 15.3 Preservation examples

**SK-EDT-030 — Split dimension policy [Contract].** Splitting a 100 mm line at 40 mm shall not create two 100 mm dimensions. Valid choices include a 100 mm dimension between the original outer endpoint references, a parent construction line carrying the original dimension, or explicit removal/conversion if the selected policy cannot express equivalent intent. “Preserve where possible” is insufficient without a chosen rule.

**SK-EDT-031 — Curve property versus interval property [Contract].** Circle radius and center relations can survive trimming to an arc; arc length, segment length, midpoint, and equality of interval lengths usually change meaning. A point-on relation survives only if its witness remains in the retained domain or its meaning explicitly uses the full support.

**SK-EDT-032 — Spline control constraints [Contract].** Exact subdivision changes control coefficients. A constraint on an old control point cannot generally be attached to a nearest new control point. Preserve it through a retained parent/control definition, express a proven equivalent relation, or diagnose its loss.

**SK-EDT-033 — Tangency remapping [Contract].** A tangent relation tied to a retained endpoint/contact may survive. Tangency at a removed point shall not migrate to a new endpoint without recomputation and user-visible intent. Inserting a fillet shall not retain the original sharp-corner coincidence as if the original endpoints still met.

Inventor explicitly describes split inheritance of direction relations and conditional loss of equal/symmetric relations. SolveSpace documents that its split replaces original entities and loses their constraints unless originals are retained as construction. These are concrete evidence that editing policy is a separate product decision. [Inventor split/trim/extend][IN2], [SolveSpace split behavior][SS1]

SOLIDWORKS documents gesture-based Power Trim and separate options to retain trimmed geometry as construction or exclude construction from trimming. These are useful interaction and intent-preservation policies above the same geometric engine. [Power Trim][SW4], [Trim options][SW5] CATIA's connecting-curve workflow offers tangent/curvature continuity and requires isolating the associative connection before some structural edits, another example of editability depending on feature provenance. [CATIA connecting curves, mirrored vendor help][CA5]

### Additional structural edit contracts

| ID / operation | Inputs / computation | Identity, constraints and dimensions | Failure / interaction |
|---|---|---|---|
| SK-EDT-025 Merge | Several supports/entities/sites; exact support equivalence or explicit coincidence merge | Many-to-one lineage; retain contributor provenance; define surviving authored IDs and semantic sites; whole measures are not summed blindly | Incompatible supports, conflicting prescriptions, ambiguous duplicate intent; preview survivor and removed links |
| SK-EDT-026 Replace geometry | Existing entity and new compatible/different representation/locus; exact conversion or fitted substitute | Identity retention policy and subreference correspondence; preservation can be semantic, exact or approximate, not simply same coordinates | Type/landmark changes, missing handles, incompatible equations; preview remapped and unresolved conditions |
| SK-EDT-027 Convert role/source | Boundary/construction/axis, native/generated/reference, live/frozen/detached modes | Role change can alter profile eligibility without locus change; detachment changes dependency and editability; preserve IDs or provide explicit map | Source unresolved, newly partitioned regions, constraints no longer prescribed; preview dependency/topology effect |
| SK-EDT-028 Delete with dependencies | Entities, sites, dimensions, conditions or recipe members; dependency closure | Distinguish delete relation, suppress relation, delete geometry, delete generated parent and detach output; dependent objects get disposition | Referenced children, shared parameters, downstream selection loss; dependency preview and atomic rollback |
| SK-EDT-029 Open/close/seam edit | Closed/periodic curve and seam site, endpoint/continuity policy | Opening can create two endpoint identities at one position without a geometric gap; closing may add intent or alter locus; parameter/site map required | Derivative mismatch, periodicity loss, incompatible references; show positional closure versus seam continuity |

**SK-EDT-034 — Semantic migration [Contract].** A structural edit shall map semantic references by meaning: persistent site, source interval, support property, fit condition, handle, whole-entity/group or generated dependency. Coordinate proximity alone is insufficient. One-to-many maps shall preserve alternatives; many-to-one maps shall retain contributor provenance; unresolved references shall be explicit.

**SK-EDT-035 — Future behavior [Contract].** An edit plan shall distinguish current-locus equivalence from preservation of future design intent. Independent children, shared-support children, retained construction parent and associative recipe are separate choices. Equivalent current geometry shall not imply equivalent future parameter response.

**SK-EDT-036 — Transform semantics [Contract].** Transform commands shall state whether they propose constrained motion, transform a detached copy, or rewrite design intent. Uniform negative scale may be represented as positive scaling with planar rotation; reflection/nonuniform affine maps require explicit orientation/type/measure remapping. H/V and external relations need not remain invariant. Shared expressions shall not be silently changed to satisfy a transform.

**SK-EDT-037 — Migration evidence [Contract].** Preservation/replacement shall state whether equivalence is exact semantic lowering, tolerance-bounded shape approximation, or heuristic correspondence. A successful edit shall not claim all intent preserved when a whole-curve condition becomes a child condition or a refit loses authored fit/handle meaning.

A general fillet can have multiple valid circles, no solution, or a solution beyond selected finite branches; curve combinations need picked contact neighborhoods and explicit trimming. General chamfer requires a named setback/tangent/arc-length convention and is not automatically the line-corner recipe. Offsets may collapse/change component count. These are capability qualifications and research areas, not reasons to omit the operations from the target.

Recipe regeneration must not recreate a manually removed condition unexpectedly. Either the recipe owns that condition and its override is saved, or editing detaches/modifies the recipe. Suppression differs from deletion and must preserve enough intent for deterministic reactivation.

## 16. Intersection / trim / split requirements

### 16.1 Pair coverage

**SK-INT-001 — Pair coverage [Contract].** Every delivered pair below shall support domain-clipped intersections and use the same event semantics for trim, split, and topology. A pair omitted from the active capability set must return Unsupported, not an empty intersection list.

| Pair | Required events and complications | Search qualification |
|---|---|---|
| Line–line | Transverse, endpoint, parallel-disjoint, coincident full/partial interval, near-parallel | Analytic supports and finite domains |
| Line–circle/arc | 0/1/2 events; tangency; clip against segment/ray and arc sweep | Analytic roots plus domain filtering |
| Circle/arc–circle/arc | 0/1/2 events; internal/external tangency; coincident circular intervals; seams | Analytic roots/overlap and seam filtering |
| Line–ellipse/elliptical arc | Quadratic support intersection with stable near-tangent handling and interval clipping | Conic roots plus finite filtering |
| Circle–ellipse; ellipse–ellipse | Multiple isolated roots, tangencies, coincident same ellipse, periodic domains | Conic root/overlap isolation |
| Spline–line | All roots per span; repeated roots/touching; collinear spans and endpoint roots | Span roots and continuum intervals |
| Spline–circle/ellipse | All interval solutions, not only sign-changing roots; rational denominator validity | Polynomial/rational event isolation |
| Spline–spline | Parameter-pair isolation/refinement, near tangencies, overlaps, repeated spans, self-intersection reuse | Declared representation/domain envelope |
| Curve–itself | Exclude the trivial parameter diagonal, retain distinct-parameter crossings and overlaps, treat periodic seam once | Distinct parameters and seam equivalence |

**SK-INT-002 — Ordered events [Contract].** Events shall be sorted along the edited curve's oriented parameter domain. Cluster only events justified by both geometric and parameter/error information. Close spatial points on distant parameter branches must remain distinct.

**SK-INT-003 — Tangential and crossing behavior [Contract].** Tangential events shall be detected without requiring a sign change. Local crossing/touching behavior shall be classified separately from tangent alignment: regular curves y=x³ and y=0 are tangent at the origin and cross. Topology shall use side/order evidence, not a blanket tangent-means-touch rule. Trim may use either event as a boundary under a stated previewed policy.

**SK-INT-004 — Overlap selection [Contract].** Trimming against coincident geometry shall offer the overlap interval or ask for a specific retained side; it shall not choose a numerically arbitrary cut point. Duplicate curves shall remain inspectable until a separate repair command removes them.

**SK-INT-005 — Exact freeform restriction [Contract].** Bézier/B-spline/NURBS trimming shall preserve the selected retained locus through exact restriction and stable parameter mapping. Splitting at an existing knot, at a repeated knot, near an endpoint, and across a periodic seam shall be defined. Knots/weights shall not be discarded through polyline conversion.

**SK-INT-006 — Extension policy [Contract].** Freeform extension shall name the strategy: expose more of an existing underlying support, extend the endpoint polynomial span within bounds, append tangent continuation, or append a fitted continuity-controlled segment. There is no single canonical extension of an arbitrary trimmed spline.

**SK-INT-007 — Closed-curve cuts and seams [Contract].** Full periodic supports shall not expose arbitrary evaluator seams as authored endpoints. Closed nonperiodic curves may have meaningful authored endpoint/seam conditions. Cutting/opening shall specify traversal, retained locus, endpoint identities and continuity: a one-cut opening can leave coincident ends and the same region locus; it shall not encode a full traversal as an ambiguous zero-sweep arc.

**SK-INT-008 — Completeness strategy [Numerical obligation].** Freeform event processing shall declare its curve/domain envelope, root/contact/overlap isolation and resolution policy, termination bounds, and unresolved parameter regions. Detect repeated/tangent contacts rather than relying on sign changes or isolated Newton seeds. Candidate method families remain open; profile admission shall use the actual evidence supplied, not an algorithm name.

Onshape documents shape-preserving Bézier splitting with independent child controls and a coincident join. This is a useful reference for distinguishing current-shape preservation from future control behavior. [Onshape Bézier editing][ON2] OCCT's curve intersector exposes both isolated points and tangential intersection segments; the exact interpretation depends on its tolerance policy. [OCCT intersection API][G1]

### Event-driven editing obligations

Line-line, line-circle, circle-circle, line-ellipse, spline-line, spline-circle, spline-conic and spline-spline forms share one event vocabulary but require different narrow-phase methods. Restrictions on finite intervals, periodic seams, self-contacts and overlap intervals apply after support-level computation. Intersection coordinates alone cannot identify which source branch/interval a trim should remove.

**SK-INT-009 — Arbitrary-curve editing coverage [Contract].** An advertised trim/split form shall identify all eligible curve-pair event forms and unresolved cases. Spline/NURBS child geometry shall be exact restriction or an explicit bounded approximation if the source representation cannot be restricted exactly; native polynomial/rational splines shall not be silently refitted. Trimming at tangent contact shall follow a stated touch-as-boundary policy, distinct from transverse crossing.

**SK-INT-010 — Event resolution [Contract].** Near-coincident events shall be clustered using both parameter and spatial/domain evidence, without merging distant traversal branches merely because they share coordinates. Multiple/high-order contacts, self-retracing intervals and nearly tangent separation shall retain uncertainty until the numerical policy establishes an ordering or returns unresolved status.

Power Trim is a gesture that chooses deletion intervals; knife/cut is a gesture that chooses split sites while retaining pieces. Both use ordinary edit plans and migration, with trail feedback, gesture resampling/error policy, cancellation and one-command history. A display hit must be converted to a model-domain query before committing.

Advanced conformance shall include tangential crossing (`y=x³` against `y=0`), even-order touch, higher-order contact, repeated roots, self-retracing intervals and periodic seam duplicates, with independent alignment/side/event facets.

## 17. Planar topology

The proposed pipeline is sound if it builds an **arrangement**, not merely a graph of endpoints:

```text
accepted eligible curve intervals
  → validated intersections + self-events + overlaps + critical decomposition
  → normalized derived fragments with all contributor/source interval maps
  → embedded half-edge arrangement and boundary walks
  → bounded cells / contact / nesting information
  → selected cells and regularized unions
  → consumer-specific open-wire or area-profile snapshot
```

CGAL's arrangement model supplies a well-developed conceptual precedent: geometry traits compute/split intersections and overlaps, while arrangement topology manages vertices, half-edges, and faces. This is architectural evidence, not a recommendation to add CGAL immediately. [CGAL arrangements][G4]

### 17.1 Vocabulary

| Concept | Definition / caveat |
|---|---|
| Vertex | Derived endpoint/intersection/contact junction with contributor sites and closure evidence; may be artificial for algorithmic seam handling |
| Fragment / edge | Maximal eligible curve interval between relevant events under the chosen decomposition; shared overlap retains multiple sources |
| Half-edge | Oriented boundary use with twin/incident-face and local ordering; exact source parameter interval retained |
| Wire | Ordered connected edge uses; may be open or closed; not every wire is a simple boundary |
| Simple loop | Simple closed boundary with an explicit Jordan/manifold qualification; distinct from a repeated face walk |
| Cycle | Graph-theoretic closed traversal; not automatically a face or useful region |
| Face / bounded cell | Connected open 2D component of the plane complement; boundary can have several walks/components; unbounded supports can create several unbounded faces |
| Boundary walk | Face-incidence traversal; may repeat bridge edges or vertices and need not be a simple loop |
| Outer / hole boundary | Boundary component in an oriented region representation; nesting/contact rules belong to the selected region model |
| Disconnected component | Geometry graph component or area component; these are different notions |
| Coverage multiplicity | Number/source identity of coincident contributors; does not automatically multiply material or create another face |
| Sliver | Small/thin bounded cell relative to chosen resolution; may be real design geometry, not automatically removable |

**SK-TOP-001 — Derived, non-destructive topology [Contract].** Detecting an intersection shall not create authored entities, split source curves, or add coincidence constraints. Topology is a derived view of one accepted revision.

**SK-TOP-002 — Embedded arrangement [Contract].** Adjacency shall include angular/local curve ordering around vertices, orientation, periodic seams, and overlap multiplicity. An endpoint graph alone cannot classify regions at crossings or tangencies.

**SK-TOP-003 — Boundary events [Contract].** Include endpoints, proper crossings, tangential contacts, and overlap boundaries. An endpoint lying in another curve's interior shall split the derived interval there. Isolated points do not partition the plane.

**SK-TOP-004 — Connectivity evidence [Numerical obligation].** Derived connectivity shall use semantic incidence and geometric event/endpoint evidence under a declared closure budget. A numerically satisfied coincidence constraint does not create exact coordinate identity. Record closure residuals, contributor sites and any tolerant interpretation; do not silently move authored geometry or admit a gap beyond policy.

**SK-TOP-005 — Nontransitive tolerance [Contract].** Clustering shall avoid an unchecked chain where A is near B and B near C but A and C exceed the permitted closure error. Each cluster needs a bounded geometric extent and deterministic representative policy.

**SK-TOP-006 — Overlap normalization [Contract].** Normalize unambiguous shared loci into derived boundaries while retaining all source contributors, orientation, intervals and coverage multiplicity. Duplicates/partial overlaps shall remain diagnosable and repairable. Reject or qualify only unresolved event/reference ambiguity or boundary forms inadmissible to a selected consumer; existence of duplicate authored geometry alone does not make the geometric region undefined.

**SK-TOP-007 — Tangencies and singular nodes [Contract].** Resolve local ordering using curve-side information when tangent directions alone tie. Touching loops shall not be joined into a fabricated single manifold loop. If the ordering cannot be determined reliably, return an unresolved topology diagnostic.

**SK-TOP-008 — Invalidation [Contract].** Geometry, construction/profile role, external update, tolerance-policy, and suppression changes shall invalidate affected topology. A cached topology object shall carry the exact input revision/policy identity. Presentation-only edits shall not invalidate it.

**SK-TOP-009 — Incremental correctness [Contract].** A whole-sketch rebuild is acceptable initially within measured limits. Incremental rebuilding may follow, but its output shall be equivalent to a full rebuild. Reuse of stale fragments is never an acceptable performance shortcut.

**SK-TOP-010 — Derived identity [Contract].** Derived topology handles shall carry revision/validity context. Identity may persist across an incremental update only when correspondence is established. Stored selections shall not assume persistence from array position or reused handle alone; otherwise resolve via provenance/policy or report ambiguity.

### 17.2 Complications the simple pipeline must handle

- A full circle has no endpoint; traversal needs a deterministic artificial seam that is not an authored point.
- A bridge/dangling edge may appear twice in one face boundary walk; it must not become a spurious area boundary.
- A disconnected arrangement can give one face multiple boundary components. Enumerating all graph cycles both overcounts and can be exponentially expensive.
- A bow-tie contains bounded cells, but its entire authored wire is not one simple boundary. Region selection and wire selection must differ.
- Two tangent circles bound two disks; their contact does not authorize a single simple loop through the touching point.
- Overlapping duplicates can create zero-area walks unless normalized; arbitrary small sliver deletion changes topology.
- A gap may close under a parameter edit and a face may split into several cells. There is no universal persistent identity across that event.
- A low-resolution polyline can miss a small loop or create a false crossing. Display quality must not change the profile list.

### Arrangement normalization and difficult events

T-junctions, transverse crossings, isolated vertices, tangencies, higher-order contacts, partial overlap and retraced intervals all require explicit event/local-order treatment. Some arrangement methods require extrema/x-monotone decomposition; these algorithmic cuts need not become authored or user-visible vertices. Sorting tangent vectors alone may fail at tangency: local curve-side/jet information or a qualified unresolved result is required.

This is a conceptual pipeline, not necessarily a sequence of full rebuilds. It is not equivalent to enumerating graph cycles. Unbounded supports need an algorithm/provider policy for arrangement extent; viewport clipping must not create fake bounded profiles. Most ordinary profile eligibility can remain finite while unbounded axes are construction.

**SK-TOP-011 — Boundary incidence [Contract].** Face extraction shall distinguish simple loops, repeated boundary walks, bridges, holes, isolated vertices and disconnected contributors. Open wires shall be available separately. A dangling edge inside a region shall not invent a second face.

**SK-TOP-012 — Contact and resolution [Contract].** Tangent/point contacts, slivers and unresolved ordering shall carry explicit status, scale/error evidence and source locations. Removing them is an explicit healing/edit operation, not an automatic byproduct of profile detection.

CGAL provides a primary reference for DCEL arrangements, geometric traits and boundary components; its Boolean model formalizes regularized sets. This supports the conceptual distinction, without claiming commercial Sketchers use these algorithms or prescribing CGAL as a dependency. [Arrangement reference][G4], [regularized set operations][G6]

## 18. Profiles and regions

**SK-PRO-001 — Selectable cells [Contract].** Shade and select bounded cells using geometric containment and the accepted arrangement. Permit selecting multiple adjacent cells and form their regularized union by removing shared internal boundaries. Show open wires separately.

**SK-PRO-002 — Nested selection [Contract].** An outer circle and an inner circle shall expose the inner disk and surrounding annulus separately. Selecting both yields the full outer disk. A third nested loop shall not be assigned material solely by an undocumented even/odd rule.

**SK-PRO-003 — Profile value [Contract].** A closed profile snapshot shall contain:

- sketch/document feature identity and accepted geometry revision;
- sketch-plane placement or a stable placement reference supplied by the feature;
- one or more connected region components;
- ordered outer and hole boundary uses, with orientation and source restrictions (exact where representable), plus explicit approximation/endpoint closure budgets;
- source-entity/subinterval provenance, tolerance/approximation information, area/bounds;
- validity classification and diagnostics, including closure residuals;
- an owned or immutable lifetime guarantee independent of later solver scratch storage.

**SK-PRO-004 — Solid boundary [Contract].** A future Solid module shall receive ready ordered closed boundaries with holes. It may validate and convert them to kernel edges/wires/faces, but shall not redo sketch intersections or guess which region was selected. Kernel conversion failures shall map back to sketch boundary provenance.

**SK-PRO-005 — Consumer admission [Contract].** Profiles shall identify current accepted geometry, resolved required sources, event/completion evidence under the numerical policy, area/component/contact structure and boundary validity. A consumer shall declare its admissible manifold/contact/approximation forms. Underconstrained valid regions are allowed. Point-touching regions and normalized duplicate contributors shall not be rejected universally: valid individual cells may coexist with an inadmissible combined boundary. Rejection shall name the consumer-specific reason.

**SK-PRO-006 — Local failures [Contract].** A defect in a disconnected component shall not unnecessarily hide valid unrelated regions. Each candidate carries validity/dependency status. A selected boundary depending on invalid geometry shall never use an unlabeled last-good substitute.

**SK-PRO-007 — Persistent profile intent [Contract].** Store selection intent with source boundary provenance and a region witness/relationship policy. After an edit, return same, updated, split, merged, missing or ambiguous resolution. Unique correspondence must be established before automatic reuse; otherwise request explicit selection or use a saved qualified policy. A nearest-centroid match is only a candidate, not identity proof.

**SK-PRO-008 — Measurement independence [Contract].** Area/perimeter calculations shall use analytic integration where practical and controlled quadrature otherwise. Display tessellation shall not determine manufacturing-profile validity or authoritative measurement error.

Fusion explicitly presents open and closed sketch profiles as different inputs to modeling operations. Its user-facing profile concept is evidence for exposing regions as first-class products, not evidence about its hidden arrangement implementation. [Fusion sketches and profiles][FU2]

### Regions, unions and consumer profiles

Authored geometry is persistent intent. Derived topology is a revisioned interpretation. A **selectable region** is a bounded cell or explicitly selected union with contact/nesting information. A **profile** is a consumer-ready snapshot of the chosen interpretation. These objects need not share identities or lifetimes.

Regularized union removes lower-dimensional appendages from an area interpretation; it does not guarantee a simple or manifold combined boundary at every point contact. Tangent disks still supply two individually valid disk cells; selecting both may yield a point-touching boundary that some consumers must split or reject. Holes can touch other boundaries under certain set representations, while a future kernel may impose stricter rules. Preserve the geometry/contact facts and validate the requested consumer policy. [Regularized Boolean reference][G6]

**SK-PRO-009 — Region operations [Contract].** Selected-cell unions and other advertised region set operations shall preserve constituent/provenance information and produce normalized exterior/hole boundary uses under an explicit regularization policy. Shared interior boundaries shall be removed from the union boundary without deleting authored curves.

**SK-PRO-010 — Snapshot assurance [Contract].** A profile shall separate represented-locus exactness, endpoint closure, intersection completion, approximation quality and consumer admission. “Certified” shall name the verified property/domain; checked or heuristic claims shall remain qualified. Downstream conversion shall not silently reuse stale geometry.

## 19. External/reference geometry

References are a dependency system with geometry adapters, not merely fixed copies.

**SK-REF-001 — Reference record [Contract].** Store source document/feature identity, source element selector, projection/section mode, source revision, association state, cached evaluated geometry, and provenance. Solver-native IDs are never source selectors.

**SK-REF-002 — Provider boundary [Contract].** A neutral reference-provider boundary shall resolve points, curves, axes and plane supports from sketches, datums and future model/B-Rep sources, without exposing kernel classes or native solver handles. Supported provider kinds are separately advertised. Sketch owns planar realization and constraint consequences; the document/provider owns upstream source identity and scheduling.

**SK-REF-003 — Association states [Contract].** Distinguish live, unresolved, ambiguous, suppressed, frozen snapshot, and detached native copy. Reference display and diagnostics shall show these states. Breaking a link is an explicit operation preserving the last evaluated shape, not a silent repair.

**SK-REF-004 — Read-only geometry [Contract].** Associated geometry is prescribed for the local solver. Constraints may position local entities against it, but the local solver cannot move the upstream source. A direct edit offers detachment or editing the source.

**SK-REF-005 — Update transaction [Contract].** Resolve upstream changes before solving dependents. Preserve the last accepted sketch if a source update produces an invalid projection, unsatisfied constraints, or ambiguous mapping. Report whether the problem is reference resolution, solve, or topology.

**SK-REF-006 — Dependency direction [Contract].** Document scheduling shall detect cycles in one-way sketch references and acyclic expressions before indefinite rebuild. Explicit bidirectional/coupled sketch solving is a separate formulation with scope, unknowns, ownership and failure policy; foreign handles shall not create accidental coupling.

**SK-REF-007 — Rebinding [Contract].** Offer source replacement with a preview of compatible sub-element mappings and affected constraints/dimensions. Candidate matching can use type, provenance, orientation, and proximity, but ambiguity shall not be resolved silently.

**SK-REF-008 — Topology change preparation [Contract].** A source edge may split, merge, disappear, reverse, or change curve type. References shall allow one-to-many candidates and explicit failure, not assume a permanent array index. Persistent naming remains the document/B-Rep provider's responsibility; Sketch owns the consequences for its references and constraints.

**SK-REF-009 — Projection variants [Contract].** Orthogonal projection, in-plane inclusion, section intersection and view-dependent/apparent contour generation shall be distinct modes. Axes/planes may yield directions, lines, a whole-plane relation or degeneracy rather than a finite curve. Silhouette/apparent references shall declare view/source dependence and tracking limits; unsupported forms remain explicit.

**SK-REF-010 — Reference profile use [Contract].** Explicitly included reference boundaries can contribute to profiles. Merely visible background geometry cannot. Solids shall not consume stale projected boundaries while the UI reports a healthy sketch.

Fusion's lost-projection workflow exposes re-link, break link, and delete. Onshape documents that used geometry updates but geometry-type changes and silhouette tracking have restrictions. Inventor documents several projection-specific associativity rules, including cases converted to fixed curves. OurPaint should make such states uniform and explicit instead of inheriting each provider's fallback behavior. [Fusion projection repair][FU3], [Onshape Use][ON1], [Inventor projection][IN3]

### Reference-provider guarantees and information

The future B-Rep/document provider need not promise perfect naming. It must make failure and correspondence explicit. Sketch can then migrate semantic references or ask for repair without guessing kernel indices.

| Provider information / outcome | Required Sketch interpretation |
|---|---|
| Source document/feature identity, opaque selector and source revision | Stable namespace and dependency scheduling; no assumption selector is an array index |
| Source coordinate frame/placement, unit convention and target-plane context | Well-defined neutral projection/inclusion/section; provider may use a documented canonical frame/unit |
| Resolved / missing / suppressed / ambiguous / unavailable | Separate status, candidates and cause; no unlabeled cached substitute |
| Neutral geometry/components and source interval/orientation | Evaluate projection/inclusion/section; retain exact/approximate provenance |
| Same / reversed / split / merged / replaced / type changed | Candidate element/subinterval correspondence, map direction and confidence |
| Parameter/landmark correspondence where known | Migrate endpoints/centers/sites/support properties; missing map becomes unresolved |
| Tolerance/error/regularity and binding policy | Decide admissible solving, editing, topology and consumer use |
| Lifetime and immutable revision snapshot | Prevent a provider update from mutating geometry during a solve/query |

**SK-REF-011 — Provider resolution contract [Contract].** Provider resolution shall return revisioned geometry/provenance and explicit status/candidate mappings. Split/merge/type-change outcomes shall distinguish established correspondence from heuristic candidates. The contract permits unresolved naming; it does not promise every source has a permanent identity.

**SK-REF-012 — Source-state conversion [Contract].** Live association, frozen snapshot and detached editable copy shall retain distinct definitions. Freeze preserves a chosen realization and provenance; detach removes the live dependency while providing native editing semantics. Rebinding shall preview affected references, constraints, dimensions and topology.

**SK-REF-013 — Semantic source migration [Contract].** A type change or split/merge update shall revalidate each dependent semantic site/interval and relation. A source that remains geometrically nearby shall not be accepted automatically as the same center, endpoint, whole edge or fit handle without correspondence evidence.

Plane placement belongs to the containing feature/document provider. A tilted circle can project to an ellipse, and an edge can collapse to a point; projection is not necessarily type preserving. Section curves are generated from source-plane intersection, not a local 2D solver trick. Source reference suppression, local construction role and simple visibility must have separate solve/profile effects.

## 20. Snapping and inference

The three layers must be explicit:

| Layer | Effect | Example |
|---|---|---|
| Visual snap | Chooses a cursor placement or highlighted candidate | Nearest point on a reference curve |
| Temporary inference | Guides or softly influences the current gesture | Temporary horizontal alignment to a hovered endpoint |
| Persistent automatic constraint | Writes a semantic design relation on commit | Coincidence accepted at the end of line creation |

**SK-INF-001 — Candidate engine [UX contract].** Candidate families shall include endpoints, centers, datums, intersections, midpoint variants, H/V, parallel/perpendicular, tangent/contact, conic landmarks, equal-size and multi-object alignment where mathematically valid. Each advertised family shall identify source sites, domain/regularity and intended relation. Search/ranking may be scoped and bounded; unsupported candidates shall not masquerade as committed constraints.

**SK-INF-002 — Screen-space acquisition [Contract].** Use pixel-distance thresholds, zoom-aware candidate acquisition, and enter/leave hysteresis. Persist the exact resulting constraint or exact geometric placement; do not use pixel thresholds as model closure tolerances.

**SK-INF-003 — Ranking and scope [Contract].** Prefer explicit user selections and hovered references over unrelated nearby geometry. Resolve ties deterministically and support cycling candidates. Provide scope controls for active sketch, construction, and external/background geometry.

**SK-INF-004 — Preview meaning [Contract].** Use distinguishable feedback for snap-only, temporary relation, and persistent relation to be added. Show referenced entities and relevant icons/text. If preflight rejects an automatic relation, the final state shall not misleadingly display that relation as committed.

**SK-INF-005 — Commit policy [Contract].** A creation command shall add only the accepted independent relation set. Check for invalid references, obvious redundancy, and conflict against a trial solve. Skip/reject conflicting automatic relations with visible feedback; never delete a manual relation to accommodate them.

**SK-INF-006 — User control [Contract].** Provide discoverable modifiers for temporarily suppressing inference, locking a candidate/direction, and cycling alternatives. Exact key bindings belong to the UI configuration; they shall not be hard-coded into Core semantics.

**SK-INF-007 — Dragging policy [Contract].** Ordinary dragging shall not silently accumulate permanent constraints. A deliberate snap-and-constrain gesture or setting may create them with preview and one-step undo.

**SK-INF-008 — Selection [Contract].** Distinguish whole entities, endpoints, centers, handles, constraints, dimensions, and regions. Allow cycling overlapping candidates, type filters, crossing versus containment box selection, chain/loop selection, and preselection highlighting. A coincident point stack shall remain individually selectable.

**SK-INF-009 — Continuous tools [Contract].** Line/arc chains, typed lengths/angles, numeric entry during drawing, and continuation from the previous endpoint shall use the same constraints and transaction machinery as manual commands. Escape cancels the current provisional element before leaving a tool.

**SK-INF-010 — Accessibility and clutter [Contract].** State shall be communicated by icons/text/patterns as well as color. Constraint glyphs may be filtered and repositioned without changing their definitions; hover isolates relevant relations and targets.

Creo documents explicit control over offered constraints and cycling. CATIA's SmartPick feature explanation is particularly instructive: visually snapping had previously been confused with actually creating a constraint, and detection scope could be limited to the last hovered element. These are strong reasons to make persistence visible. [Creo inference controls][CR3], [Dassault SmartPick behavior][CA1]

### Candidate lifecycle

Snapping acquires a position/site; temporary inference offers a direction/relationship for the current gesture; persistence records design intent. Alignment guides may refer to distant geometry without incidence. Candidate rank can incorporate explicit selection, hover, recency, cursor distance, relation plausibility and tool context, but must allow scope control and cycling in dense sketches.

Equal-size/symmetry inference and contextual suggestions can be useful, yet should be hypotheses with visible affected geometry. Persistent auto-constraints should be enabled by a stated tool/user policy, checked in a trial, and grouped with creation history. A cursor guide disappears without changing definition; a committed relation persists and appears in the navigator.

**SK-INF-011 — Suggestion provenance [UX contract].** Guides and contextual suggestions shall identify their source geometry and proposed effect. Acceptance, candidate locking, cycling, numeric-entry locking and temporary suppression shall have visible feedback. A suggestion shall not silently replace a manual constraint or infer unique intent from proximity.

## 21. Interaction and mature product UX

Mathematical semantics say what a relation means; UX says how a person discovers, selects and controls it. UI colors, shortcuts and gestures are replaceable policies over stable domain operations. Mature efficiency comes from inspectable defaults, continuity of workflow and fast recovery as much as feature breadth.

| Capability | Observable behavior |
|---|---|
| Direct numeric / heads-up entry | Enter length, angle, coordinates, radius or expression during creation; show which component is locked and which remains cursor-controlled |
| Repeat / continue | Keep the active recipe/mode for repeated elements; continue mixed chains; explicit finish, cancel provisional element and cancel tool |
| Drag intent | Distinct site/body/handle/rigid-group modes, protected conditions and achieved versus requested motion |
| Constraint visibility | Selected/related glyphs, type/origin/status/activation filters; dependent highlighting; clutter control |
| DOF visibility | Remaining motion tied to relevant entities/components; coupled and local/singular qualifications |
| Navigator / browser | Geometry, semantic sites, constraints, parameters, generated recipes, sources and issues linked to viewport selection |
| Selection | Entity/site/constraint/dimension/region filters; overlap cycling; chain/loop/member selection; box/window policies |
| Profiles | Shaded cells/unions, hole/contact/open-contour feedback, current/stale status |
| Conflict recovery | Highlight implicated conditions, qualified repair alternatives, reversible trial, clear cancellation |
| Freeform analysis | Poles/fit sites/knots, tangent and curvature handles, combs, inflections, continuity inspection |
| Gesture editing | Power Trim versus knife, bounded hit acquisition, preview trail, complete one-command undo |
| Work/inference scopes | User control over candidate sources and local movement preference without breaking hard dependency semantics |
| Contextual suggestions | Continue tangent arc, close contour, apply equal/symmetric rule, repair source; proposals with explicit acceptance |
| Accessibility / efficiency | Status beyond color; discoverable modifiers; configurable keys; readable dimensions at zoom; scalable glyph density |

**SK-UX-001 — Numeric creation [UX contract].** Heads-up values shall state units, scope, locking and whether they seed placement or create persistent dimensions. Expression errors shall be shown before commit; editing a label shall be distinguishable from editing its value.

**SK-UX-002 — Browser linkage [UX contract].** Selecting an entity/constraint/parameter/source/issue in a browser shall reveal related geometry and semantic dependencies. Filters shall include type, origin, activation and health; hidden/inactive/read-only states shall be distinguishable.

**SK-UX-003 — Selection precision [UX contract].** Chain/loop selection shall state its continuity/topology policy and exclude unrelated coincident sources unless selected. Multiple overlapping sites/constraints/regions shall remain individually selectable through cycling/filtering. Geometry drag, dimension-value edit and annotation placement shall have distinct targets.

**SK-UX-004 — Pending work [UX contract].** Interactive pending/limited/failed state shall be visible. Cancellation and new edits shall supersede stale work; a pending or last-good profile shall not appear current. Progress shall use measured information rather than fabricated completion estimates.

**SK-UX-005 — Learning and repeatability [UX contract].** Active mode, next action and modifier effects shall be discoverable without relying solely on color or memorized keys. Repeat/continue behavior shall preserve selected policy, with provisional versus committed results clear.

NX's Navigator and found/persistent relation distinction, CATIA's SmartPick modes, and Onshape's filtered constraint management illustrate different useful interaction policies. These are evidence for inspectable intent and source linkage; they do not establish identical solver or topology implementations. [NX Navigator][NX2], [NX relation controls][NX1], [CATIA SmartPick][CA1], [Onshape constraint management][ON3]

## 22. Diagnostics

**SK-DIA-001 — Structured diagnostic [Contract].** A diagnostic shall contain a stable code, subsystem, severity, revision, public entity/constraint/reference IDs, localized-message key with arguments, evidence/quality, and optional recovery actions. Human-readable text is not the API.

**SK-DIA-002 — Required categories [Contract].** At minimum distinguish:

| Category | User-relevant evidence | Recovery choices |
|---|---|---|
| Free motion | DOF count/availability, movable entities or coupled groups | Drag to inspect; add a relation/dimension |
| Redundancy | Duplicate or dependent relation set; partial versus full | Keep with warning, remove, convert dimension to driven |
| Conflict | Qualified conflicting subset and involved geometry | Edit value, suppress/remove chosen relation, revert edit |
| Nonconvergence | Residuals, last valid state, solver cause if known | Retry from accepted state, inspect branch/conditioning |
| Singularity / instability | Degenerate derivatives, near-rank change, responsible candidate entities | Move away, change parameterization, select branch |
| Broken reference | Source selector and candidates/status | Rebind, freeze/detach, delete |
| Invalid geometry | Entity invariant and offending values | Correct edit or discard candidate |
| Parameter error | Expression path/cycle and source dimension | Fix expression or upstream dependency |
| Topology error | Gap endpoints, overlap intervals, self-touch/crossing, unresolved event | Zoom to location; explicit repair or different region selection |
| Editing failure | Operation stage, affected inputs and lost-intent list | Adjust selection/policy; cancel or accept explicit conversion |
| Unsupported feature | Exact semantic form and missing capability | Choose supported backend/mode or preserve unsolved definition |

**SK-DIA-003 — Repair is a transaction [Contract].** Diagnostic actions shall preview their consequences and be undoable. “Fix sketch” shall not silently add coincidence, delete duplicate geometry, or drop constraints.

**SK-DIA-004 — Bounded explanation [Contract].** Offer immediate lightweight analysis and a cancellable deeper conflict search. Time-limited diagnosis shall retain useful candidate subsets with qualified confidence. Minimum conflict explanation is not required for every nonlinear system.

**SK-DIA-005 — Whole-sketch context [Contract].** List constraints by type, source, activation, status, and referenced entity. Selecting an issue highlights the actual affected geometry, not just a generic red sketch icon. Suppressed constraints and broken links remain discoverable.

**SK-DIA-006 — Debug reproducibility [Contract].** A local diagnostic export shall capture the neutral sketch, relevant revisions, tolerances, backend/version, and interaction request needed to reproduce a problem. It shall not require shipping native pointer addresses or unrelated document data.

**SK-DIA-007 — No fabricated certainty [Contract].** Unavailable DOF/conflict/redundancy analysis shall be represented as unavailable, not zero or an empty “no problems” list. An entity suspected of instability shall not be presented as a proven root cause without evidence.

Useful precedents differ: SOLIDWORKS SketchXpert proposes repair sets; NX's Sketch Navigator groups curves, relations, external references, and issues; Onshape provides filtered constraint management; FreeCAD exposes conflicting, redundant, partially redundant, and malformed constraint categories in its API. These support a layered diagnostic model rather than a single failure string. [SOLIDWORKS diagnostics][SW2], [NX Navigator][NX2], [Onshape manager][ON3], [FreeCAD SketchObject API][FC1]

### Diagnostic evidence and repair vocabulary

| Evidence level | Example | Required qualification |
|---|---|---|
| Established structural/mathematical fact | Duplicate definition, unit mismatch, invalid knot ordering, empty analytic domain | Predicate/definition and affected scope |
| Numerical observation | Residual, iteration limit, rank under tolerance, observed sensitivity | Algorithm/configuration, scale/tolerance and analyzed state |
| Conditional assurance | Isolated events over supported domain, irreducible subset under verified deletion tests | Assumptions, verification method and completeness coverage |
| Heuristic suspicion | Likely branch switch, unstable near-singular group, probable source correspondence | Confidence/suspected wording and alternatives |
| Repair proposal | Remove/change relation, convert driving to driven, rebind, heal gap, change seed | Expected intent loss, applicability and preview; not an implied unique remedy |

**SK-DIA-008 — Evidence qualification [Contract].** Diagnostic facts, numerical estimates, suspected causes and repair proposals shall be distinguishable fields. Conflict/dependency/singularity/instability reports shall include analyzed revision/scope and relevant tolerance or evidence. Nonconvergence shall not be asserted infeasible without independent justification.

**SK-DIA-009 — Complete status families [Contract].** Diagnostics shall distinguish hard-intent underconstraint, numerical completion, full/partial redundancy, inconsistency, nonconvergence, singularity, observed instability, known/suspected branch ambiguity, broken reference, invalid geometry, topology/consumer problems, failed edit and unsupported semantic capability. Unavailable analysis shall remain unavailable, not zero or healthy.

**SK-DIA-010 — Repair lifecycle [UX contract].** Suggested repair shall identify changed intent and preserve an original checkpoint. Trial alternatives shall be inspectable and cancellable; successful residual reduction shall not imply that user intent was preserved. Automatic healing shall require an explicit command/policy and its own error/displacement evidence.

SOLIDWORKS separates several solved/sketch health states; FreeCAD source exposes full/partial redundancy, malformed conditions and geometry repair helpers. These establish the diagnostic design space, not universal completeness or a unique cause attribution. [SOLIDWORKS sketch states][SW11], [FreeCAD diagnostic APIs][FC4]

## 23. Robustness requirements

### 23.1 Tolerance policy

**SK-ROB-001 — Separate tolerances [Contract].** Define positional residual, angular residual, relative length, parameter isolation, topology clustering, approximation, and display tessellation tolerances separately. Converting one into another requires a geometric scale/derivative bound, not an arbitrary shared epsilon.

**SK-ROB-002 — Scale and supported envelope [Contract].** Use normalized local calculations and publish the supported coordinate/feature-size range. Absolute tolerance protects near zero; relative tolerance scales with relevant local lengths. A distant unrelated entity shall not inflate the tolerance enough to erase a small local feature.

**SK-ROB-003 — Resolution contract [Contract].** Final solve error shall fit within the geometry/topology error budget. Features near the resolution limit shall be reported as unresolved or below minimum size, not unpredictably merged. A user-requested tolerance change is an explicit rebuild-affecting policy edit.

**SK-ROB-004 — Predicate quality [Contract].** Use robust orientation/incidence predicates and guarded arithmetic in topological decisions. Adaptive exact predicates can prevent floating-point sign errors for represented inputs; they do not by themselves certify approximate spline roots or intended equality of noisy data. [Shewchuk robust predicates][G5]

### 23.2 Failure ownership

| Case | Primary owner | Required behavior |
|---|---|---|
| Nearly coincident endpoints | Geometry queries + topology | Measure gap, report tolerance-based join, offer explicit coincidence repair |
| Nearly tangent curves | Intersection engine + topology | Isolate/classify all candidate events or return unresolved parameter regions; distinguish proven contact from unresolved proximity |
| Tiny geometry / huge coordinates | Representation + normalization + validation | Keep finite representable values; reject unsupported resolution; avoid cancellation through local frames |
| Zero-length line | Representation; solver candidate gate | Reject committed degeneracy; a temporary creation preview may remain provisional |
| Zero-radius or collapsed arc | Representation + solver | Reject/diagnose; no hidden conversion to point |
| Degenerate ellipse or near circle | Representation + constraint semantics | Enforce positive semiaxes; handle undefined/nearly irrelevant major-axis orientation without arbitrary swapping |
| Zero spline tangent / cusp | Differential geometry + solver | Mark undefined tangent/curvature; prevent meaningless G1/G2 constraints |
| Spline self-intersection | Intersection engine + topology | Preserve curve, expose crossing cells, diagnose invalid whole-wire profile |
| Multiple roots at nearly equal parameters | Intersection isolation | Cluster only with justified bounds; preserve multiplicity/branch data |
| Full/partial overlap and duplicate curves | Geometry + topology | Return overlap intervals; retain provenance; require explicit cleanup for ambiguous profile use |
| Constraint singularity | Solver analysis | Qualified rank/DOF state and last-good geometry; no unexplained branch jump |
| Sliver cell / zero area loop | Topology + profile validation | Diagnose; do not delete automatically based on display size |
| Missing external dependency | Reference resolver | Mark broken/frozen state; block affected current-profile consumption |
| Invalid typed input | UI parser + domain validation | Immediate explanation; Core validates independently of the UI |

**SK-ROB-010 — Degenerate input [Contract].** NaN, infinity, invalid domains, negative radii/weights where disallowed, malformed knots, and invalid IDs shall be rejected before native solver calls or geometry indexing.

**SK-ROB-011 — No silent healing [Contract].** Automatic repair shall never silently change design geometry to make a region close. Repair may be suggested and previewed, with measured displacement and affected constraints.

**SK-ROB-012 — Metamorphic verification [Contract].** Geometry tests shall verify invariants under translation, rotation, permitted uniform scaling, curve reversal, operand swapping, and split/rejoin evaluation. Topology results shall be independent of insertion order and rendering tessellation within the supported numerical policy.

**SK-ROB-013 — Resource bounds [Contract].** Imported or adversarial geometry shall have bounded subdivision/iteration/memory limits, cancellation, and explicit incomplete results. No geometry query may hang the interactive thread indefinitely.

**SK-ROB-014 — Serialization [Contract].** Save/load shall preserve geometry definitions, IDs/watermarks, units, constraints, expressions, external bindings, provenance, branch choices, and profile selection intent. Derived caches are rebuildable and versioned; they are not the authoritative model.

### Resolution is not one tolerance

Kernel/model positional tolerance, solver residual scaling, parameter/event isolation error, topology closure, approximation error and display acquisition/tessellation answer different questions. They need compatible budgets, not one global epsilon. Angular and curvature tolerances have different dimensions; local feature scale must not be swamped by a distant unrelated entity.

**SK-ROB-015 — Assurance budget [Numerical obligation].** An accepted downstream result shall identify relevant representation, solve, intersection, closure and approximation errors/bounds or estimates. If combined uncertainty prevents deciding contact/order/closure at the requested resolution, report unresolved rather than fabricate a topological answer.

**SK-ROB-016 — Validity layers [Contract].** Representation validity, differential regularity, solve admissibility, arrangement completeness and consumer-profile admission shall be separate. A self-intersecting regular spline may be valid authored geometry yet unsuitable as one simple loop; a degenerate record may remain preserved but unavailable for evaluation/profile use.

**SK-ROB-017 — Error transport [Numerical obligation].** Translation, rotation, scale and unit changes shall transport dimensional error policies consistently. Metamorphic checks apply within the documented envelope and corresponding transported tolerances; bitwise/invariant results outside resolution limits are not promised.

Adaptive exact predicates help establish signs for represented input, while spline event construction and noisy-data interpretation need other methods. Exact arithmetic cannot discover an intended coincidence absent from the data. Tolerant healing is product/geometry policy, not a substitute for valid predicates. [Robust predicates][G5]

## 24. Comparison with existing CAD systems

### 24.1 Notable approaches

This is an evidence-led comparison, not a winner ranking or exhaustive current-release feature certification. “Not verified” is deliberately different from “absent.” Product-level abilities must not be assumed available through a solver library.

| System / evidence scope | Geometry, constraints, dimensions | Interaction / diagnostics | Editing, profiles, references | Idea for OurPaint and caution                                                                                                                                                                                                                     |
|---|---|---|---|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Siemens NX / Designcenter, 2023–2026 vendor articles | Dynamic profile creation, infinite lines, typed length/angle controls; distinction between found and persistent relations | Relation relaxation; Navigator groups curves, relations, external references and issues | Documented trim/profile workflows and included references; internal topology/constraint-migration algorithms not disclosed | Inspectable inferred intent and source/status browser; do not make transient relation discovery indistinguishable from persistent design rules. [relations][NX1] [Navigator][NX2] [relaxation][NX3] [creation][NX4]                               |
| SOLIDWORKS, 2025/2026 help | Analytic/conic/spline relations; equal curvature and higher continuity forms; explicit support-curve semantics | Automatic relations/snaps; SketchXpert repair alternatives; dangling relation replacement | Repair Sketch targets gaps/overlaps; rich sketch operations, but preservation needs operation-specific verification | Treat diagnostics and reference repair as core UX; relation names alone do not establish finite contact. [relations][SW1] [repair][SW2] [FAQ][SW3]                                                                                                |
| Autodesk Fusion, rolling help | Analytic geometry and splines; curvature relation; dimensional/parametric sketch workflow | Constraint glyphs and constrained state; persistent versus construction/projection roles | Open/closed profiles; projection-link repair and detachment | First-class region selection and explicit lost-link recovery; do not copy product-specific centerline participation defaults unquestioningly. [constraints][FU1] [profiles][FU2] [projection][FU3]                                                |
| Autodesk Inventor, 2025/2026 and available editing help | Interpolation/control-vertex splines, ellipse axes, G2 endpoint smoothing, spline shape controls | Configurable inference/relaxation; equation-driven dimension protection | Documented split relation inheritance, trim/extend contacts, projection-specific associations | Specify editing preservation per semantic relation; spline authoring representation and associativity matter. [constraints][IN1] [edits][IN2] [references][IN3] [splines][IN5]                                                                    |
| PTC Creo, r12/r13 help | Automatic weak dimensions, strong dimensions, reference dimensions; equal curvature and conic parameterization | Constraint offering can be locked/disabled/cycled; Resolve Sketch exposes undo/delete/reference conversion | Reference-aware sketch setup; topology persistence algorithms not established by public help | Study weak/strong intent UX, but avoid using hidden weak dimensions to pretend every sketch is explicitly defined. [dimensions][CR2] [inference][CR3] [resolve][CR4] [conics][CR5]                                                                |
| CATIA, 3DEXPERIENCE vendor feature note plus mirrored vendor help | Standard/construction elements; dimensional reference modes; spline tangent/curvature handles and continuous connecting curves | SmartPick distinguishes preview placement from auto-created relations; geometry/use-edge/diagnostic analysis tabs | Implicit profiles, open-profile repair, profile orientation, associative use-edge replacement; connections may require isolation before trim | Separate geometry health, solve health and source health. Mirror/version uncertainty prevents claiming exhaustive current geometry-pair support. [SmartPick][CA1] [analysis][CA2] [dimension modes][CA3] [connections][CA5] [spline editing][CA6] |
| Onshape, rolling help read 2026-10-03 | Bézier/control-point editing; driving/driven dimensions; constraints tied to reference mode | Filtered Constraint manager; individual glyph highlighting | Exact-shape Bézier split/trim behavior; Use projections and explicitly documented silhouette restrictions | Good model for capability qualifications and visible constraint provenance; no inference about internal region algorithm. [Bézier][ON2] [manager][ON3] [Use][ON1] [dimensions][ON4]                                                               |
| FreeCAD Sketcher, project API docs and developer posts | B-spline point-on/knot tangency; rich semantic/internal geometry references | API exposes DOF, conflict, redundant, partially redundant and malformed categories; branch-locking mechanisms | SketchObject exposes split/trim/fillet, constraint transfer, external geometry rebuilding and validation | Study open-source semantics and test cases; API presence is not proof of robustness or current universal pair support. [API][FC1] [knot tangency][FC2] [point-on-spline][FC3]                                                                     |
| SolveSpace, official manual / technology / library docs | Points, lines, circles/arcs and cubic Bézier/interpolating splines; dimensional constraints and construction geometry | Numerical solve seeded by current state; DOF visualization | Splitting can discard source constraints; modeling operations live beyond the solver library | Compact implementation reference; avoid importing destructive split semantics or assuming libslvs is a full Sketcher. [manual][SS1] [technology][SS2] [library][SS3]                                                                              |

### 24.2 Geometry coverage and evidence gaps

The sources do not justify a universal “all products support arbitrary NURBS equally” claim. User-facing spline tools can hide very different degrees, interpolation policies, knot structures, weights, and constraint sites.

| Product | Geometry specifically established by reviewed evidence | What still needs exact release/API verification |
|---|---|---|
| NX | Line/profile workflows, infinite lines, spline-oriented creation/editing in vendor articles. [spline workflow][NX5] | Complete editable ellipse/NURBS forms and each spline constraint pairing |
| SOLIDWORKS | Lines, circular/elliptical/conic and spline relation forms | Exact operation coverage for every rational/trimmed spline and higher-continuity pair |
| Fusion | Points, lines, circles/arcs, splines; ellipse-related constraint forms | Exact rational-spline authoring and topology-preservation details |
| Inventor | Lines, circles/arcs, ellipse axes, interpolation and control-vertex splines | Editable knot/weight breadth and general offset cleanup guarantees |
| Creo | Basic geometric constraints, spline equal-curvature, conic-arc rho dimension | General NURBS editing and constraint-preservation matrix |
| CATIA | Curve/profile/construction workflows, spline point/tangency/curvature editing and connecting arcs/splines. [spline controls][CA6] [connections][CA5] | Complete conic/NURBS taxonomy and exact current Sketcher pair matrix; do not substitute xDesign or surface-workbench tools as proof |
| Onshape | Basic sketch/reference curves plus Bézier controls, degree and split/trim | Full rational-weight authoring and arbitrary-contact curvature constraints |
| FreeCAD | Spline, ellipse/conic internal references and primitive APIs in reviewed source documentation | Current release behavioral coverage for every command and degenerate input |
| SolveSpace | Point, line, circular and cubic spline forms in manual | No evidence here for a general editable ellipse/NURBS Sketcher contract through libslvs |

### 24.3 Conclusions drawn from the comparison

1. The useful common baseline is analytic geometry, dimensions, relations, constrained dragging, reference geometry, editing, and downstream profiles. Those should be one coherent core mechanical slice.
2. Inference models differ substantially: persistent automatic constraints, weak dimensions, and found relations represent different intent policies. OurPaint should choose deliberately rather than mix them implicitly.
3. Spline support is a family of capabilities, not one checkbox. Endpoint tangency, arbitrary contact, control points, fit points, G2/G3, representation edits, and rational weights need separate contracts.
4. Mature products devote substantial UI to conflict and broken-reference repair. Diagnostics cannot remain a backend string if the product aims to be dependable.
5. Public user manuals rarely reveal exact profile algorithms or complete topology persistence. The arrangement/provenance proposal is an engineering recommendation, not a reverse-engineered claim about NX, CATIA, or SOLIDWORKS.
6. Siemens describes a separate Profile Geometry Manager for high-level profile/offset work alongside D-Cubed 2D DCM. Even industrial component offerings distinguish those responsibilities. OurPaint need not adopt either component to adopt that separation. [Siemens component architecture][DC3]

### Additional distinctive ideas established in the second pass

| System | Further useful concept | Qualification |
|---|---|---|
| NX | Found-relation conversion, work/inference scope, navigator-linked status | Scope cannot ignore required hard dependencies. [Controls][NX1] |
| SOLIDWORKS | Full Define relation/dimension choices; equation curves; rational style controls and high-order join command | Relation generation is one proposed parameterization; vendor continuity terminology needs mathematical normalization. [Full Define][SW10], [equation curves][SW7], [style controls][SW8], [continuity command][SW9] |
| CATIA | SmartPick policy during creation/drag; separate geometry/source/solve analysis | Mirror release confidence remains limited; no exhaustive current certification. [SmartPick][CA1], [analysis][CA2] |
| Creo | Replaceable weak dimensions; strengthening and conflict resolution | Modern weak dimensions must not be conflated with legacy weak geometric constraints. [Dimensions][CR2], [terminology][CR6] |
| Inventor | Reusable block definitions/instances; spline fitting and conversion policy | Rigid group, reusable block and flexible nested instance are separate semantics. [Blocks][IN6], [splines][IN5] |
| Fusion | AutoConstrain alternatives with optional controlled geometry displacement; conic rho workflows | Learning is a possible proposal method, not a required architecture. [AutoConstrain][FU7], [conics][FU5] |
| Onshape | Filtered constraint management; dimensionable spline handles | Handle semantics and reference mode remain explicit. [Manager][ON3], [spline][ON5] |
| FreeCAD | Weight/conic/knot-site constraints and active/driving separation in public source | Source presence establishes a form, not unrestricted product behavior. [Constraint definitions][FC5], [SketchObject][FC4] |
| SolveSpace | Ratio/difference measures, stepping and point tracing in a compact system | Planar exploration need not imply assembly simulation; libslvs remains a solver boundary. [Manual][SS1], [library][SS3] |

D-Cubed and geometric kernels are component evidence, not additional ranked CAD products. Backend/component capability cannot be inferred solely from a complete application's user interface, and product capability cannot be inferred solely from a component's feature sheet.

## Part B — Architecture implications

## 25. Capability model across subsystem boundaries

A Boolean `supportsSpline` is too coarse. Support depends on the operation, reference form, curve representation, solver mode, and quality of diagnostics.

**SK-CAP-001 — Runtime semantic query [Contract].** Provide an exact preflight query over a proposed semantic definition/request. Return support level, limitations, missing prerequisites, and diagnostic/interaction qualifications. The query shall not mutate state or consume IDs.

A concise support description may use native execution, exact lowering, derived recipe, opt-in approximation, read-only evaluation or unsupported mode, with quality and scope as separate qualifiers. These are semantic descriptions, not a mandatory exclusive enum. Approximate support shall never masquerade as exact hard-constraint satisfaction.

Examples of capability keys:

```text
constraint / tangent / line-circle / support / internal-or-external
constraint / tangent / spline-circle / endpoint-contact
constraint / tangent / spline-circle / free-contact-parameters
constraint / continuity / G2 / spline-spline / endpoint
geometry / ellipse / editable
geometry / rational-bspline / evaluate-only
solve / drag / multiple-targets / branch-preserving
diagnostic / dof / whole-sketch / local-rank
diagnostic / redundancy / partial-relations
diagnostic / conflict / qualified-subset
```

**SK-CAP-002 — Multiple providers [Contract].** Solver capability, geometry-query capability, edit capability, and profile capability shall be queried separately. A backend that lacks spline solving shall not imply that a neutral spline cannot be displayed, intersected, or referenced as prescribed geometry.

**SK-CAP-003 — Compile-time versus runtime [Contract].** Compile-time options choose available adapters/libraries. Runtime descriptors report actual adapter version and implemented semantic forms. UI feature availability is derived from end-to-end support, not hand-maintained unrelated toolbar flags.

**SK-CAP-004 — Exact fallback [Contract].** A lowering may express concentricity as center coincidence or endpoint G1 as coincidence plus a suitable tangent equation, provided semantics and diagnostics are preserved. Backend helper IDs remain private and map back to the original relation.

**SK-CAP-005 — No fake fallback [Contract].** Do not approximate an unsupported ellipse as a circle, spline tangency as a control-polygon angle, or a general offset as a scaled spline. A geometry approximation is an explicit new design operation with its own error and association policy.

**SK-CAP-006 — Backend switching [Contract].** Preflight the complete neutral definition. Preserve all data and IDs; reject the switch or install a clearly restricted/unsolved session when requirements are unmet. Switching shall not remove constraints, convert driving to driven, or silently change branch intent.

**SK-CAP-007 — Conformance tier [Contract].** An adapter may be available without meeting a complete product tier. A claimed configuration/tier shall advertise its end-to-end semantic scope and verification evidence. Each adapter shall meet the subset it advertises; reduced diagnostics, unsupported forms and measurement-only support shall remain visible. Required default tiers are delivery decisions in Part C.

**SK-CAP-008 — Capability provenance [Contract].** Distinguish library potential, adapter implementation, tested support, and current-instance applicability. A marketing page or native enum is not proof that the OurPaint adapter correctly implements that feature.

Mixing two solvers inside one coupled sketch is not a general fallback: alternating solutions can violate each other's equations and branches. Use one responsible solver per coupled constraint component unless a separately designed coupled-solving protocol exists. Geometry algorithms can be shared across backends without this difficulty.

### Semantic capability descriptions without a flag explosion

The owner of a capability can be solver, geometry service, editor, reference provider, topology service or complete UI workflow. Ask for a **semantic operation signature**, representation/site/domain qualifiers and required assurance, rather than inventing a boolean for every combination.

Examples: tangent contact of a rational B-spline interval and circle with editable interior witness; G2 endpoint join with available second derivatives; physical-DOF analysis after exact lowering; overlap-aware trim with interval lineage; periodic-NURBS region extraction under a specified event policy. Solving, dragging, diagnostic precision, editing and profile use can have different support.

Keep the descriptor conceptually simple: operation + reference/representation/domain qualifications → supported mode and limitations. Separate execution mode (native, exact lowering, derived recipe, approximation, read-only) from quality/coverage and reason. Compile-time registration selects compiled adapters; runtime query checks the actual instance/version/model form; feature flags enable UX policies rather than alter mathematical semantics.

**SK-CAP-009 — End-to-end support [Contract].** A product feature shall be available only when its required model, evaluation, solve/validation, edit, reference, topology and UX slice is supported. Partial read-only or measurement-only forms may be offered with explicit limits. Unsupported whole workflows shall not be inferred from one backend's equation support.

**SK-CAP-010 — Fallback semantics [Contract].** Exact semantic lowering may be transparent with provenance; approximation, fitting, freezing or restricted-domain fallback shall be disclosed and never used to pretend an unsupported hard relation is satisfied. Multiple numerical solvers shall not be coupled implicitly as a fallback; coupled ownership/convergence is a separate research formulation.

Detailed C++ structs, registry layout and feature negotiation are deferred. A concise signature/qualifier API plus structured reasons can provide the architectural benefit without a universal capability framework.

## 26. OurPaint architecture alternatives and recommendations

### 26.1 Current repository assessment

Inspection covered the current working tree, including ongoing uncommitted changes. This is an assessment of the files visible on the research date, not a claim about a released build.

| Inspected file | Observed design | Implication                                                                                                            |
|---|---|------------------------------------------------------------------------------------------------------------------------|
| `src/core/sketch/Sketch.h`, `Sketch.cc` | Sketch owns IDs/policy/backend lifetime; backend owns live geometry; neutral detached query values | Good public encapsulation; insufficient independence of long-term model storage                                        |
| `src/core/sketch/SketchTypes.h` | Point/line/circle/arc variant; typed IDs; construction flag; coarse capabilities and exact preflight; two solve outcomes | Useful foundation; needs richer reference, state, diagnostic, and geometry semantics                                   |
| `src/core/sketch/backends/ISketchBackend.h` | CRUD, queries, counts, construction metadata, constraints, solve and drag in one backend interface | This is both a model-store abstraction and a solver adapter, not just a numerical boundary                             |
| `DcmSketchBackend.cc` | OurPaintDCM owns coordinates; private ID maps and intrinsic arc relation; diagnostics intentionally limited | Do not equate its status/reporting with Siemens D-Cubed                                                                |
| `SolveSpaceSketchBackend.cc` | Owned libslvs records; private handles; current adapter rejects the public whole-curve tangency form | Native SolveSpace feature support and public OurPaint constraint semantics differ                                      |
| `src/core/Document.h`, `Document.cc` | Document owns one Sketch and currently selects SolveSpace | Future feature/support/dependency context belongs around Sketch                                                        |
| `src/app/editor/tools/CursorTool.cc` and architecture documentation | Neutral selections and absolute drag targets; cancel currently discards gesture state without restoring geometry | Preserve neutral selection; add transactional gesture semantics                                                        |
| `docs/en/sketch-architecture.md` | Explicit backend-authoritative storage and possible failed-iterate exposure | This specification proposes a deliberate future change; existing document remains current implementation documentation |

The current backend-authoritative arrangement is one viable ownership model, but the broader capability map requires explicit answers for unsupported definitions, rollback, persistence, editing and neutral consumers. These can be supplied around an authoritative backend; they are not inherently impossible. The alternatives below compare the costs and guarantees rather than treating coordinate duplication itself as a defect.

### Ownership alternatives: A, B and C

| Concern | A — backend-authoritative geometry | B — neutral authoritative model + adapters | C — controlled hybrid/session |
|---|---|---|---|
| Authority | Backend owns current numerical geometry; neutral facade exports meaning | Neutral definition and accepted realization own persisted meaning | Neutral persisted intent/accepted state; native session owns temporary numerical work |
| Synchronization | Native edits are direct; neutral export/import must be semantically complete | Revisioned compile/dirty update and validated acceptance | Incremental session updates and published validated snapshots/deltas |
| Persistence | Neutral export possible; representation translation and passthrough needed | Neutral schema directly; native cache disposable | Neutral schema; session internals omitted |
| Undo | Backend/neutral checkpoints with documented restore | Neutral snapshots/deltas and transactions | Accepted checkpoint plus speculative-session discard |
| Failed solve | Protect/restore prior accepted state; native queries must avoid failed iterates | Reject candidate without mutating accepted realization | Trial remains private; prior published state survives |
| Unsupported geometry | Side storage/passthrough or backend becomes a general repository | Preserved without solver support | Preserved neutrally; only supported subset compiled |
| Backend switch | Export/import preflight and representation conversion burden | Recompile same neutral semantics | Finish/discard session and start another from accepted state |
| External references | Neutral association layer plus native prescription | Neutral bindings and evaluated source snapshots | Same bindings, incrementally synchronized into session |
| Derived topology | Detached accepted export can feed independent topology | Accepted neutral snapshot is natural input | Only published accepted snapshots, never mutable trial buffers |
| Performance | Efficient native mutation; export cost for other consumers | Translation/duplication cost; dirty updates/cache reuse mitigate | Less copying during gestures; lifecycle/synchronization more involved |

**Recommendation / ADR candidate:** B is the clearest default for OurPaint's extensible target. C is a compatible optimization when authority is explicit by state and time. A can meet the product contracts, but retaining unsupported intent and making persistence/editing portable expands the backend/facade responsibilities. Choose based on semantic completeness, one-writer authority, rollback, cache lifecycle and measured cost—not a blanket rule against duplicate coordinates.

Persisted definition, realization seed/last accepted geometry, trial unknowns, helper variables and derived snapshots are separate ownership categories. One state must not have two independently mutable authorities. A fit authoring definition can produce coefficients during evaluation; these coefficients may be accepted realization data without becoming the sole authoring truth.

### 26.2 Recommended logical modules

```text
Document / feature graph / units / history / support-plane providers
                     |
           Sketch domain facade
          /          |            \
  SketchDefinition   |      Sketch evaluation state
  entities, refs,    |      last accepted geometry,
  constraints,      |      diagnostics, revisions
  dimensions, params|
                    |
       Sketch edit and solve coordinator
        |           |                |
   Edit planner  Parameter eval   Solver session
        |                            |
  Geometry services          ISketchSolverBackend
        |                      /           \
  Planar arrangement       OurPaintDCM   SolveSpace
        |
  Profiles / region selection resolution

Application interaction -> neutral commands, drag intents, inference candidates
UI/rendering <- immutable geometry, annotations, diagnostics, profile views
```

This is a responsibility diagram, not a requirement for eleven libraries, public interfaces, threads, or heap objects. Geometry services can begin as ordinary functions over variants. Topology is a derived service/cache, not a mutable child subsystem independently editing Sketch. Inference's geometric reasoning can be portable; cursor processing and screen-space ranking belong in the application.

**SK-ARC-001 — One authoritative design definition [ADR candidate].** Under candidate B/C, OurPaint should own the persistent neutral definition and accepted realization; adapters own disposable or session-scoped computational representations. Any chosen model should have one writer for each authoritative state and preserve unsupported recognized intent.

**SK-ARC-002 — Controlled numerical duplication [ADR candidate].** Native sessions/caches may duplicate coefficients as numerical representations. Synchronization should be directional and revisioned: accepted intent into compilation/session, validated candidates back into accepted state. Hybrid sessions may publish deltas instead of whole-model copies; no hidden bidirectional mutation.

**SK-ARC-003 — Backend scope [ADR candidate].** The solver adapter should translate solve semantics, own native resources/private maps, update/rebuild sessions, solve/drag and map diagnostics/results. Profiles, edit migration policy, annotations, provider rebinding, persistence and cursor inference should have independent owners even if implementation code is initially colocated.

**SK-ARC-004 — Accepted state [ADR candidate].** Authored changes, trial numerical state and last accepted realization should remain distinguishable. Snapshot, delta or copy-on-write rollback is an implementation choice. Consumers should not receive arbitrary failed native iterates as ordinary accepted geometry.

**SK-ARC-005 — Revision ownership [ADR candidate].** The coordinator should accept a result only against its input revision and invalidate explicit dependencies. Constraint-only edits invalidate solving; topology refresh depends on changed realized geometry/eligibility/source policy; annotation-only changes do not solve.

**SK-ARC-006 — Kernel independence [ADR candidate].** Geometry/profile contracts should permit independent 2D services and future kernel adapters. A full Solid kernel is neither required nor prohibited as a geometry implementation; evaluate actual query coverage, error policy, footprint, licensing and portability before an implementation decision.

### 26.3 Recommended ownership and lifetimes under B/C

| Object | Authority / lifetime | Backend relationship |
|---|---|---|
| Sketch identity and support placement | Document/SketchFeature | Solver receives local frame only |
| Entity / constraint / parameter IDs | Neutral model within documented scope | Mapped to private handles, never allocated as native public IDs |
| Endpoint/control/fit-point identity | Entity definition and semantic sub-reference registry | May map to several native points/variables |
| User parameters and expressions | Document or sketch parameter namespace | Evaluated target inputs; native variables remain private |
| Constraints and driving measures | Neutral definition | Translated equations with provenance |
| Accepted coordinates/coefficients | Neutral evaluated state | Backend candidate is validated before commit |
| Iterates, factorization, private helpers | Solver session | Disposable; no serialization obligation |
| External selectors and cached reference shape | Reference subsystem in neutral definition/evaluation | Prescribed values, no reverse write into source |
| Derived arrangement/profile snapshot | Geometry/profile service keyed by revision | Independent of solver backend and scratch lifetime |
| Undo commands and gesture history | Application/document | Uses neutral transaction state, not native memory snapshots |
| Selection/hover/annotation layout | Application/UI, with serializable presentation where needed | Never part of solver equations |

**SK-ARC-007 — Stable semantic subreferences [ADR candidate].** Public endpoint/control/fit/site identities should be distinct from private pole indices and role aliases such as Start/End. Reverse, split, knot and conversion operations should explicitly map persistent identities and changing aliases.

**SK-ARC-008 — Backend execution context [ADR candidate].** Adapters should declare session ownership, concurrency, global scratch state and resource lifecycle. Serialize native global-state use where required. Public Core interfaces should remain portable and should not require Qt, OS events, or renderer types.

### 26.4 Conditional migration if B/C is adopted

1. Preserve existing public IDs, value queries, common validation, and exact support preflight.
2. Establish neutral definition/accepted-state records and transaction semantics for current point/line/circle/arc types.
3. Convert existing adapters into sessions compiled from that model, preserving their incremental mappings where practical.
4. Replace broad backend CRUD/query responsibility with synchronization and candidate-result operations. Keep the public facade stable where semantics are unchanged.
5. Introduce geometry services and a small arrangement/profile implementation over current primitives.
6. Route tools through edit plans and gesture transactions; add richer references/dimensions and advanced geometry incrementally.

A separately approved implementation task should reconcile `sketch-architecture.md` with this target. This research task intentionally changes no APIs or existing architecture documentation.

### Architecture conclusions and limits

`Sketch`, `ISketchBackend`, `DcmBackend` and `SolveSpaceBackend` are not enough names to express every responsibility, but they may remain a small public facade if the internal boundaries are clear. Renaming an interface does not solve ownership. A solver boundary should accept a neutral solve model of geometry, parameters, semantic relations and references and publish validated realization/analysis, with no authority to trim, choose regions, heal gaps, rebind sources or rewrite dimensions.

The recommended decomposition need not create a library/interface/object per box. Start with explicit values, small services and session ownership. Avoid prematurely committing to a giant `ICurve`, mandatory asynchronous work, universal event bus, distributed registry or B-Rep kernel. Advanced geometry, fitting recipes and solving may cooperate in outer evaluation loops; those loops must declare who owns structural changes, convergence and accepted-state validation.

**SK-ARC-009 — Conditional authority [ADR candidate].** Record the adopted A/B/C policy and its state/lifetime table in an ADR before interface migration. The observable contracts for persistence, rollback, unsupported forms, references and profile consumers shall remain the evaluation criteria, regardless of selected private storage.

## 27. Responsibility boundaries between modules

**SK-BND-001 — Responsibility allocation [Boundary recommendation].** Use the following owner boundaries. Shared helpers are allowed; hidden side effects across boundaries are not.

| Capability | Owner | Explicitly outside that owner |
|---|---|---|
| Persistent entities, intent, IDs | Sketch model | Native solver handles, screen state |
| Curve representation and jets | Geometry representation | Constraint solving, selecting which fragment to delete |
| Semantic constraint validation | Constraint model/compiler | Guessing user intent from the cursor |
| Numerical equation solve, rank analysis | Solver adapter/backend | Profile creation, external source naming |
| Intersections, closest points, length | Computational geometry | Writing constraints or undo history |
| Split/trim/extend/fillet plan | Editing service | Direct renderer calls, undocumented constraint deletion |
| Incidence, half-edges, face walks | Planar topology | Changing authored geometry to repair a gap |
| Regions, holes, selected unions, profile snapshots | Profile service | Extrusion/B-Rep reconstruction |
| Expressions and unit validation | Parameter service | Label placement, solver-owned parameter numbering |
| Driving/driven measure semantics | Dimension model | Text formatting as an equation |
| External resolution/projection | Reference service plus document provider | Moving upstream geometry through the local solver |
| Snap candidates / geometric inference | Portable inference helpers | Mouse buttons or renderer types |
| Screen ranking, gestures, selection | Application interaction | Reimplementing curve mathematics |
| Diagnostic aggregation and repair plans | Sketch coordinator/domain services | Silent automatic fixes |
| Labels, colors, icons, previews | UI/render adapters | Deciding validity from visual appearance |
| Feature dependency scheduling and undo | Document/application | Embedding a specific solver into public feature APIs |
| Solid conversion and extrusion | Future Solid module | Re-solving sketch equations or rediscovering selected regions |

**SK-BND-002 — Structural changes [Boundary recommendation].** Ordinary numerical solving should not silently choose degree, knot structure, source identity, authored splits or profile material. A declared fit/recipe outer procedure may change realization structure under its own policy and migration/validation contract. Derived topology fragments are not authored splits.

**SK-BND-003 — Reuse without coupling [Boundary recommendation].** All consumers shall use the same source/provenance definitions. Rendering may tessellate curves; Solid may convert them; neither owns the meaning of a sketch reference or entity.

### What does not belong in ISketchBackend

Curve-curve intersections, closest-point search, offsets, restriction/extension, tolerant arrangements, profile extraction, shape tessellation, creation recipes, dimension labels, source rebinding, undo policy and cursor snapping are not intrinsically solver responsibilities. A library may happen to implement some of them; access should be through their semantic service boundary, not because every backend must emulate that library's entire modeling application.

Conversely, geometry services do not decide which constraints a trim should delete; the editor owns that policy. Topology does not heal authored gaps; it reports interpretation/uncertainty. The reference provider does not decide whether an ellipse-center dimension should become a spline control dimension after a type change. UI supplies intent and presentation, not a second geometric truth.

## Part C — Staging, acceptance, and research

## 28. Implementation staging

Staging is secondary to the target model. These slices are provisional and may be reordered by research, demand and evidence. Every advanced family remains in the capability inventory; “research” means significant uncertainty, not exclusion. No stage is sized to a bachelor thesis or current team.

| Stage | Relation to existing milestone names | Coherent delivery slice | What it establishes |
|---|---|---|---|
| Foundation | M0 | Portable definition/site IDs, local frame, roles, units, revisioned accepted/trial state, transactions, solver capability/result/diagnostic contract; small point/line/circle/arc corpus; geometry/topology/provider seams | Replaceable solver without encoding product scope; ownership ADR and basic conformance harness |
| Core mechanical | M1 | Analytic geometry/relations/dimensions; DAG parameters; stable gestures; transactional analytic edits; arrangement/cells/holes/unions; references/rebinding; persistence and inspectable inference/diagnostics | Practical coherent Sketcher and first Solid-consumer profile contract |
| Mature mechanical | M2 subset | Ellipse/conic semantics, richer creation recipes, blocks/patterns, weak completion, auto-dimension/recognition proposals, stronger diagnosis/relaxation, richer reference migration and management UX | Mature intent authoring and editing independent of freeform breadth |
| Advanced freeform | M2 subset and later | Bézier/B-spline/NURBS, fit/control lifecycle, rational/periodic forms, knot/degree/weight edits, G1/G2/selected G3/C controls, full advertised pair queries, exact restriction and general editing/profile coverage | Complete freeform family slices rather than isolated spline buttons |
| Industrial / research extensions | Future; some research begins earlier | Robust nonlinear/global/inequality analysis, high-order continuity, variable-knot/weight solving, advanced offsets/arrangements/repair, equation/coupled/area driving, solution exploration, scalable inference/reference persistence | Extended mathematical/interaction assurance and specialized authoring |

### Foundation / M0

Establish observable contracts with representative analytic entities; inventory both existing adapters and their exact semantics. Decide authority A/B/C before expanding interfaces. Exercise rollback, cancellation, unsupported-form preservation, parameter/site identity and consumer snapshots. Geometry query and profile boundaries can be tested with small analytic fixtures without selecting a full future kernel.

**SK-MIL-001 — M0 exit [Delivery gate].** A representative sketch can be created, constrained, solved, dragged, cancelled, serialized and read by neutral consumers, with exact unsupported-form reporting and private native handles. The authority/accepted-state model and capability/diagnostic contracts are documented; the limited implemented subset is explicit. This is a foundation gate, not the complete product taxonomy.

### Core mechanical / M1

The proposed initial certification slice remains deliberately coherent:

- Points, segments, circles and oriented circular arcs; datum origin/axes; construction/centerline roles.
- Coincidence, incidence with declared domains, H/V, parallel/perpendicular, collinear, analytic tangent/contact, endpoint G1, concentric, midpoint, equal length/radius, point/axis symmetry forms, fix/prescription.
- Basic distance/length/angle/radius/diameter dimensions, driving/driven modes, units, named parameters and DAG expressions.
- Physical DOF and qualified conflict/dependency analysis, constrained drag modes, branch safeguards, rollback/undo, diagnostics.
- Analytic pair intersection/overlap, split/trim/line-arc extend, path/analytic merge, line-corner fillet/chamfer, move/rotate/copy/mirror.
- Restricted offsets for lines/circles/arcs and declared simple chains/joins; refuse unresolved cleanup/collapse with precise reason. Independent outputs are sufficient initially.
- Nondestructive arrangements, selectable cells/holes/unions, open-wire diagnostics, profile snapshots and safe selection correspondence.
- Earlier-sketch/datum provider slice with live/frozen/detached state, broken-reference repair, and neutral future model-provider contract.
- Direct numeric entry, inspectable inference/snapping, filtering/selection, profile/status display and persistence.

**SK-MIL-002 — M1 exit [Delivery gate].** Section 29 gates pass for the declared reference implementation/configuration and benchmark policy. All mandatory M1 forms are supported end-to-end; other adapters may advertise smaller subsets honestly. A conformance-qualified product configuration is not a claim of formal proof for every numerical input.

First Solid consumers need the accepted profile contract and its supported analytic corpus; they do not define the upper bound of Sketch capabilities. Completion/weak dimensions, unbounded authored geometry and automatic full-constrain tools can enter later while their semantics remain prepared in the model.

### Mature mechanical / advanced freeform

Deliver complete family slices: geometry/authoring → relation/validation → editing/migration → references → topology/profile → UX/diagnostics → persistence/robustness. Exact degree/weight/periodicity limits must be explicit. Ellipse and conic editing need not wait for arbitrary rational-spline offset cleanup; blocks and intent proposals need not wait for G3. These are separate coherent expansions.

**SK-MIL-003 — Advanced exit [Delivery gate].** Each delivered M2 family has its advertised form/assurance envelope, ordinary and degenerate scenarios, site/identity migration, and full feature completion rubric. Unsupported rational, arbitrary-contact or high-order forms remain explicit; neither native solver enums nor a rendering demonstration establish support.

Research can start in M0 and inform later designs. Priorities are not a rigid sequential waterfall. Class-A surfaces, assemblies, CAM and exchange-format projects stay outside this specification even if they later consume Sketch products.

## 29. Concrete acceptance criteria for M1

These gates define a testable delivery target. They are not execution results. Tests should live near `src/core/tests/` with headless geometry/model coverage, plus application-level gesture/undo coverage. Backend tests validate advertised subsets; the conformance-qualified M1 configuration must pass all applicable gates.

### 29.1 Numerical and performance baseline

**SK-ACC-001 — Declared numerical benchmark policy [Benchmark-plan gate].** Before M1 sign-off, record hardware/OS/build/compiler, backend/version, units, admissible input scale/representation envelope and separate residual, event, closure and approximation policies. Choose/justify thresholds using the corpus and downstream requirements; publish observed limits. No numerical value in this document is an experimentally established universal Sketcher constant.

**SK-ACC-002 — Measured interaction budget [Benchmark-plan gate].** Define representative gesture/solve/profile corpora with coupled/disconnected and difficult cases. Measure latency distributions, cancellation responsiveness, failure rate and memory on the recorded configuration. Set and justify release-specific budgets before sign-off; progress/cancellation and stale-result protection remain mandatory when work exceeds the interactive budget.

### 29.2 Functional gates

| Acceptance ID | Setup / action | Required observable result | Requirements exercised |
|---|---|---|---|
| SK-ACC-010 Neutral persistence | Save a dimensioned sketch, reopen, switch to another supporting adapter | Stable IDs, expressions, units, roles, branch choices and geometry within policy; no native IDs in file | GEO-001/006, ROB-014, CAP-006 |
| SK-ACC-011 Unsupported switch | Include a constraint absent from target adapter, attempt switch | Definition untouched; exact unsupported form reported; no dropped constraints | PRN-005, CAP-001/006 |
| SK-ACC-012 Solver failure isolation | Add an impossible length to a fixed segment, then cancel | Conflict/nonconvergence qualified; last accepted geometry preserved; cancel restores complete prior state | SOL-003/006/008, DRG-001 |
| SK-ACC-013 DOF completeness | Free point, free segment, free circle in disconnected components | Generic total DOF 2+4+3=9 at regular nondegenerate state; correct free entities; fixed frame not counted | SOL-004 |
| SK-ACC-014 Fully defined rectangle | Coincident corners, H/V, anchored corner, width and height | Zero local DOF with satisfied residuals; ordinary drag does not move it; driven diagonal changes after width edit | CON, DIM-001, DRG-006 |
| SK-ACC-015 Redundancy versus conflict | Add duplicate length 10, then independently request length 11 on same segment | First diagnosed as dependent/satisfied; second as conflicting or qualified failed solve; no silent deletion | SOL-005/006, DIA-002 |
| SK-ACC-016 Parameter semantics | Define `width=2*height`, use mixed unit input, then introduce a cycle | Correct converted dimensions; display-unit change preserves shape; cycle rejected with dependency path | DIM-002/003/004 |
| SK-ACC-017 Reproducible drag | Drag an under-constrained linkage along recorded nonsingular path and reverse; repeat | Hard residuals pass, branch stable, no large unrequested jump, equivalent repeated results | SOL-007/012, DRG-004/009 |
| SK-ACC-018 Singular drag | Drive linkage toward a toggle/collapse, then beyond | No invalid geometry accepted or unexplained reflected branch; limited/ambiguous motion reported | DRG-005, ROB-010 |
| SK-ACC-019 Multi-selection/cancel | Move endpoints from a common original displacement, then cancel | No cumulative projection drift; atomic multiple-target solve; all original state restored | DRG-001/002 |
| SK-ACC-020 Analytic pair suite | Line-line, line-circle, circle-circle plus arc-domain variants | All transverse/tangent/endpoint/overlap events and parameters correct; no false empty success | CG-003/004/005, INT-001 |
| SK-ACC-021 Split line | Split length-100 line at 40; inspect dimensions and move children | Children reproduce locus; no duplicate 100 dimensions; seam/contact and chosen total-length intent explicit | CUR-005, EDT-010/030 |
| SK-ACC-022 Trim at several events | Cut line crossing two circles; choose one interior interval | Exact previewed interval removed; surviving endpoint references/remappable constraints preserved; new contacts explicit | EDT-011/031, INT-002 |
| SK-ACC-023 Circle/arc seams | Trim circle across parameter seam; reverse and mirror resulting arc | Correct retained sweep and endpoints, radius preserved, no full/zero-circle confusion | GEO-003, INT-007 |
| SK-ACC-024 Extend/join | Extend a line/arc to boundary, join collinear segments | Forward boundary chosen; preserved endpoint ID policy; conflicting length explained; exact support merge or wire distinction | EDT-013/014 |
| SK-ACC-025 Fillet/chamfer | Dimensioned line corner, valid then too-large radius/setback | Valid tangent/chamfer geometry with intended measures; old corner dimension policy visible; failed edit atomic | EDT-016/017/033 |
| SK-ACC-026 Restricted offset | Offset line, arc, circle and supported chain; attempt collapsing/unsupported result | Correct distance and side; correct joins; no scale-as-offset; unsupported cleanup refused with preview | CG-006, EDT-015 |
| SK-ACC-027 Move/rotate/copy/mirror | Operate on partly constrained set and copied parameterized set | Hard intent preserved or conflict reported; copied IDs unique; internal links and shared/value parameter policy correct | EDT-018/019/021/024 |
| SK-ACC-028 Rectangle plus divider | Closed rectangle crossed by eligible segment, then make segment construction | Two bounded cells, union equals rectangle; then one cell; source curves never physically split by topology | TOP-001/003/008, PRO-001 |
| SK-ACC-029 Nested loops | Three nested circles; select annulus, disk and combinations | Correct cell containment, holes, orientations and selected union boundaries | PRO-001/002/003 |
| SK-ACC-030 Crossing and touching | Bow-tie polyline; two tangent circles; dangling bridge | Cells distinguished from invalid whole wires; no fictitious zero-area region; contact/components reported and consumer-inadmissible union rejected | TOP-002/007, PRO-005 |
| SK-ACC-031 Gaps and duplicates | Near-gap, visible gap, duplicate and partially overlapping edges | Bounded tolerance policy; precise diagnostics; no silent healing; unaffected valid regions still usable | TOP-004/005/006, PRO-006, ROB-011 |
| SK-ACC-032 Consumer contract | Feed a selected region with hole into a mock Solid consumer | Ordered exact boundary intervals, plane and provenance available; consumer needs no sketch intersection search | PRO-003/004 |
| SK-ACC-033 Profile persistence | Edit dimension preserving region, then split/merge selected region | Unique continuation resolved; ambiguous change reported; no arbitrary region index substitution | PRO-007 |
| SK-ACC-034 Reference updates | Project earlier sketch edge; resize, delete, then rebind source | Associative update; broken state on deletion; explicit previewed replacement; local solver cannot move source | REF-001/004/005/007 |
| SK-ACC-035 Reference cycles | Two sketches reference each other or cyclic parameters | Cycle detected at document boundary before indefinite rebuild | REF-006, DIM-004 |
| SK-ACC-036 Inference meaning | Draw near midpoint/H/V/tangency; suppress/cycle/accept candidate | Snap-only and persistent states distinguishable; correct exact relation; no redundant/conflicting auto relation committed | INF-001–007 |
| SK-ACC-037 Selection fidelity | Several coincident endpoints, curve, dimension and shaded region overlap | Individual candidates selectable/cyclable; endpoint drag differs from curve translation and dimension-label movement | INF-008, DRG-003, DIM-007 |
| SK-ACC-038 Scale/ordering invariance | Translate/rotate/scale valid corpus within envelope; permute insertion order | Equivalent events/regions and residual quality; topology not dependent on display tessellation | ROB-002/012 |
| SK-ACC-039 Stale work | Change geometry while an earlier solve/profile job is pending | Old result never overwrites new state; pending jobs cancellable; current status honest | SOL-001, DRG-008, TOP-008 |
| SK-ACC-040 Diagnostic usability | Trigger conflict, unsupported constraint, broken reference, topology gap | Distinct diagnostic codes, affected IDs and useful repair choices; unavailable analysis not displayed as zero | DIA-001–007 |
| SK-ACC-041 Command round trip | For every M1 edit, commit -> undo -> redo -> save/load | Stable identity/provenance and equivalent state; no orphan constraints, dimensions or bindings | EDT-001–004, ROB-014 |
| SK-ACC-042 Constraint conformance | For every M1 relation, test each advertised reference form, both applicable branches, and an invalid pairing | Independent neutral residuals pass; domains and branch conditions hold; invalid forms rejected before allocation/mutation | CON-001/002/040, MAT-001/002, CAP-008 |

### 29.3 Feature completion rubric

**SK-ACC-050 — Definition of done [Contract].** Each delivered capability shall have: semantic definition; compatible reference forms; ownership boundary; runtime capability query; persistence behavior; ordinary/failure/degenerate cases; cancellation/undo policy where applicable; diagnostics; downstream impact; and focused conformance coverage. UI-only or happy-path-only support shall be labeled incomplete.

For M2, extend the same corpus with exact Bézier subdivision, rational-conic equivalence, knot insertion, periodic seams, spline-line/circle/spline tangencies and intersections, G2 regularity/degeneracy, freeform offsets, control-reference migration, and region changes under spline edits. These are advanced extensions to the same contracts, not a separate ad hoc Sketcher.

### Gate interpretation and benchmark plan

These are proposed acceptance scenarios, not a report of executed tests. “Equivalent,” “bounded” and “no large unrequested jump” require recorded thresholds appropriate to the fixture scale and selected policy. Establish those values experimentally; do not borrow one tolerance for all subsystems.

The numerical corpus should stratify geometry scale/translation, residual conditioning, near-contact separation, constraint coupling, event multiplicity, disconnected components and reference/profile updates. Record success/failure/unsupported/incomplete separately. Measure solve, accepted preview, topology refresh and commit independently as well as end-to-end; compare fresh and incremental paths.

A supported form is complete only when its semantics, capability answer, persistence, edit migration, diagnostic/failure, cancellation, geometry/query/topology and consumer behavior are demonstrated. Basic mathematical fixtures can have exact expected results; hard numerical cases need independently checked evidence and explicitly unresolved outcomes.

## 30. Open technical questions and decision risks

| Question / risk | Why it matters | Recommended next decision or experiment |
|---|---|---|
| Neutral model transition cost | Existing adapters are authoritative stores; replacing ownership changes failure and query semantics | Write an architecture decision record comparing neutral authority versus a separate neutral store adapter; prototype current primitives and cancellation first |
| M1 backend feature gap | Current public tangency forms are not supported by either adapter as a complete M1 contract; OurPaintDCM analysis is limited | Inventory exact native/library support, choose exact lowerings and adapter work; designate a conformance-qualified configuration rather than promise backend parity |
| Geometry library strategy | Analytic queries are small; reliable rational-spline arrangements and offsets are much larger | Benchmark a bounded corpus with existing math facilities and candidate kernels; assess footprint, licensing, portability and maintenance before dependency approval |
| Tolerance model | Solve residuals, clustering, approximate intersections, and future B-Rep tolerances must fit one budget | Adopt explicit units and a normal-size envelope; stress translation/scale invariance; publish unresolved cases |
| Native arc representation | Current center/start/end trial state can violate circularity; direction conventions vary | Choose canonical public orientation and validity rules; adapters own intrinsic equations and conversion |
| Finite point-on/tangency domains | Many solvers naturally constrain full supports; interval bounds are not mere equalities | Decide on witness constraints/active-set handling; reject unsupported finite forms rather than misuse support tangency |
| Split intent policy | Independent children cannot automatically retain every original control/length relation | Define independent and parent-preserving modes; implement analytic transfer table before freeform editing |
| Associative offsets | Exact offset evaluation, topology cleanup, and backend solving are separate challenges | Begin with restricted analytic independent output; prototype a derived recipe with error accounting in M2 |
| Freeform certification | Numerical root finding can miss tangent/overlap events; exact arithmetic does not solve all approximate-data issues | Establish degree/weight/scale envelope and event-completeness strategy before claiming spline–spline profile support |
| Profile identity | Face splitting/merging invalidates index-based downstream selections | Use provenance plus selection policy; fail ambiguous M1 rebinding; test fixtures with crossing/tangent transitions |
| Conflict explanation cost | Irreducible/minimum nonlinear conflict sets can require many solves | Budget analysis; label confidence; compare backend analysis with trial-deletion verification |
| Performance targets | Interactive behavior depends on hardware, components, solver and geometry distribution | Pin a reference machine/configuration and scripted gesture corpus; measure p50/p95, not a marketing maximum entity count |
| Cross-platform/WASM | Solver global state, dependencies and thread assumptions may constrain portability | Audit adapter concurrency and numerical dependencies without building a Web UI now |
| Source/version confidence | Rolling help evolves; CATIA mirror release unknown; native libraries expose less than full products | Verify uncertain pairs against licensed/current docs or reproducible product sessions before implementation-specific promises |
| License and dependency obligations | Solver/kernel distribution and code reuse have project-wide consequences | Review exact pinned licenses and repository dependency process when selecting implementations; this document imports no code or library |

**SK-RSK-001 — Evidence ledger [Contract].** Track a capability as proposed, documented upstream, implemented in adapter, or verified in OurPaint. Keep release/version, source, and conformance fixture references where available.

**SK-RSK-002 — Research restraint [Contract].** Do not treat vendor claims of natural dragging, robustness, AI intent, or “all solutions” as formal numerical guarantees. Evaluate behavior against OurPaint's own explicit acceptance corpus.

## 31. Requirement continuity and change ledger

Revision 0.2 preserves all revision 0.1 requirement and acceptance identifiers. Stage annotations were removed from Part A contracts and moved to inventory/staging. Existing IDs keep their semantic topic; the substantive refinements below are explicit. Unlisted IDs retain their obligation with editorial/numbering or delivery-metadata changes only. Architecture choices and principles are reclassified rather than treated as intrinsic Sketcher truths.

| Existing ID | Change in revision 0.2 |
|---|---|
| SK-ACC-001 | Replaced arbitrary numeric acceptance envelope with an evidence-based policy-definition gate. |
| SK-ACC-002 | Removed arbitrary entity/equation/latency numbers; retained measured release budget obligation. |
| SK-ACC-030 | Aligned point-touch acceptance with consumer policy. |
| SK-ARC-001 | Reclassified as architecture recommendation; clarified ownership/session or implementation-choice scope. |
| SK-ARC-002 | Reclassified as architecture recommendation; clarified ownership/session or implementation-choice scope. |
| SK-ARC-003 | Reclassified as architecture recommendation; clarified ownership/session or implementation-choice scope. |
| SK-ARC-004 | Reclassified as architecture recommendation; clarified ownership/session or implementation-choice scope. |
| SK-ARC-005 | Reclassified as architecture recommendation; clarified ownership/session or implementation-choice scope. |
| SK-ARC-006 | Reclassified as architecture recommendation; clarified ownership/session or implementation-choice scope. |
| SK-ARC-007 | Reclassified as architecture recommendation; clarified ownership/session or implementation-choice scope. |
| SK-ARC-008 | Reclassified as architecture recommendation; clarified ownership/session or implementation-choice scope. |
| SK-BND-001 | Reclassified allocation table as an architecture recommendation; retained independent semantic ownership. |
| SK-BND-002 | Allowed explicit outer authoring procedures while preventing hidden structural solver side effects. |
| SK-CAP-007 | Removed M1/default staging from semantic capability model. |
| SK-CG-003 | Separated tangential alignment from crossing/touching and other event facets. |
| SK-CG-005 | Scoped completeness and added unresolved-domain result rather than an unqualified universal promise. |
| SK-CON-013 | Expanded mathematical meaning and qualifications; removed delivery-stage restriction. |
| SK-CON-015 | Expanded mathematical meaning and qualifications; removed delivery-stage restriction. |
| SK-CON-019 | Expanded mathematical meaning and qualifications; removed delivery-stage restriction. |
| SK-CON-021 | Expanded mathematical meaning and qualifications; removed delivery-stage restriction. |
| SK-CON-022 | Expanded mathematical meaning and qualifications; removed delivery-stage restriction. |
| SK-CON-025 | Expanded mathematical meaning and qualifications; removed delivery-stage restriction. |
| SK-CON-026 | Expanded mathematical meaning and qualifications; removed delivery-stage restriction. |
| SK-CON-028 | Expanded mathematical meaning and qualifications; removed delivery-stage restriction. |
| SK-CON-031 | Removed Future exclusion and expanded named measures; separated measurement from driving. |
| SK-CUR-004 | Separated orthogonal completion/accuracy/multiplicity/status fields. |
| SK-DIM-003 | Removed small-library limitation and clarified extensible unit/domain and nonsmooth semantics. |
| SK-DIM-004 | Separated DAG expression errors from legitimate explicit coupled equations; removed Future restriction. |
| SK-DRG-004 | Made locality observable through fixtures; removed implied universal minimum-motion algorithm guarantee. |
| SK-DRG-009 | Qualified path-dependent underconstrained return while preserving regression and cancellation obligations. |
| SK-EDT-020 | Allowed negative uniform scaling with explicit orientation semantics; singular transforms remain qualified. |
| SK-GEO-001 | Reclassified neutral storage as an ADR candidate; retained portable public semantics. |
| SK-GEO-002 | Distinguished preserved invalid definitions from admissible evaluated geometry. |
| SK-GEO-003 | Corrected reversal/reflection orientation wording and full-traversal/opening distinction. |
| SK-GEO-007 | Relaxed blanket fixed-knot rule; explicit variable policies allowed while discrete edits remain controlled. |
| SK-GEO-008 | Allowed legitimate declared fitting realization; retained intent protection. |
| SK-GEO-010 | Separated representation, provenance and accuracy; clarified algebraic exactness. |
| SK-INF-001 | Moved candidate stages to roadmap and added facet/regularity meaning. |
| SK-INT-003 | Corrected tangent-equals-touch assumption with a regular tangential-crossing example. |
| SK-INT-007 | Distinguished periodic artificial seams from authored closed-curve endpoints and one-cut opening. |
| SK-INT-008 | Scoped event completeness and kept algorithm choice open. |
| SK-MIL-001 | Retained foundation exit subject; ownership choice is conditional rather than silently fixed to neutral authority. |
| SK-MIL-002 | Retained M1 exit; removed implication that certification means formal numerical proof. |
| SK-MIL-003 | Mapped old M2 advanced exit to mature-mechanical/freeform family slices; target taxonomy remains independent. |
| SK-PRN-001 | Reclassified as a product principle; delivery metadata moved to staging. |
| SK-PRN-002 | Reclassified as a product principle; delivery metadata moved to staging. |
| SK-PRN-003 | Reclassified as a product principle; delivery metadata moved to staging. |
| SK-PRN-004 | Reclassified as a product principle; delivery metadata moved to staging. |
| SK-PRN-005 | Reclassified as a product principle; delivery metadata moved to staging. |
| SK-PRN-006 | Reclassified as a long-term principle; removed implementation-size framing. |
| SK-PRN-007 | Reclassified as a product principle; delivery metadata moved to staging. |
| SK-PRO-003 | Clarified exact restrictions separately from numerical closure and approximation. |
| SK-PRO-005 | Replaced blanket point-touch/overlap rejection and unqualified certification with consumer policy. |
| SK-PRO-007 | Removed M1 policy from the long-term correspondence contract. |
| SK-REF-002 | Removed staging from provider kinds and clarified ownership. |
| SK-REF-006 | Separated ordinary dependency-cycle errors from explicit advanced coupling. |
| SK-REF-009 | Retained advanced apparent contours in taxonomy with tracking qualifications. |
| SK-SOL-004 | Added physical/gauge, first-order/finite and active-bound qualifications; separated completion stabilization. |
| SK-SOL-005 | Distinguished structural, local and partial dependency from globally redundant intent. |
| SK-SOL-012 | Scoped reproducibility to implementation/configuration and tie-breaking rather than universal equality. |
| SK-TOP-004 | Replaced implied exact coincidence with residual/closure evidence. |
| SK-TOP-006 | Removed blanket early-stage overlap rejection from long-term semantics. |
| SK-TOP-010 | Allowed proven incremental identity instead of requiring every derived ID to be revision-local. |

New families: `SK-CRT-*` creation semantics; `SK-SPL-*` freeform lifecycle; `SK-AUT-*` intent proposals/configuration; `SK-UX-*` mature interaction. New IDs within existing prefixes add conic/domain/rational semantics, support equivalence/activation/admissibility, advanced dimension/solver/query/edit migration, topology/contact/profile assurance, provider guarantees, diagnostic evidence, error budgets and end-to-end capabilities. No old ID has been repurposed for an unrelated new topic.

The M0/M1/M2 names remain usable. M2 is now navigated as mature-mechanical and advanced-freeform slices; Future denotes industrial/research staging, not taxonomic exclusion. `SK-ACC-001/002` retain their baseline/budget subjects but are now benchmark-plan gates instead of fixed arbitrary numerical targets.

## 32. Sources and reference bibliography

Sources were consulted on 2026-10-03. Links attached to claims identify the evidence actually used. The register is intentionally selective; vendor text has been summarized, not reproduced. Rolling pages may change. The specification's APIs, ownership model, budgets, and milestone choices are original proposals.

| Key | Primary evidence and scope |
|---|---|
| NX1 | [Siemens, Control sketch behavior][NX1], July 2026: found/persistent relations and configuration controls |
| NX2 | [Siemens, NX June 2024 Core Design][NX2]: Navigator categories and conflict visibility |
| NX3 | [Siemens, NX Sketch June 2023 tips][NX3]: relation relaxation and ignoring |
| NX4 | [Siemens, Sketch and Profile Creation][NX4], July 2025: dynamic chain creation, locks and infinite lines |
| NX5 | [Siemens, Sketch and Voice Command Assistant][NX5]: Draw Shape spline/trim workflows; do not generalize to all sketch curve operations |
| SW1 | [SOLIDWORKS 2025, Description of Sketch Relations][SW1]: compatible selections, support semantics and curvature forms |
| SW2 | [SOLIDWORKS 2025, Troubleshooting Resources][SW2]: SketchXpert and Repair Sketch roles |
| SW3 | [SOLIDWORKS 2025, FAQ Sketch Relations][SW3]: inference controls, redundant dimensions and dangling-reference repair |
| SW4 | [SOLIDWORKS 2025, Trimming with Power Trim][SW4]: gesture and trail behavior |
| SW5 | [SOLIDWORKS 2025, Trim Entities][SW5]: trim modes and construction-preservation policies |
| FU1 | [Autodesk Fusion, Constraints in sketches][FU1]: relation family including curvature |
| FU2 | [Autodesk Fusion, Sketches][FU2]: open/closed profiles and geometry roles |
| FU3 | [Autodesk Fusion, Project geometry onto a sketch plane][FU3]: associative links and lost-projection repair |
| IN1 | [Inventor 2026, Apply Geometric Constraints][IN1]: ellipse axes, spline endpoint restrictions, Smooth G2 |
| IN2 | [Inventor 2026, Split, Trim, or Extend Curves][IN2]: operation-specific relation/dimension preservation and interaction |
| IN3 | [Inventor 2026, Projecting Sketch Geometry][IN3]: documented association exceptions; version-qualified evidence |
| IN4 | [Inventor LT 2015, Relax Mode settings][IN4]: historical explicit relax policy and equation preservation |
| IN5 | [Inventor 2026, Create and Edit Splines][IN5]: fit/control representations and conversion caveats; length procedure includes 3D context, not used to claim general 2D length support |
| CR1 | [Creo r13, Available Constraints][CR1]: common relations and equal curvature |
| CR2 | [Creo r12, Dimensioning][CR2]: weak/strong dimensions, scaling and conflict response |
| CR3 | [Creo r12, Using Constraints][CR3]: offered-constraint controls and spline contact prerequisite |
| CR4 | [Creo r12, Resolve a Conflict][CR4]: undo/delete/reference conversion and explanation |
| CR5 | [Creo r13, Conic Arc Dimensions][CR5]: rho and endpoint/tangency parameterization |
| CA1 | [Dassault-hosted R2019x SmartPick feature explanation][CA1]: creation versus preview and background detection scope |
| CA2 | [CATIA/3DEXPERIENCE, Analyzing Sketched Geometries][CA2]: vendor-authored help on third-party mirror; unspecified release, reduced version confidence |
| CA3 | [CATIA/3DEXPERIENCE, Modifying Constraints][CA3]: same mirror; reference dimension and formula-driven interaction distinctions |
| CA4 | [CATIA/3DEXPERIENCE, Over-Constrained or Inconsistent Sketches][CA4]: same mirror; differentiated geometry/constraint statuses |
| CA5 | [CATIA/3DEXPERIENCE, Connecting Curves][CA5]: same mirror; associative connections and continuity choices |
| CA6 | [CATIA/3DEXPERIENCE, Editing a Spline][CA6]: same mirror; indexed vendor-help text was accessible, direct fetch was unavailable; lower access/version confidence |
| ON1 | [Onshape, Use][ON1]: associative projection, type-change and silhouette restrictions |
| ON2 | [Onshape, Bézier][ON2]: control editing and trim/split behavior |
| ON3 | [Onshape, Working with Constraints][ON3]: status display and Constraint manager |
| ON4 | [Onshape, Dimensions][ON4]: driving/driven semantics |
| FC1 | [FreeCAD, SketchObject API documentation][FC1]: diagnostics, reference rebuilding, structural edits and branch-locking API; generated docs may lag current source |
| FC2 | [FreeCAD developer news, Tangency at B-spline knots][FC2], December 2022 |
| FC3 | [FreeCAD developer news, Point-on-Spline][FC3], January 2023 |
| SS1 | [SolveSpace reference manual][SS1]: primitive/spline workflow, split constraint loss and DOF visualization |
| SS2 | [SolveSpace technology][SS2]: equation solving, under-constrained motion and initial-state dependence |
| SS3 | [SolveSpace library interface][SS3]: embedding scope; pinned repository headers remain authority for current integration |
| DC1 | [Siemens D-Cubed 2D DCM][DC1]: industrial constraint/spline/diagnostic scope, distinct from OurPaintDCM |
| DC2 | [Siemens D-Cubed 2D DCM v71][DC2]: G3 and scalable-set examples; advanced reference only |
| DC3 | [Siemens D-Cubed components technical sheet][DC3]: separate Profile Geometry Manager and constraint manager roles |
| G1 | [OCCT Geom2dAPI_InterCurveCurve][G1]: isolated and segment intersection results |
| G2 | [OCCT Geom2d_BSplineCurve][G2]: rationality, periodicity, knot editing and restriction |
| G3 | [OCCT Geom2d_OffsetCurve documentation][G3]: offset evaluator and self-intersection caveat |
| G4 | [CGAL 2D Arrangements user manual][G4]: geometric traits, overlaps and planar arrangement structure |
| G5 | [Jonathan Shewchuk, Robust Predicates][G5]: adaptive precision for geometric predicates |

[NX1]: https://blogs.sw.siemens.com/designcenter/how-to-control-sketch-behavior/
[NX2]: https://blogs.sw.siemens.com/designcenter/whats-new-in-nx-june-2024-core-design/
[NX3]: https://blogs.sw.siemens.com/designcenter/sketch-summer-2023-nx-tips-and-tricks/
[NX4]: https://blogs.sw.siemens.com/designcenter/sketch-and-profile-creation/
[NX5]: https://blogs.sw.siemens.com/designcenter/whats-new-in-nx-design-sketch-and-voice-command-assistant/
[SW1]: https://help.solidworks.com/2025/english/solidworks/sldworks/c_Description_of_Sketch_Relations.htm
[SW2]: https://help.solidworks.com/2025/English/SolidWorks/sldworks/c_SOLIDWORKS_Troubleshooting_Resources.htm
[SW3]: https://help.solidworks.com/2025/english/Solidworks/sldworks/c_FAQ_Sketch_Relations.htm
[SW4]: https://help.solidworks.com/2025/english/SolidWorks/sldworks/t_Trimming_with_Power_Trim.htm
[SW5]: https://help.solidworks.com/2025/english/SolidWorks/sldworks/c_Trim_Entities.htm
[FU1]: https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/SKT-CONSTRAINTS.htm
[FU2]: https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/SKT-3D-SKETCH.htm
[FU3]: https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/GUID-850061C4-71F5-4418-AF9C-F0D232022F9C.htm
[IN1]: https://help.autodesk.com/cloudhelp/2026/ENU/Inventor-Help/files/GUID-A85A7A30-7D81-4C75-8769-CAD034EEA930.htm
[IN2]: https://help.autodesk.com/cloudhelp/2026/ENU/Inventor-Help/files/GUID-DD00037F-6BBA-4B95-AA90-4F99DCEB667D.htm
[IN3]: https://help.autodesk.com/cloudhelp/2026/ENU/Inventor-Help/files/GUID-B61431DA-FABC-41EE-945C-6E54B4806582.htm
[IN4]: https://help.autodesk.com/cloudhelp/2015/ENU/InventorLT-Help/files/GUID-9BEDBEDE-044B-4B4A-88B1-7EBB3380A490.htm
[IN5]: https://help.autodesk.com/cloudhelp/2026/ENU/Inventor-Help/files/GUID-DB15C479-66F2-456A-9E54-40A60F83DC92.htm
[CR1]: https://support.ptc.com/help/creo/creo_pma/r13/usascii/part_modeling/sketcher/Available_Constraints.html
[CR2]: https://support.ptc.com/help/creo/creo_pma/r12/usascii/part_modeling/sketcher/About_Dimensioning.html
[CR3]: https://support.ptc.com/help/creo/creo_pma/r12/usascii/part_modeling/sketcher/About_Using_Constraints.html
[CR4]: https://support.ptc.com/help/creo/creo_pma/r12/usascii/part_modeling/sketcher/To_Resolve_a_Conflict.html
[CR5]: https://support.ptc.com/help/creo/creo_pma/r13/usascii/part_modeling/sketcher/About_Conic_Arc_Dimensions.html
[CA1]: https://3dswym.3dexperience.3ds.com/post/catia-user-community/3dexperience-r2019x-newfunction-sketcher-automatically-create-constraints-when-snapping-during-drag-or-snapping-on-external-edges_j8ffHhZjQW6qSmjXyOh67Q
[CA2]: https://help-3dexperience.aesvietnam.com/English/DysUserMap/dys-c-SketchAnalyze-SketchedGeometryAnalyze.htm
[CA3]: https://help-3dexperience.aesvietnam.com/English/DysUserMap/dys-t-ConstraintSet-ConstraintModify.htm
[CA4]: https://help-3dexperience.aesvietnam.com/English/DysUserMap/dys-m-OverConstrainedSketchAnalyze-sb.htm
[CA5]: https://help-3dexperience.aesvietnam.com/English/DysUserMap/dys-t-SimpleProfileSketch-CurveConnect.htm
[CA6]: https://help-3dexperience.aesvietnam.com/English/DysUserMap/dys-t-SketchEdit-SplineEdit.htm
[ON1]: https://cad.onshape.com/help/Content/Sketch/use.htm
[ON2]: https://cad.onshape.com/help/Content/Sketch/bezier.htm
[ON3]: https://cad.onshape.com/help/Content/Sketch/working_with_constraints.htm
[ON4]: https://cad.onshape.com/help/Content/Sketch/dimension.htm
[FC1]: https://freecad.github.io/SourceDoc/d9/dad/classSketcher_1_1SketchObject.html
[FC2]: https://blog.freecad.org/2022/12/28/tangent-constraint-at-b-spline-knots/
[FC3]: https://blog.freecad.org/2023/01/23/new-constraint-point-on-spline/
[SS1]: https://solvespace.com/ref.pl
[SS2]: https://solvespace.com/tech.pl
[SS3]: https://solvespace.com/library.pl
[DC1]: https://www.siemens.com/en-us/products/plm-components/d-cubed/2d-dcm/
[DC2]: https://blogs.sw.siemens.com/plm-components/d-cubed-2d-dcm-version-71-0/
[DC3]: https://www.plm.automation.siemens.com/media/global/en/Siemens-PLM-D-Cubed-components-fs-4834-A3_tcm27-60564.pdf
[G1]: https://dev.opencascade.org/doc/refman/html/class_geom2d_a_p_i___inter_curve_curve.html
[G2]: https://dev.opencascade.org/doc/refman/html/class_geom2d___b_spline_curve.html
[G3]: https://dev.opencascade.org/doc/occt-7.1.0/refman/html/_geom2d___offset_curve_8hxx.html
[G4]: https://doc.cgal.org/latest/Arrangement_on_surface_2/index.html
[G5]: https://www.cs.cmu.edu/~quake/robust.html


### Additional evidence used in revision 0.2

| Key | Evidence and scope |
|---|---|
| SW6 | [SOLIDWORKS 2025, Rectangle PropertyManager][SW6]: rectangle/parallelogram placement variants and numeric entry |
| SW7 | [SOLIDWORKS 2025, Equation Driven Curves][SW7]: explicit and parametric equation forms; conflicting global-variable help claims are not adopted |
| SW8 | [SOLIDWORKS 2025, Style Spline PropertyManager][SW8]: Bézier/B-spline form choices and rational weight controls; specific degree limits remain product-specific |
| SW9 | [SOLIDWORKS 2025, Applying Torsion Continuity Relations][SW9]: documented 2D endpoint curvature/rate matching; vendor command name is not adopted as planar torsion semantics |
| SW10 | [SOLIDWORKS 2025, Fully Define Sketch PropertyManager][SW10]: selected scope, relation choices and baseline/ordinate/chain dimensions |
| SW11 | [SOLIDWORKS 2025, Sketch Status Conventions][SW11]: under/overdefined, no-solution and invalid geometry distinctions; proposed validity layers do not copy its entire classification |
| FU4 | [Fusion, Create Slots][FU4]: slot recipes, construction aids and numeric workflows |
| FU5 | [Fusion, Create Conic Curves][FU5]: endpoint/rho workflows producing elliptical, parabolic or hyperbolic conics |
| FU6 | [Fusion, Create and Edit Splines][FU6]: fit/control editing, curvature handles/combs and degree-change consequences |
| FU7 | [Fusion, AutoConstrain workflow][FU7]: multiple proposals, datum, relation/dimension control and optional geometry modification |
| IN6 | [Inventor 2026, Sketch Blocks][IN6]: definition/instance association and flexible nested forms |
| ON5 | [Onshape, Spline][ON5]: fit-site workflows, handles and curvature visualization |
| CR6 | [Creo r12, Sketcher Constraints][CR6]: legacy weak geometric constraints distinguished from current weak dimensions |
| FC4 | [FreeCAD, current SketchObject header][FC4]: active/driving modes, diagnostic categories and repair APIs; main branch is not a pinned release |
| FC5 | [FreeCAD, current Constraint header][FC5]: weight, conic landmarks and representation-site forms; source definitions are not universal UI guarantees |
| G6 | [CGAL, 2D Regularized Boolean Set-Operations][G6]: regularized area sets, contact, disconnected components and holes |
| A1 | [MIT Shape Interrogation, B-spline Algorithms][A1]: shape-preserving insertion/subdivision and continuity/removal distinctions |
| A2 | [MIT Shape Interrogation, NURBS][A2]: rational Bézier specialization and exact conics |
| A3 | [Piegl and Tiller, The NURBS Book, 2nd ed. (1997)][A3]: publisher/contents verified; reference bibliography, not a claim of full-text review |
| A4 | [Patrikalakis, Maekawa and Cho, Shape Interrogation Hyperbook (December 2009)][A4]: differential geometry, numerical roots, intersections, distance and offsets |
| A5 | [Barsky and DeRose, UCB/CSD-88-417 (1988)][A5]: parameter-independent geometric continuity; archival abstract has a terminology typo, so neutral equations are separately stated |
| A6 | [Hoffmann and Joan-Arinyo, A Brief on Constraint Solving (2005)][A6]: author publication record and solver-survey reference |
| A7 | [Hoffmann, Lomonosov and Sitharam, Decomposition Plans, Part I (2001)][A7]: decomposition quality/design intent/performance framework; historical complexity claims not adopted universally |
| A8 | [Zou et al., A Review on Geometric Constraint Solving (2022 preprint)][A8]: structural/nonstructural dependency and research map; not an authority for blanket algorithm guarantees |
| A9 | [Zou and Feng, witness-method limitations (2019 preprint)][A9]: limitations of witness analysis for solver tasks |
| A10 | [MIT Shape Interrogation, planar curve intersection][A10]: repeated/tangent roots and floating-point perturbation |
| A11 | [MIT Shape Interrogation, curve/curve methods][A11]: tangency and termination/completeness difficulties; method choice remains open |
| A12 | [Farouki and Neff, Analytic Properties of Plane Offset Curves (1990)][A12]: offset singularities/self-intersections and extraneous-loop trimming |
| A13 | [Hoffmann and Joan-Arinyo, Parametric Modeling (2002)][A13]: parametric intent, representations and dependency context |

### Small academic/reference reading path

| Topic | Reference / role in this specification |
|---|---|
| Constraint solving and parametric intent | Hoffmann/Joan-Arinyo survey and parametric-modeling chapter: decompositions, representations and design intent. [Survey][A6], [parametric modeling][A13] |
| Rigidity/DOF and decomposition | Hoffmann/Lomonosov/Sitharam: decomposition quality and design intent; follow rigidity literature for precise local/generic/global distinctions. [Decomposition plans][A7] |
| Modern diagnosis limitations | Zou et al. review and Zou/Feng witness critique: why graph counting or one rank test cannot solve every dependency/conflict task. [Review][A8], [witness limitations][A9] |
| Spline representation/lifecycle | Piegl/Tiller reference book; MIT accessible knot/NURBS chapters: representation, fitting, exact conics and shape modification. [Book][A3], [algorithms][A1], [NURBS][A2] |
| Geometric continuity | Barsky/DeRose: parameter-independent G semantics versus C parameter derivatives. [Technical report][A5] |
| Roots and curve intersections | MIT Shape Interrogation: differential regularity, distance functions, repeated roots and intersection methods. [Hyperbook][A4], [tangent-root example][A10], [methods][A11] |
| Offsets | Farouki/Neff: singularities, self-intersections and trimming extraneous loops. [Publication][A12] |
| Planar arrangements and area sets | CGAL manuals: embedding/DCEL, geometry traits, face boundary components and regularized operations. [Arrangements][G4], [set operations][G6] |
| Robust predicate arithmetic | Shewchuk: adaptive signs for represented-input predicates, distinct from numerical construction and tolerant intent. [Reference][G5] |

This bibliography supports conceptual distinctions and research questions. It is not a literature review, implementation selection, or claim that every listed full text was read. Publisher metadata, author preprints and accessible technical chapters have differing evidence scopes, recorded above.


[SW6]: https://help.solidworks.com/2025/english/SolidWorks/sldworks/HIDD_DVE_SKETCH_RECTANGLES.htm
[SW7]: https://help.solidworks.com/2025/english/SolidWorks/sldworks/c_equation_driven_curves.htm
[SW8]: https://help.solidworks.com/2025/english/solidworks/sldworks/r_style_spline_pm.htm
[SW9]: https://help.solidworks.com/2025/english/SolidWorks/sldworks/t_applying_torsion_continuity_relations.htm?id=28.19.2.15
[SW10]: https://help.solidworks.com/2025/english/solidworks/sldworks/HIDD_FULLY_DEFINE_SKETCH.htm
[SW11]: https://help.solidworks.com/2025/english/solidworks/sldworks/c_Sketch_Status_Conventions.htm
[FU4]: https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/SKT-CREATE-SLOTS.htm
[FU5]: https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/SKT-SKETCH-CREATE-CONIC-CURVES.htm
[FU6]: https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/SKT-CREATE-SPLINES.htm
[FU7]: https://help.autodesk.com/cloudhelp/ENU/Fusion-Sketch/files/SKT-AUTO-CONSTRAIN.htm
[IN6]: https://help.autodesk.com/cloudhelp/2026/ENU/Inventor-Help/files/GUID-D4403D63-95CE-4392-A73E-DBB1B8C8C110.htm
[ON5]: https://cad.onshape.com/help/Content/Sketch/spline.htm
[CR6]: https://support.ptc.com/help/creo/creo_pma/r12/usascii/part_modeling/sketcher/About_Sketcher_Constraints.html
[FC4]: https://github.com/FreeCAD/FreeCAD/blob/main/src/Mod/Sketcher/App/SketchObject.h
[FC5]: https://github.com/FreeCAD/FreeCAD/blob/main/src/Mod/Sketcher/App/Constraint.h
[G6]: https://doc.cgal.org/latest/Boolean_set_operations_2/index.html
[A1]: https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node18.html
[A2]: https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node20.html
[A3]: https://link.springer.com/book/10.1007/978-3-642-59223-2
[A4]: https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/mathe.html
[A5]: https://www2.eecs.berkeley.edu/Pubs/TechRpts/1988/5276.html
[A6]: https://www.cs.purdue.edu/cgvlab/www/publications/hoffmann2005brief/
[A7]: https://www.cise.ufl.edu/~sitharam/pdfs/drone-final.pdf
[A8]: https://arxiv.org/abs/2202.13795
[A9]: https://arxiv.org/abs/1904.00526
[A10]: https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node80.html
[A11]: https://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node82.html
[A12]: https://research.ibm.com/publications/analytic-properties-of-plane-offset-curves
[A13]: https://www.cs.purdue.edu/cgvlab/www/resources/papers/Hoffmann-CAGD-2002-Parametric_Modeling.pdf

## 33. Technical research frontiers

These are substantial domains in their own right, suitable for sustained engineering, graduate research or dissertations. An unresolved method does not remove its product capability. Product policy, mathematical guarantees and implementation evidence must remain separate.

| Frontier | Why difficult / unresolved obligation | Owner | Nature | Later study / decision |
|---|---|---|---|---|
| Robust nonlinear solving | Coupled equations, degeneracy, scaling, local minima and disconnected solution sets; no one numerical method guarantees every sketch | Solver/coordinator | Numerical mathematics, engineering, research | Decomposition/variational methods, continuation, interval/global methods; declared success envelope. [solver survey][A6], [decomposition plans][A7], [recent review][A8] |
| Branch-preserving dragging | Closest feasible motion is path/policy-dependent; singularities can destroy continuation or require choice | Interactive solve/session + UX | Product design, numerical research | Continuation, trust regions, constrained optimization, branch witnesses; gesture corpus and limits |
| Reliable DOF/rigidity/redundancy | Physical gauges, intrinsic constraints, nonstructural dependencies and singularity; local rank differs from finite/global motion | Solver analysis | Rigidity theory, numerical mathematics | Generic/local/global rigidity, witness methods and limitations; truthful evidence vocabulary. [decomposition plans][A7], [witness limitations][A9] |
| Conflict explanation | Failed optimization is not infeasibility; irreducible/minimum sets can be expensive and order-dependent | Diagnostics + solver | Algorithms, product design, research | Deletion verification, decomposition, conflict certificates where possible, bounded repair search |
| Multiple-solution exploration | Branch count may be large or not enumerable for general procedural/coupled forms | Solver/exploration | Algebraic/numerical research, UX | Restricted families, completeness claims, branch-preserving preview and commit |
| Spline-spline intersection | Repeated roots/tangencies, overlaps, close branches and termination/completeness under finite precision | Computational geometry | Numerical mathematics, computational geometry | Interval/subdivision, clipping, implicit/algebraic, filtered/exact predicates; no method preselected. [tangent roots][A10], [intersection methods][A11] |
| General curve offsets | Nonrational locus, cusps, loops, collapse and distance-boundary cleanup; exactness and topology differ | Geometry construction + topology/editor | Computational geometry, research | Normal evaluators, approximation bounds, arrangement cleanup, special exact families. [offset analysis][A12] |
| Tolerant arrangements | Event isolation/order and endpoint closure uncertainty can contradict embedding; noisy intended equality differs from exact arithmetic | Geometry/topology | Computational geometry, numerical engineering | Robust predicates plus certified/bounded construction, cluster extent, conservative unresolved cases. [arrangements][G4], [robust predicates][G5] |
| Profile/reference persistence | Faces and sources split/merge/type-change; geometric similarity does not prove same semantic identity | Document provider + profile selection | Product design, algorithms, research | Provenance, interval maps, witnesses, candidate correspondence and explicit ambiguity; B-Rep naming interface only |
| Semantic migration through edits | Equivalent current shape may lose future response; pole/fit/whole-curve references lack universal substitution | Editor/model | Product semantics, engineering, research | Migration algebra, equivalence verification, recipe overrides, one/many lineage and constraint intent |
| High-order continuity | Parameter gauges, derivative conditioning, rational weights and insufficient freedoms; G/C goals differ | Freeform authoring + solver | Differential geometry, numerical research | Jets, arc-length curvature derivatives, local reparameterization and degeneracy handling. [geometric continuity][A5] |
| Variable-knot/weight fitting | Nonlinear knot ordering/weight gauges, interpolation conditioning and structure changes | Fitting/authoring coordinator | Approximation theory, research | Stable fitting objectives, admissibility, local/global policies, exact versus approximating edits. [B-spline algorithms][A1], [NURBS reference][A3] |
| Length/area/minimum-distance driving | Integration and witness/topology switches create nonsmooth equations; measurements are easier than driving | Measures + solver/recipe + topology | Numerical mathematics, research | Sensitivity, integral error, active-set/witness branch and topology transition policies |
| Scalable inference/recognition | Dense candidates and many plausible design intents; ranking needs predictable control rather than hidden rewriting | Inference/proposal system + UX | Product design, algorithms, optional learning | Spatial indexes, latency/candidate budgets, deterministic cycling, proposal validation and intent displacement |
| Automated full-constrain/repair | Many valid parameterizations; zero DOF is not evidence of intended design or good future edits | Intent proposals/diagnostics | Product design, optimization, research | Datum/allowed-family choices, alternative previews, replaceable completion, explainability |
| Scalable hybrid sessions | Incremental cache validity, concurrency, cancellation, diagnostics and accepted snapshots must agree | Coordinator/adapters | Systems engineering | Benchmark B versus C; dirty dependency updates, lifecycle tests and versioned publication |
| Assurance/performance tradeoff | Fast local preview, complete geometry search and deep diagnosis have different costs | All services/coordinator | Product policy, engineering | Tiered quality requests, measured budgets, final validation and honest incomplete states |

The next research action should define a bounded problem, observable success/failure criteria and independent evidence before selecting an algorithm. A dissertation-level result may improve one frontier without replacing the whole Sketcher architecture. The capability model supplies the interfaces and semantics against which that result can be evaluated.
