cmake_minimum_required(VERSION 3.14)
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
enable_testing()
set(SOURCES ../src/ID.cc ../src/Objects.cc)
#Test for ID
add_executable(IDGTEST IDGTEST.cc ${SOURCES})
target_link_libraries(IDGTEST GTest::gtest_main)
#Test for objects
add_executable(ObjectsGTEST ObjectsGTEST.cc ${SOURCES})
target_link_libraries(ObjectsGTEST GTest::gtest_main)

