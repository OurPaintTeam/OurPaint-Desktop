add_executable(ourpaint
        main.cc
        Application.cc
        controllers/PainterController.cc
        controllers/MainWindController.cc
        controllers/LeftMenuController.cc
        SceneQtAdapter.cc

        controllers/PainterController.h
        controllers/MainWindController.h
        controllers/LeftMenuController.h
        SceneQtAdapter.h

        ###########  GUI  ############


        ../GUI/GUI_cc/MainFile/Mainwindow.cc

        ../GUI/GUI_cc/Rendering/Scaling.cc
        ../GUI/GUI_cc/Rendering/ClosestPoint.cc
        ../GUI/GUI_cc/Rendering/DrawAdditionalInf.cc
        ../GUI/GUI_cc/Rendering/DrawBackground.cc
        ../GUI/GUI_cc/Rendering/DrawFigures.cc
        ../GUI/GUI_cc/Rendering/Modes.cc
        ../GUI/GUI_cc/Rendering/QTPainter.cc


        ../GUI/GUI_cc/Rendering/Mouse/DrawTool.cc
        ../GUI/GUI_cc/Rendering/Mouse/DrawPointTool.cc
        ../GUI/GUI_cc/Rendering/Mouse/DrawLineTool.cc
        ../GUI/GUI_cc/Rendering/Mouse/DrawCircleTool.cc
        ../GUI/GUI_cc/Rendering/Mouse/DrawArcTool.cc
        ../GUI/GUI_cc/Rendering/Mouse/MouseDrawingManager.cc
        ../GUI/GUI_cc/Rendering/Mouse/DrawRectangleTool.cc

        ../GUI/GUI_cc/Events/MainWindowController.cc
        ../GUI/GUI_cc/Events/SmileRightClickFilter.cc
        ../GUI/GUI_cc/Events/KeyWorkWindow.cc
        ../GUI/GUI_cc/Events/MouseEventWorkWindow.cc

        ../GUI/GUI_cc/Widgets/LeftMenuBar.cc
        ../GUI/GUI_cc/Widgets/CustomConsole.cc
        ../GUI/GUI_cc/Widgets/EmojiWidget.cc
        ../GUI/GUI_cc/Widgets/TreeNode.cc
        ../GUI/GUI_cc/Widgets/TreeModel.cc
        ../GUI/GUI_cc/Widgets/ParameterDelegate.cc
        ../GUI/GUI_cc/Widgets/ProtectedPrefixLineEdit.cc
        ../GUI/GUI_cc/Widgets/QRegex.cc
        ../GUI/GUI_cc/Widgets/AnimationWidget.cc
        ../GUI/GUI_cc/Widgets/AngleLineEdit.cc

        ../GUI/GUI_cc/Windows/Help.cc
        ../GUI/GUI_cc/Windows/CustomWindowError.cc
        ../GUI/GUI_cc/Windows/CustomWindowSuccessful.cc
        ../GUI/GUI_cc/Windows/CustomWindowWarning.cc
        ../GUI/GUI_cc/Windows/CustomIpListWindow.cc
        ../GUI/GUI_cc/Windows/InputWindow.cc
        ../GUI/GUI_cc/Windows/SaveDialog.cc
        ../GUI/GUI_cc/Windows/CloseDialog.cc


        ../GUI/GUI_cc/Logger/GUI_Logger.cc
        ../GUI/GUI_cc/FileSystem/FileSystems.cc


        ../GUI/GUI_cc/Rendering/RenderFigures/RenderCircles.cc
        ../GUI/GUI_cc/Rendering/RenderFigures/RenderPoints.cc
        ../GUI/GUI_cc/Rendering/RenderFigures/RenderArcs.cc
        ../GUI/GUI_cc/Rendering/RenderFigures/RenderLines.cc

        ../GUI/GUI_cc/Rendering/RenderRequirements/RenderDistance.cc

        ../GUI/GUI_h/Windows/CustomWindowError.h
        ../GUI/GUI_h/Windows/CustomWindowWarning.h
        ../GUI/GUI_h/Windows/CustomWindowSuccessful.h
        ../GUI/GUI_h/Windows/Help.h
        ../GUI/GUI_h/Windows/CustomIpListWindow.h
        ../GUI/GUI_h/Windows/InputWindow.h
        ../GUI/GUI_h/Windows/SaveDialog.h
        ../GUI/GUI_h/Windows/CloseDialog.h
        ../GUI/GUI_h/Windows/ui_help.h

        ../GUI/GUI_h/Events/MainWindowController.h
        ../GUI/GUI_h/Events/KeyEventToolPanel.h
        ../GUI/GUI_h/Events/GlobalEventFilter.h
        ../GUI/GUI_h/Events/KeyEventCollapsedPanel.h
        ../GUI/GUI_h/Events/KeyTopBar.h
        ../GUI/GUI_h/Events/KeyWorkWindow.h
        ../GUI/GUI_h/Events/SmileRightClickFilter.h

        ../GUI/GUI_h/Widgets/LeftMenuBar.h
        ../GUI/GUI_h/Widgets/CustomConsole.h
        ../GUI/GUI_h/Widgets/EmojiWidget.h
        ../GUI/GUI_h/Widgets/TreeNode.h
        ../GUI/GUI_h/Widgets/TreeModel.h
        ../GUI/GUI_h/Widgets/ParameterDelegate.h
        ../GUI/GUI_h/Widgets/ProtectedPrefixLineEdit.h
        ../GUI/GUI_h/Widgets/QRegex.h
        ../GUI/GUI_h/Widgets/AnimationWidget.h
        ../GUI/GUI_h/Widgets/AngleLineEdit.h

        ../GUI/GUI_h/Rendering/Scaling.h
        ../GUI/GUI_h/Rendering/QTPainter.h
        ../GUI/GUI_h/Rendering/Colors.h

        ../GUI/GUI_h/Rendering/Mouse/DrawTool.h
        ../GUI/GUI_h/Rendering/Mouse/DrawPointTool.h
        ../GUI/GUI_h/Rendering/Mouse/DrawLineTool.h
        ../GUI/GUI_h/Rendering/Mouse/DrawCircleTool.h
        ../GUI/GUI_h/Rendering/Mouse/DrawArcTool.h
        ../GUI/GUI_h/Rendering/Mouse/MouseDrawingManager.h
        ../GUI/GUI_h/Rendering/Mouse/DrawRectangleTool.h


        ../GUI/GUI_h/MainFile/Mainwindow.h
        ../GUI/GUI_h/MainFile/ui_mainwindow.h


        ../GUI/GUI_h/Logger/GUI_Logger.h
        ../GUI/GUI_h/FileSystem/FileSystems.h


        ../GUI/GUI_h/Rendering/RenderingStyle/RenderStyle.h
        ../GUI/GUI_h/Rendering/RenderingFigures/RenderCircles.h
        ../GUI/GUI_h/Rendering/RenderingFigures/RenderPoints.h
        ../GUI/GUI_h/Rendering/RenderingFigures/RenderArcs.h
        ../GUI/GUI_h/Rendering/RenderingFigures/RenderLines.h
        ../GUI/GUI_h/Rendering/RenderRequirements/RenderDistance.h
)
target_include_directories(ourpaint
        PRIVATE
        /
        controllers

        ../GUI/GUI_h
        ../GUI/GUI_h/Windows
        ../GUI/GUI_h/Events
        ../GUI/GUI_h/Rendering
        ../GUI/GUI_h/Rendering/Mouse
        ../GUI/GUI_h/Rendering/RenderingStyle
        ../GUI/GUI_h/Rendering/RenderingFigures
        ../GUI/GUI_h/Rendering/RenderRequirements
        ../GUI/GUI_h/MainFile
        ../GUI/GUI_h/Widgets
        ../GUI/GUI_h/Logger
        ../GUI/GUI_h/FileSystem
)

set_target_properties(ourpaint PROPERTIES UNITY_BUILD OFF)

target_link_libraries(ourpaint PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Svg
        ourpaint_core
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
        target_compile_options(ourpaint PRIVATE -Wall -Wextra -Wpedantic -Werror)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(ourpaint PRIVATE /W4)
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


