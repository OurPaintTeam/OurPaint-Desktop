cmake_minimum_required(VERSION 3.26)
project(ourpainttest)

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

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

enable_testing()
include(GoogleTest)

function(add_ourpaint_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name}
        PRIVATE
            GTest::gtest_main
    )
    gtest_discover_tests(${name})
endfunction()


# Tests
add_ourpaint_test(ObjectsGTEST ObjectsGTEST.cc)
add_ourpaint_test(SceneGTEST SceneGTEST.cc)
add_ourpaint_test(UndoRedoGTEST UndoRedoTests.cc)
add_ourpaint_test(ObjectInJsonTest ObjectInJsonGTEST.cc)
add_ourpaint_test(ComponentTest ComponentTest.cc)
add_ourpaint_test(RequirementJsonTest requirementInJsonGTEST.cc)
add_ourpaint_test(MoveTest MoveGTEST.cc)



set(TYPE_OF_COMPILING BOOL 1)
if (TYPE_OF_COMPILING)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        add_compile_options(/W4)
    endif ()
endif ()

set(TYPE_OF_OPTIMIZATION BOOL 1)
if (TYPE_OF_OPTIMIZATION)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    endif ()
endif ()


