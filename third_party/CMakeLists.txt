add_library(glad STATIC glad/src/gl.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

add_subdirectory(DCM)
add_subdirectory(UI)
add_subdirectory(glfw)

add_library(imgui
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp

        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
        imgui/
        imgui/backends
)

target_link_libraries(imgui
        #glad
        glfw
)

add_library(imgui_demo
        imgui/imgui_demo.cpp
)
target_link_libraries(imgui_demo PUBLIC
        imgui
)

add_subdirectory(nanosvg)
target_include_directories(nanosvg INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/nanosvg/src>"
)
target_compile_definitions(nanosvg PRIVATE NANOSVG_ALL_COLOR_KEYWORDS)
