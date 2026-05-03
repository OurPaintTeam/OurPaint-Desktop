add_executable(ourpaint
        main.cc
        Application.cc
        QtPlatformRuntime.cc
        QtViewportHost.cc
        RenderDataBuilder.cc
        ViewportController.cc
        QtMainWindowBinder.cc
        UIController.cc
        InteractionTools/EditorSession.cc
        InteractionTools/PointTool.cc
        InteractionTools/LineTool.cc
        InteractionTools/CircleTool.cc
        InteractionTools/CursorTool.cc
        InteractionTools/CubicBezierTool.cc
        InteractionTools/ArcTool.cc
        Cpu2dPicker.cc
        SelectionModel.cc
        CommandConsole.cc
        CommandConsole.h
        AxisTexts.cc
)

set_target_properties(ourpaint PROPERTIES UNITY_BUILD OFF)

target_link_libraries(ourpaint PRIVATE
        ourpaint_core
        ourpaint_rendering
        ourpaint_ui
)


if (OpenMP_CXX_FOUND)
    target_link_libraries(ourpaint PRIVATE
            OpenMP::OpenMP_CXX
    )
    target_compile_options(ourpaint PRIVATE
            ${OpenMP_CXX_FLAGS}
    )
    target_link_options(ourpaint PRIVATE
            ${OpenMP_CXX_FLAGS}
    )
endif ()

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


