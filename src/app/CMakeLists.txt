add_executable(ourpaint
        main.cc
        Application.cc
        platform/QtPlatformRuntime.cc
        platform/QtViewportHost.cc
        viewport/render/RenderSceneBuilder.cc
        viewport/ViewportController.cc
        UI/QtMainWindowBinder.cc
        UI/UIController.cc
        editor/EditorSession.cc
        editor/tools/PointTool.cc
        editor/tools/LineTool.cc
        editor/tools/CircleTool.cc
        editor/tools/CursorTool.cc
        editor/tools/CubicBezierTool.cc
        editor/tools/ArcTool.cc
        editor/tools/DimensionTool.cc
        viewport/picking/Cpu2dPicker.cc
        viewport/SelectionModel.cc
        editor/CommandConsole.cc
        editor/CommandConsole.h
        viewport/AxisTexts.cc
        project/DocumentView.cc
        viewport/ViewportStyle.cc
)

set_target_properties(ourpaint PROPERTIES UNITY_BUILD OFF)

target_link_libraries(ourpaint PRIVATE
        ourpaint_core
        ourpaint_render
        ourpaint_ui
)

target_include_directories(ourpaint PRIVATE
        editor
        platform
        project
        ui
        viewport
)

set(TYPE_OF_COMPILING BOOL 1)
if (TYPE_OF_COMPILING)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(ourpaint PRIVATE -Wall -Wextra)# -Wpedantic -Werror)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        #target_compile_options(ourpaint PRIVATE /W4)
    endif ()
endif ()

set(TYPE_OF_OPTIMIZATION BOOL 1)
if (TYPE_OF_OPTIMIZATION)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    endif ()
endif ()

add_custom_command(TARGET ourpaint
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/render/render/shaders
            $<TARGET_FILE_DIR:ourpaint>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/render/render/shaders
            $<TARGET_FILE_DIR:ourpaint>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/render/render/fonts
            $<TARGET_FILE_DIR:ourpaint>/fonts
    COMMENT "Copy shaders and fonts"
)
