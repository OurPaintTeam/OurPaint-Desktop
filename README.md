# OurPaint Desktop

OurPaint Desktop is a C++ and Qt 6 desktop application for working with
two-dimensional geometry. It supports creating and editing geometric
primitives and defining constraints between objects.

The active development branch is `dev`.


## Technology Stack

- C++20
- CMake
- Ninja
- Qt 6
- OpenGL 3.3
- Eigen
- nlohmann/json
- GoogleTest
- FreeType
- GLM

The `third_party/` directory contains the UI and DCM Git submodules, GLAD,
and bundled fonts. DCM contains its own Math submodule. Other C++ libraries
are downloaded into the build tree by CMake's FetchContent. Platform-specific
build tools and Qt must be installed separately.

## Clone the Repository

```bash
git clone https://github.com/OurPaintTeam/OurPaint-Desktop.git
cd OurPaint-Desktop
git switch dev
```

### Initialize the Git submodules:

```bash
git submodule update --init --recursive
```

## Building on Linux

### Ubuntu and Debian

Install the required packages:

```bash
sudo apt update
sudo apt install -y \
    git \
    cmake \
    ninja-build \
    build-essential \
    pkg-config \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-svg-dev \
    libqt6opengl6-dev \
    libgl1-mesa-dev \
    libxkbcommon-dev \
    libxcb-cursor0
```

Configure the project:
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```
Build the application:
```bash
cmake --build build --target ourpaint --parallel
```
Run:
```bash
cd build/src/app
./ourpaint
```

## Building on Windows
The supported Windows toolchain is MSYS2 UCRT64 with GCC/MinGW-w64.

Do not mix this toolchain with Qt or MinGW libraries from another
installation.

### 1. Install MSYS2

Install MSYS2 and open the MSYS2 UCRT64 terminal.

Do not use the plain MSYS, MINGW64, or CLANG64 terminal for this build.

### 2. Update MSYS2

Run:
```bash
pacman -Syu
```

If MSYS2 asks you to close the terminal, close it, reopen the
MSYS2 UCRT64 terminal, and run the command again:

```bash
pacman -Syu
```

### 3. Install the Build Tools

Run the following command inside the MSYS2 UCRT64 terminal:

```bash
pacman -S --needed \
    git \
    mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-qt6-base \
    mingw-w64-ucrt-x86_64-qt6-svg \
    mingw-w64-ucrt-x86_64-qt6-tools
```

### 4. Configure the Project

From the repository root, run:
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### 5. Build the Application

```bash
cmake --build build --target ourpaint --parallel
```

### 6. Run the Application

Run the application from the MSYS2 UCRT64 terminal:

```bash
./build/src/app/ourpaint.exe
```

The exact executable path may differ if the output directory is changed in
CMake.

## Static Release Build on Windows

This PowerShell workflow uses Qt 6.10.0 sources and CLion's MinGW toolchain.
Adjust the paths for your installation. Build Qt and OurPaint with the same
compiler; do not mix this environment with MSYS2 libraries. Python 3 must
also be available in `PATH`.

### 1. Build and Install Static Qt

Install the Qt sources (for example, through Qt Maintenance Tool), then run:

```powershell
$clionBin = "C:\Program Files\JetBrains\CLion 2025.3.3\bin"
$env:PATH = "$clionBin\mingw\bin;$clionBin\cmake\win\x64\bin;$clionBin\ninja\win\x64;$env:PATH"

New-Item -ItemType Directory -Force C:\Qt\build-6.10.0-static
Set-Location C:\Qt\build-6.10.0-static

& C:\Qt\6.10.0\Src\configure.bat `
    -prefix C:\Qt\6.10.0\mingw_64_static `
    -release -static -static-runtime `
    -submodules qtbase,qtsvg `
    -nomake examples -nomake tests

cmake --build . --parallel 4
cmake --install .
```

`-static` builds static Qt libraries; `-static-runtime` enables static compiler
runtime linking. `qtsvg` provides support for the application's SVG icons.
Setting `BUILD_SHARED_LIBS=OFF` in OurPaint alone does not make an existing
shared Qt installation static.

### 2. Build OurPaint

Return to the repository root in the same PowerShell session. Use a new build
directory to avoid cached paths to shared Qt:

```powershell
cmake -S . -B build-release-static -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_SHARED_LIBS=OFF `
    -DCMAKE_PREFIX_PATH=C:/Qt/6.10.0/mingw_64_static `
    -DQt6_DIR=C:/Qt/6.10.0/mingw_64_static/lib/cmake/Qt6 `
    "-DCMAKE_EXE_LINKER_FLAGS=-static"

cmake --build build-release-static --target ourpaint --parallel 4
```

Configuration should print `Qt6::Core type: STATIC_LIBRARY`. The executable
is `build-release-static/src/app/ourpaint.exe`. Keep the generated `shaders`
and `fonts` folders beside it; these resources are not embedded in the EXE.

Inspect DLL dependencies without launching the application:

```powershell
objdump -p .\build-release-static\src\app\ourpaint.exe |
    Select-String "DLL Name"
```

The list should contain no `Qt6*.dll` dependencies. Windows system DLLs
remain normal dependencies.

