cmake_minimum_required(VERSION 3.26)
project(ourpaint)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

include(cmake/Dependencies.cmake)
include(cmake/OurPaintTests.cmake)

add_subdirectory(src/app/editor)
add_subdirectory(src/app/ourpaint_qt)
add_subdirectory(src/app/ourpaint_imgui)
add_subdirectory(src/app/project)

add_subdirectory(src/core)
add_subdirectory(src/core/tinynurbs)

add_subdirectory(src/render)

add_subdirectory(third_party)

add_subdirectory(tests)


