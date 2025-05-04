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


include_directories(../headers)
include_directories(../Math/headers)

include_directories(../headers/objects)

enable_testing()
set(SOURCES
        ../src/objects/GeometricObjects.cc
        ../src/objects/BoundBox.cc
        ../src/Scene.cc
#        ../src/Requirements.cc
#        ../Math/src/ErrorFunctions.cc
#        ../Math/src/Function.cc
#        ../src/Paint.cc
#        ../src/BMPfile.cc
#        ../src/painters/BMPpainter.cc
#        ../src/Requirements.cc
#        ../src/saveload/objectInFile.cc
#        ../src/saveload/FileOurP.cc
#        ../src/saveload/requirementInFile.cc
)

#Test for ID
add_executable(IDGTEST IDGTEST.cc ${SOURCES} RequirementsGTEST.cc)
target_link_libraries(IDGTEST GTest::gtest_main)

#Test for objects
add_executable(ObjectsGTEST ObjectsGTEST.cc ${SOURCES})
target_link_libraries(ObjectsGTEST GTest::gtest_main)

#Test for VarsStorage
add_executable(VarsStorageGTEST VarsStorageGTEST.cc ${SOURCES})
target_link_libraries(VarsStorageGTEST GTest::gtest_main)

#Test for Requirement
add_executable(RequirementsGTEST RequirementsGTEST.cc ${SOURCES})
target_link_libraries(RequirementsGTEST GTest::gtest_main)

#Test for Paint
add_executable(SceneGTEST SceneGTEST.cc
        ../src/Scene.cc
        ../src/objects/GeometricObjects.cc
        ../src/objects/BoundBox.cc
        ../src/painters/BMPpainter.cc
        ../src/BMPfile.cc
)
target_link_libraries(SceneGTEST GTest::gtest_main)

#Test for UndoRedo
add_executable(UndoRedoGTEST UndoRedoTests.cc ../src/UndoRedo/Transaction.cc ../src/UndoRedo/UndoRedo.cc)
target_link_libraries(UndoRedoGTEST GTest::gtest_main)

#Test for UndoRedoTemplate
add_executable(UndoRedoTemplateGTEST UndoRedoGTEST.cc ../src/UndoRedo/UndoRedoTemplate.cc)
target_link_libraries(UndoRedoTemplateGTEST GTest::gtest_main)