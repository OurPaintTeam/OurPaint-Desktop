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

Most third-party C++ libraries are downloaded and built automatically by
CMake. Platform-specific build tools and Qt must be installed separately.

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

