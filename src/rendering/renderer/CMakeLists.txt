add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/gl.c)

target_include_directories(glad PUBLIC glad/include)

set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_BROTLI ON)
add_subdirectory(freetype-2.14.3)

add_library(ourpaint_rendering STATIC
        OpenGLRenderer.cc
        Camera2D.cc
        Logger.cc
)
target_include_directories(ourpaint_rendering PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
)
target_link_libraries(ourpaint_rendering PUBLIC
        glad
        freetype
        ${OPENGL_LIB}
)

