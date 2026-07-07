cmake_minimum_required(VERSION 3.20)
project(ourpaint)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_LEAVE_TEST_IN_ALL OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
        Eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0
)
FetchContent_MakeAvailable(Eigen)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

find_package(OpenMP REQUIRED)

add_subdirectory(src/app)
add_subdirectory(src/core)
add_subdirectory(src/core/tests)
add_subdirectory(src/rendering/renderer)
add_subdirectory(src/UI)



if(MSVC)
    add_compile_options(
        /Zc:__cplusplus     # Правильный __cplusplus
        /Zc:preprocessor    # Современный препроцессор (для C++20)
        /permissive-        # Строгий режим стандарта
        /std:c++17
    )
endif()
