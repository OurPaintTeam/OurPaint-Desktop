# OurPaint agent instructions

## Project

OurPaint is a compact parametric CAD application written in C++20. It builds on work from OurPaintCAD, OurPaintDCM, and OurPaintMath while allowing their architecture to evolve. Favor a small, understandable codebase and dependencies whose value justifies their cost. The intended platforms are Windows, Linux, and, later, Web.

## Repository map

- `src/app/` contains the current Qt desktop application, editor tools, and viewport logic. The application layer coordinates documents, commands, tools, undo/redo, UI, Core, and rendering; keep computational geometry and graphics API details out of it.
- `src/core/` contains platform-independent CAD logic, including geometry, documents, serialization, and related state. Core tests and fixtures live in `src/core/tests/`.
- `src/render/render/` contains the current OpenGL renderer and shaders. Keep render representations and renderer backends separate from Core and UI so another graphics backend can be added later.
- `third_party/` contains the UI and DCM Git submodules, GLAD, and bundled fonts. Check submodule boundaries before editing their code or changing their interfaces.
- Root and component `CMakeLists.txt` files define build targets. Consult `CODING_STANDARD.md` and `.clang-format` for repository conventions.

These paths describe the current repository, not a requirement to preserve every existing dependency or module boundary forever. Verify the relevant files and call sites before making architectural changes.

## Architecture

- Keep Core independent of Qt, OpenGL, operating system APIs, and windowing code. Design its document model, sketcher, solver boundary, geometry, and serialization so the same computational model can eventually serve desktop and WebAssembly builds.
- Keep UI interaction, application orchestration, CAD computation, and rendering in their respective layers. Avoid leaking UI types, graphics API types, or solver-specific IDs into public Core or Sketch APIs.
- Keep render scene data independent of a specific graphics API. OpenGL is the current backend; WebGPU is a possible future backend, not a reason to implement it in unrelated work.
- Before changing interfaces, ownership, or Sketch/Solver boundaries, inspect existing abstractions and all relevant call sites. Consult `docs/en/sketch-architecture.md` if it exists. Make ownership and lifetime clear; update affected callers and documentation when an interface changes.
- Treat Web/WASM support as an architectural constraint, not a request to add Web code to every task. Avoid desktop-only dependencies in portable Core code.

## Working rules

- Make small, focused diffs that solve the requested problem. Do not perform unrelated refactors or expand the feature set without a task-specific reason.
- Follow nearby patterns before introducing a new abstraction. Prefer the simplest design that preserves module boundaries and interactive performance.
- Do not add a third-party dependency for a small convenience. Check existing facilities first and follow the dependency review process in `CODING_STANDARD.md` before proposing a new library.
- Use C++20, RAII, explicit ownership, and value semantics where appropriate. Avoid raw owning pointers and macros when a language feature suffices.
- Follow `CODING_STANDARD.md`, `.clang-format`, and the naming conventions of surrounding code. The current style uses four-space indentation and a 160-column limit. Use UpperCamelCase for new `.h` and `.cpp` files where that matches the component; preserve nearby `.cc` conventions. Write code comments in English. Format only touched code.
- For changes to math, constraints, geometry, topology, serialization, or feature rebuild behavior, add or update focused GoogleTest coverage where feasible. Put Core tests near related cases in `src/core/tests/` and register new test executables in its `CMakeLists.txt` when needed.

## Validation and execution limits

- **Do not run the OurPaint application unless the user explicitly asks.** This includes launching the desktop UI for manual inspection.
- **Do not run tests unless the user explicitly asks.** Writing or updating tests does not authorize executing them.
- **Do not perform a full build or run CI unless the user explicitly asks.** Do not automatically run `cmake --build build --parallel`, a full CTest suite, or equivalent repository-wide checks.
- Prefer code inspection and narrowly scoped static checks for routine verification. If compilation is needed and the user has not requested a full build, limit it to the affected target with the existing CMake setup; do not use that as a reason to launch the application or tests.
- Report what was checked and what was not run. Never claim build, test, CI, or runtime success without actually obtaining that result.

## Completion and handoff

Explain the behavior or design changed, the files affected, and any remaining uncertainty. If execution was not requested, state that the application, tests, and full build/CI were not run. For pull requests, include the relevant validation results and screenshots only when available; do not create a screenshot by launching the application without an explicit request.
