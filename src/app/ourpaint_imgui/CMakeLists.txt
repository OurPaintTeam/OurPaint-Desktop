add_executable(ourpaint_imgui
        main.cc
        ImGuiViewportHost.cc
)

target_link_libraries(ourpaint_imgui PUBLIC
        ourpaint_project
        glad
        imgui
        #imgui_demo
        nanosvg
        nanosvgrast
)

# The existing renderer resolves these resources relative to its working directory.
add_custom_command(TARGET ourpaint_imgui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/render/shaders $<TARGET_FILE_DIR:ourpaint_imgui>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/third_party/fonts $<TARGET_FILE_DIR:ourpaint_imgui>/fonts
    COMMENT "Copy ImGui CAD shaders and fonts"
)

set_target_properties(ourpaint_imgui PROPERTIES WIN32_EXECUTABLE TRUE)

