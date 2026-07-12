cmake_minimum_required(VERSION 3.20)
project(ourpaint)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

include(FetchContent)

add_subdirectory(src/app)
add_subdirectory(src/core)
add_subdirectory(src/core/tests)
add_subdirectory(src/Rendering/renderer)
add_subdirectory(src/UI)
