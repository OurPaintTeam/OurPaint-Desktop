add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/gl.c)

target_include_directories(glad PUBLIC glad/include)

# Zlib
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.3.2
)
FetchContent_MakeAvailable(zlib)


# FreeType
include(FetchContent)
FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
        GIT_TAG        VER-2-14-3
)
set(FT_DISABLE_ZLIB OFF)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_HARFBUZZ ON)
set(FT_DISABLE_BROTLI ON)
FetchContent_MakeAvailable(freetype)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)
FetchContent_MakeAvailable(glm)


add_library(ourpaint_rendering STATIC
        OpenGLRenderer.cc
        Camera2D.cc
        Logger.cc
        #text/FreeTypeFontManager.cc
)
target_include_directories(ourpaint_rendering PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        ${CMAKE_SOURCE_DIR}/src/Rendering/renderer
)
target_link_libraries(ourpaint_rendering PUBLIC
        glad
        freetype
        glm::glm
        ${OPENGL_LIB}
)

