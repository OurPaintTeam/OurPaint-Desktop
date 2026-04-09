add_library(glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/gl.c)

target_include_directories(glad PUBLIC glad/include)

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
        ${OPENGL_LIB}
)

