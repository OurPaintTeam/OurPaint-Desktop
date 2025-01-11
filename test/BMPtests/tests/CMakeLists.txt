cmake_minimum_required(VERSION 3.14)
project(tests)
# Проверка защиты ветки
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        eff443c6ef5eb6ab598bfaae27f9427fdb4f6af7
  # URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  tests
  test1.cpp
)
target_link_libraries(
  tests
  bitmap
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)

include_directories(../code)

