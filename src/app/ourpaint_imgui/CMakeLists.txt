set(icon_files
    icons/primitives/point.svg
    icons/primitives/line.svg
    icons/primitives/circleByRadius.svg
    icons/primitives/arcByRadius.svg
    icons/primitives/beze.svg
    icons/constraints/Distance.svg
    icons/constraints/VDistance.svg
    icons/constraints/HDistance.svg
    icons/constraints/Angle.svg
    icons/constraints/Coincident.svg
    icons/constraints/Horizontal.svg
    icons/constraints/Vertical.svg
    icons/constraints/Parallel.svg
    icons/constraints/Perpendicular.svg
    icons/constraints/Tangent.svg
    icons/constraints/Equal.svg
    icons/constraints/FixUnfix.svg
    icons/constraints/Concentric.svg
    icons/constraints/Collinear.svg
    icons/constraints/Midpoint.svg
    icons/constraints/Symmetric.svg
    icons/constraints/Curvature.svg
)

set(icon_header "${CMAKE_CURRENT_BINARY_DIR}/ToolbarIcons.h")
add_custom_command(
    OUTPUT "${icon_header}"
    COMMAND "${CMAKE_COMMAND}" "-DICON_FILES=${icon_files}" "-DOUTPUT=${icon_header}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/GenerateIcons.cmake"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS ${icon_files} GenerateIcons.cmake CMakeLists.txt
    COMMENT "Generating ImGui toolbar icons"
    VERBATIM
)

add_executable(ourpaint_imgui
        main.cc
        "${icon_header}"
)

target_include_directories(ourpaint_imgui PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

if (MINGW)
    target_link_options(ourpaint_imgui PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
        -s
    )
endif()

target_link_libraries(ourpaint_imgui PUBLIC
        ourpaint_project
        glad
        imgui
        imgui_demo
        nanosvg
        nanosvgrast
)

# The existing renderer resolves these resources relative to its working directory.
add_custom_command(TARGET ourpaint_imgui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/render/shaders $<TARGET_FILE_DIR:ourpaint_imgui>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/third_party/fonts $<TARGET_FILE_DIR:ourpaint_imgui>/fonts
    COMMENT "Copy shaders and fonts"
)

set_target_properties(ourpaint_imgui PROPERTIES WIN32_EXECUTABLE TRUE)

