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

enable_testing()
set(SOURCES  ../painter/objects.cpp  ../painter/saveload/objectInFile.cpp ../painter/saveload/FileOurP.cpp)
add_executable(
  savergtest
  savergtests.cpp
  ${SOURCES}
)
add_executable(
        ourpainttest
        test.cpp
        ${SOURCES}
)
add_executable(
        matrixgtest
        TestsMatrix.cpp
        ${SOURCES}
        TestsMatrix.cpp)
add_executable(
        addReqInComponentGtest
        addReqInComponentTests.cpp
        ${SOURCES}
        addReqInComponentTests.cpp)
target_link_libraries(
  ourpainttest
  GTest::gtest_main
)
target_link_libraries(
        savergtest
        GTest::gtest_main
)
target_link_libraries(
        addReqInComponentGtest
        GTest::gtest_main
)
target_link_libraries(
        ourpainttest
        GTest::gtest_main
)
target_link_libraries(
        matrixgtest
        GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(ourpainttest)
gtest_discover_tests(matrixgtest)
