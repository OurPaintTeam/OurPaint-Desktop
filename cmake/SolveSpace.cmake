# Build only SolveSpace's public libslvs wrapper and its solver implementation.
# Do not include SolveSpace's top-level CMake project: it changes global project settings.
function(ourpaint_add_solvespace)
    set(solvespace_root "${PROJECT_SOURCE_DIR}/third_party/solvespace")

    if(NOT EXISTS "${solvespace_root}/include/slvs.h")
        message(FATAL_ERROR "SolveSpace is missing. Initialize third_party/solvespace before configuring.")
    endif()
    if(NOT TARGET Eigen3::Eigen)
        message(FATAL_ERROR "SolveSpace requires the Eigen3::Eigen target from OurPaintDCM.")
    endif()
    if(NOT EXISTS "${solvespace_root}/extlib/mimalloc/CMakeLists.txt")
        message(FATAL_ERROR
            "SolveSpace's mimalloc submodule is missing. Run: "
            "git -C third_party/solvespace submodule update --init extlib/mimalloc")
    endif()

    # SolveSpace uses mimalloc for its temporary solver arena. Keep its options
    # local to this function and its CMake project in a separate binary directory.
    set(MI_OVERRIDE OFF)
    set(MI_BUILD_SHARED OFF)
    set(MI_BUILD_STATIC ON)
    set(MI_BUILD_OBJECT OFF)
    set(MI_BUILD_TESTS OFF)
    add_subdirectory("${solvespace_root}/extlib/mimalloc"
                     "${CMAKE_CURRENT_BINARY_DIR}/ourpaint_mimalloc" EXCLUDE_FROM_ALL)

    add_library(ourpaint_solvespace STATIC EXCLUDE_FROM_ALL
        "${solvespace_root}/src/constrainteq.cpp"
        "${solvespace_root}/src/entity.cpp"
        "${solvespace_root}/src/expr.cpp"
        "${solvespace_root}/src/platform/platformbase.cpp"
        "${solvespace_root}/src/system.cpp"
        "${solvespace_root}/src/util.cpp"
        "${solvespace_root}/src/slvs/lib.cpp"
    )
    add_library(OurPaint::SolveSpace ALIAS ourpaint_solvespace)

    target_compile_definitions(ourpaint_solvespace
        PUBLIC STATIC_LIB
        PRIVATE LIBRARY)
    target_include_directories(ourpaint_solvespace
        PUBLIC "${solvespace_root}/include"
        PRIVATE "${solvespace_root}/src")
    target_include_directories(ourpaint_solvespace SYSTEM PRIVATE
        "${solvespace_root}/extlib/mimalloc/include")
    target_link_libraries(ourpaint_solvespace PRIVATE Eigen3::Eigen mimalloc-static)
endfunction()

ourpaint_add_solvespace()
