add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/gl.c)
target_include_directories(glad PUBLIC glad/include)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# --------------------------------------------------------------------
# zlib
# --------------------------------------------------------------------

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.3.2
)

set(ZLIB_BUILD_SHARED OFF CACHE BOOL "Build shared zlib" FORCE)
set(ZLIB_BUILD_STATIC ON  CACHE BOOL "Build static zlib" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "Build zlib tests" FORCE)
set(ZLIB_INSTALL OFF CACHE BOOL "Install zlib" FORCE)

FetchContent_MakeAvailable(zlib)

if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

set(ZLIB_USE_STATIC_LIBS ON CACHE BOOL "Use static zlib" FORCE)
set(ZLIB_LIBRARY zlibstatic CACHE STRING "zlib target" FORCE)
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR}" CACHE PATH "zlib include directory" FORCE)

# --------------------------------------------------------------------
# FreeType
# --------------------------------------------------------------------

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG        VER-2-14-3
)

set(FT_DISABLE_ZLIB OFF CACHE BOOL "Enable ZLIB" FORCE)
set(FT_REQUIRE_ZLIB ON CACHE BOOL "Require ZLIB" FORCE)

set(FT_DISABLE_BZIP2 ON CACHE BOOL "Disable BZIP2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "Disable PNG" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable HarfBuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable Brotli" FORCE)
set(FREETYPE_NO_DISTUTILS ON CACHE BOOL "No distutils" FORCE)

set(SKIP_INSTALL_ALL ON)
FetchContent_MakeAvailable(freetype)
unset(SKIP_INSTALL_ALL)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)
FetchContent_MakeAvailable(glm)

add_library(ourpaint_render STATIC
    OpenGL2dRenderer.cc
    Camera2D.cc
    Logger.cc
    #text/FreeTypeFontManager.cc
)

target_include_directories(ourpaint_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    ${CMAKE_SOURCE_DIR}/src/Rendering/renderer
)

target_link_libraries(ourpaint_render PUBLIC
    glad
    freetype
    glm::glm
    ${OPENGL_LIB}
)