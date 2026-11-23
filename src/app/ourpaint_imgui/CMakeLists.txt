add_executable(ourpaint_imgui
        main.cc
)

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
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/app/ourpaint_imgui/icons $<TARGET_FILE_DIR:ourpaint_imgui>/icons
    COMMENT "Copy shaders, fonts and icons"
)

set_target_properties(ourpaint_imgui PROPERTIES WIN32_EXECUTABLE TRUE)

