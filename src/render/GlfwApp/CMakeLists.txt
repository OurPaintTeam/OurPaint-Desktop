cmake_minimum_required(VERSION 3.20)
project(MVK_OpenGL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mvk_openGL
        main.cpp
        OpenGLApp.cc
        GlfwRuntimePlatform.cc
        GlfwViewportHost.cc
)
target_include_directories(mvk_openGL PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include
)
target_link_libraries(mvk_openGL PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/glfw/libglfw3dll.a
        ourpaint_rendering
)

add_custom_command(TARGET mvk_openGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/glfw/glfw3.dll
            $<TARGET_FILE_DIR:mvk_openGL>
)



add_library(ourpaint_glfw
        GlfwRuntimePlatform.cc
        GlfwViewportHost.cc
)
target_include_directories(ourpaint_glfw PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include
)
target_link_libraries(ourpaint_glfw PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/glfw/libglfw3dll.a
        ourpaint_rendering
)

add_custom_command(TARGET ourpaint_glfw POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/glfw/glfw3.dll
            $<TARGET_FILE_DIR:ourpaint>
)
