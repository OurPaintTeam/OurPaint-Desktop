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
