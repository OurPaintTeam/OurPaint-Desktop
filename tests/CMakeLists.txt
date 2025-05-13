cmake_minimum_required(VERSION 3.20)
project(ourpainttest)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
if (NOT TARGET gtest)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG 03597a01ee50ed33e9dfd640b249b4be3799d395
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Math ${CMAKE_BINARY_DIR}/Math)

#Test for objects
add_executable(ObjectsGTEST  ObjectsGTEST.cc
        ../src/objects/GeometricObjects.cc
        ../src/objects/BoundBox.cc
        ../Math/src/Function.cc
        ../Math/src/ErrorFunctions.cc
)
target_include_directories(ObjectsGTEST PRIVATE ../headers/objects ../Math/headers)
target_link_libraries(ObjectsGTEST GTest::gtest_main Math Eigen3::Eigen)

#Test for Scene
add_executable(SceneGTEST SceneGTEST.cc
        ../src/Scene.cc
        ../src/objects/GeometricObjects.cc
        ../src/objects/BoundBox.cc
        ../src/painters/BMPpainter.cc
        ../src/BMPfile.cc
        ../src/Requirements.cc
        ../src/saveload/FileOurP.cc
        ../src/saveload/objectInFile.cc
        ../src/saveload/requirementInFile.cc

        ../Math/src/ErrorFunctions.cc
        ../Math/src/Function.cc
        ../Math/src/LevenbergMarquardtSolver.cc
)
target_include_directories(SceneGTEST PRIVATE
        ../headers
        ../headers/objects
        ../headers/painters
        ../headers/saveload

        ../Math/headers/decomposition
        ../Math/headers
        ../Math/headers/graph
        ../Math/headers/optimizers
)
target_link_libraries(SceneGTEST GTest::gtest_main Math Eigen3::Eigen)

#Test for UndoRedo
add_executable(UndoRedoGTEST UndoRedoTests.cc  ../src/UndoRedo/Transaction.cc  ../src/UndoRedo/UndoRedo.cc)
target_include_directories(UndoRedoGTEST PRIVATE ../headers/UndoRedo)
target_link_libraries(UndoRedoGTEST GTest::gtest_main)

#Test for UndoRedoTemplate
#add_executable(UndoRedoTemplateGTEST UndoRedoGTEST.cc ../src/UndoRedo/UndoRedoTemplate.cc)
#target_include_directories(UndoRedoTemplateGTEST PRIVATE ../headers/UndoRedo)
#target_link_libraries(UndoRedoTemplateGTEST GTest::gtest_main)

#Test for move functionality
add_executable(MoveGTEST MoveGTEST.cc
        ../src/Scene.cc
        ../src/objects/GeometricObjects.cc
        ../src/objects/BoundBox.cc
        ../src/painters/BMPpainter.cc
        ../src/BMPfile.cc
        ../src/Requirements.cc
        ../src/saveload/FileOurP.cc
        ../src/saveload/objectInFile.cc
        ../src/saveload/requirementInFile.cc
        ../Math/src/ErrorFunctions.cc
        ../Math/src/Function.cc
        ../Math/src/LevenbergMarquardtSolver.cc
)
target_include_directories(MoveGTEST PRIVATE
        ../headers
        ../headers/objects
        ../headers/painters
        ../headers/saveload
        ../Math/headers/decomposition
        ../Math/headers
        ../Math/headers/graph
        ../Math/headers/optimizers
)

target_link_libraries(MoveGTEST GTest::gtest_main Math Eigen3::Eigen)

set(TYPE_OF_COMPILING BOOL 1)
if (TYPE_OF_COMPILING)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-Wall -Wextra -Wpedantic)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_compile_options(/W4)
    endif ()
endif ()
