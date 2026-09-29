# SolveSpace solver dependency

`third_party/solvespace` is a Git submodule of the [official SolveSpace repository](https://github.com/solvespace/solvespace).
Its revision is pinned by the ParamsCAD Git tree. The source and GPLv3-or-later license
are in `solvespace/`, including `solvespace/COPYING.txt`; no SolveSpace source is copied into OurPaint.

`ParamsCAD::SolveSpace` builds the public `slvs.h` API wrapper and the solver-only sources.
It statically links SolveSpace's pinned mimalloc submodule. Its Eigen headers come from
ParamsCAD's existing `Eigen3::Eigen` dependency in OurPaintDCM, which is pinned to the
same Eigen 3.4.0 commit as SolveSpace's own Eigen submodule. The SolveSpace GUI, CLI,
rendering, and exporters are not built.
Only `slvs.h` is exposed to consumers of the target; the internal headers and Eigen and
mimalloc include directories remain private.

After checking out ParamsCAD, initialize the submodule and its required mimalloc submodule:

```sh
git submodule update --init third_party/solvespace
git -C third_party/solvespace submodule update --init extlib/mimalloc
```

Targets that use the C API should link `ParamsCAD::SolveSpace` and include `<slvs.h>`.
The target propagates SolveSpace's `STATIC_LIB` definition because `slvs.h` uses it to
disable DLL imports for static links on Windows.
Because this is a static link to GPLv3-or-later code, review the resulting distribution's
license obligations before shipping it. This integration does not change ParamsCAD's license.

The source list in `cmake/SolveSpace.cmake` follows SolveSpace's `slvs-solver` target at the
pinned revision. Recheck that list and the Eigen and mimalloc versions when updating the
submodule. Build `SolveSpaceLinkSmoke` explicitly to check the public header and link boundary.
