cmake_minimum_required(VERSION 3.20)
project(ourpaint)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

add_subdirectory(third_party)
add_subdirectory(src/app)
add_subdirectory(src/core)
include(cmake/Dependencies.cmake)
add_subdirectory(src/core/tests)
add_subdirectory(src/app/tests)
add_subdirectory(src/render/render)
