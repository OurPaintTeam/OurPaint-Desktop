cmake_minimum_required(VERSION 3.26)
project(OurPaintDCM VERSION 0.01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(OurPaintDCM
        headers/figures/Arc.h
        headers/figures/Circle.h
        headers/figures/GeometryStorage.h
        headers/figures/Line.h
        headers/figures/Point2D.h
        headers/figures/PointBase.h
        headers/functions/RequirementFunction.h
        headers/functions/RequirementFunctionFactory.h
        headers/requirements/Components.h
        headers/requirements/Requirements.h
        headers/system/RequirementFunctionSystem.h
        headers/system/RequirementSystem.h
        headers/utils/Enums.h
        headers/utils/FigureDescriptor.h
        headers/utils/ID.h
        headers/utils/IDGenerator.h
        headers/utils/RequirementDescriptor.h
        headers/DCMManager.h

        src/figures/GeometryStorage.cpp
        src/functions/RequirementFunction.cpp
        src/functions/RequirementFunctionFactory.cpp
        src/requirements/Requirements.cpp
        src/system/RequirementFunctionSystem.cpp
        src/system/RequirementSystem.cpp
        src/DCMManager.cpp
)

target_include_directories(OurPaintDCM PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/headers
  ${CMAKE_CURRENT_SOURCE_DIR}/headers/figures
  ${CMAKE_CURRENT_SOURCE_DIR}/headers/functions
  ${CMAKE_CURRENT_SOURCE_DIR}/headers/requirements
  ${CMAKE_CURRENT_SOURCE_DIR}/headers/system
  ${CMAKE_CURRENT_SOURCE_DIR}/headers/utils
)

target_link_libraries(OurPaintDCM PUBLIC Math Eigen3::Eigen)


add_subdirectory(app)

enable_testing()
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_subdirectory(tests)
