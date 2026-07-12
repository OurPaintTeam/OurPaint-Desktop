set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(ourpaint_UI
        QtApp.cpp
)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets OpenGLWidgets OpenGL)
target_link_libraries(ourpaint_UI
        PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::OpenGLWidgets
        Qt6::OpenGL
        Qt6::Concurrent
)
target_include_directories(ourpaint_UI
        PUBLIC
        ${Qt6Core_INCLUDE_DIRS}
        ${Qt6Widgets_INCLUDE_DIRS}
        ${Qt6OpenGLWidgets_INCLUDE_DIRS}
        ${Qt6OpenGL_INCLUDE_DIRS}
        ${Qt6Concurrent_INCLUDE_DIRS}
)