cmake_minimum_required(VERSION 3.26)
project(ourpainttest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
add_ourpaint_test(UndoRedoGTEST UndoRedoTests.cc)
add_ourpaint_test(ObjectInJsonTest ObjectInJsonGTEST.cc)
add_ourpaint_test(ComponentTest ComponentTest.cc)
add_ourpaint_test(RequirementJsonTest requirementInJsonGTEST.cc)
add_ourpaint_test(MoveTest MoveGTEST.cc)

add_ourpaint_test(GeometryAlgorithmTest GeometryAlgorithmsGTEST.cc)
target_link_libraries(GeometryAlgorithmTest PRIVATE OurPaint::Sketch)

add_ourpaint_test(SketchGTEST SketchGTEST.cc)
target_link_libraries(SketchGTEST PRIVATE OurPaint::Sketch)

# Build explicitly to verify the public SolveSpace C API include and link boundary.
add_executable(SolveSpaceLinkSmoke EXCLUDE_FROM_ALL SolveSpaceLinkSmoke.cpp)
target_link_libraries(SolveSpaceLinkSmoke PRIVATE OurPaint::SolveSpace)



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


