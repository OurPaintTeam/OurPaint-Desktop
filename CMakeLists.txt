cmake_minimum_required(VERSION 3.20)
project(ourpaint)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)

include(FetchContent)
FetchContent_Declare(
        Eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0
)
FetchContent_MakeAvailable(Eigen)

include_directories(ourpaint
        headers
        headers/painters
        headers/saveload
        headers/UndoRedo
        headers/objects
        headers/commands
        headers/commands/Add
        headers/commands/Delete

        GUI/GUI_h
        GUI/GUI_h/Unnecessary
        GUI/GUI_h/client_server
        GUI/GUI_h/Windows
        GUI/GUI_h/Events
        GUI/GUI_h/Drawing
        GUI/GUI_h/Drawing/Mouse
        GUI/GUI_h/MainFile
        GUI/GUI_h/Widgets
        GUI/GUI_h/Logger
        GUI/GUI_h/Settings

        Math/headers
        Math/headers/decomposition
        Math/headers/optimizers
        Math/headers/graph
)

add_executable(ourpaint
        main.cc
        src/Scene.cc
        src/Requirements.cc
        src/objects/GeometricObjects.cc
        src/objects/BoundBox.cc

        src/painters/BMPpainter.cc

        src/UndoRedo/Transaction.cc
        src/UndoRedo/UndoRedoTemplate.cc
        src/UndoRedo/UndoRedo.cc

        src/saveload/objectInFile.cc
        src/saveload/FileOurP.cc
        src/saveload/requirementInFile.cc

        src/Application.cc
        src/BMPfile.cc

        headers/Requirements.h

        headers/painters/Painter.h
        headers/painters/BMPpainter.h

        headers/saveload/FileOurP.h
        headers/saveload/requirementInFile.h
        headers/saveload/objectInFile.h

        headers/Application.h
        headers/BMPfile.h

        headers/UndoRedo/Command.h
        headers/UndoRedo/Transaction.h
        headers/UndoRedo/UndoRedoTemplate.h
        headers/UndoRedo/UndoRedo.h

        headers/objects/Enums.h
        headers/objects/Objects.h
        headers/objects/GeometricObjects.h
        headers/objects/Component.h
        headers/Requirements.h
        headers/objects/ID.h
        headers/Scene.h
        headers/UndoRedo/UndoRedoTemplate.h
        headers/objects/BoundBox.h

        ########### UNDO/REDO ###########

        headers/commands/Add/CommandAddObject.h
        headers/commands/Add/CommandAddPoint.h
        headers/commands/Add/CommandAddSection.h
        headers/commands/Add/CommandAddCircle.h
        headers/commands/Add/CommandAddArc.h

        headers/commands/Delete/CommandDeletePoint.h
        headers/commands/Delete/CommandDeleteSection.h
        headers/commands/Delete/CommandDeleteCircle.h
        headers/commands/Delete/CommandDeleteArc.h

        headers/commands/CommandAddRequirement.h
        headers/commands/CommandDeleteRequirement.h

        src/commands/Add/CommandAddObject.cc
        src/commands/Delete/CommandDeleteObject.cc
        src/commands/CommandAddRequirement.cc
        src/commands/CommandDeleteRequirement.cc

        ###########  GUI  ############

        GUI/GUI_cc/MainFile/Mainwindow.cc

        GUI/GUI_cc/Drawing/Scaling.cc
        GUI/GUI_cc/Drawing/ClosestPoint.cc
        GUI/GUI_cc/Drawing/DrawAdditionalInf.cc
        GUI/GUI_cc/Drawing/DrawBackground.cc
        GUI/GUI_cc/Drawing/DrawFigures.cc
        GUI/GUI_cc/Drawing/Modes.cc
        GUI/GUI_cc/Drawing/QTPainter.cc
        GUI/GUI_cc/Drawing/SelectedRectangle.cc


        GUI/GUI_cc/Drawing/Mouse/DrawTool.cc
        GUI/GUI_cc/Drawing/Mouse/DrawPointTool.cc
        GUI/GUI_cc/Drawing/Mouse/DrawLineTool.cc
        GUI/GUI_cc/Drawing/Mouse/DrawCircleTool.cc
        GUI/GUI_cc/Drawing/Mouse/DrawArcTool.cc
        GUI/GUI_cc/Drawing/Mouse/MouseDrawingManager.cc

        GUI/GUI_cc/Events/SmileRightClickFilter.cc
        GUI/GUI_cc/Events/KeyWorkWindow.cc
        GUI/GUI_cc/Events/MouseEventWorkWindow.cc

        GUI/GUI_cc/Widgets/LeftMenuBar.cc
        GUI/GUI_cc/Widgets/CustomConsole.cc
        GUI/GUI_cc/Widgets/EmojiWidget.cc
        GUI/GUI_cc/Widgets/TreeNode.cc
        GUI/GUI_cc/Widgets/TreeModel.cc
        GUI/GUI_cc/Widgets/ParameterDelegate.cc
        GUI/GUI_cc/Widgets/ProtectedPrefixLineEdit.cc
        GUI/GUI_cc/Widgets/Regex.cc

        GUI/GUI_cc/client_server/Server.cc
        GUI/GUI_cc/client_server/Client.cc
        GUI/GUI_cc/client_server/LocalScanner.cc
        GUI/GUI_cc/client_server/PortCheckTask.cc

        GUI/GUI_cc/Windows/Help.cc
        GUI/GUI_cc/Windows/CustomWindowError.cc
        GUI/GUI_cc/Windows/CustomWindowSuccessful.cc
        GUI/GUI_cc/Windows/CustomWindowWarning.cc
        GUI/GUI_cc/Windows/CustomIpListWindow.cc
        GUI/GUI_cc/Windows/InputWindow.cc
        GUI/GUI_cc/Windows/SaveDialog.cc

        GUI/GUI_cc/Unnecessary/FrameOverlay.cc
        GUI/GUI_cc/Unnecessary/DrawMouse.cc

        GUI/GUI_cc/Logger/GUI_Logger.cc

        GUI/GUI_cc/Settings/Settings.cc

        GUI/GUI_h/client_server/Client.h
        GUI/GUI_h/client_server/LocalScanner.h
        GUI/GUI_h/client_server/PortCheckTask.h
        GUI/GUI_h/client_server/Server.h

        GUI/GUI_h/Windows/CustomWindowError.h
        GUI/GUI_h/Windows/CustomWindowWarning.h
        GUI/GUI_h/Windows/CustomWindowSuccessful.h
        GUI/GUI_h/Windows/Help.h
        GUI/GUI_h/Windows/CustomIpListWindow.h
        GUI/GUI_h/Windows/InputWindow.h
        GUI/GUI_h/Windows/SaveDialog.h
        GUI/GUI_h/Windows/ui_help.h

        GUI/GUI_h/Events/KeyEventToolPanel.h
        GUI/GUI_h/Events/GlobalEventFilter.h
        GUI/GUI_h/Events/KeyEventCollapsedPanel.h
        GUI/GUI_h/Events/KeyTopBar.h
        GUI/GUI_h/Events/KeyWorkWindow.h
        GUI/GUI_h/Events/SmileRightClickFilter.h

        GUI/GUI_h/Widgets/LeftMenuBar.h
        GUI/GUI_h/Widgets/CustomConsole.h
        GUI/GUI_h/Widgets/EmojiWidget.h
        GUI/GUI_h/Widgets/TreeNode.h
        GUI/GUI_h/Widgets/TreeModel.h
        GUI/GUI_h/Widgets/ParameterDelegate.h
        GUI/GUI_h/Widgets/ProtectedPrefixLineEdit.h
        GUI/GUI_h/Widgets/Regex.h

        GUI/GUI_h/Drawing/Scaling.h
        GUI/GUI_h/Drawing/QTPainter.h
        GUI/GUI_h/Drawing/SelectedRectangle.h
        GUI/GUI_h/Drawing/Colors.h

        GUI/GUI_h/Drawing/Mouse/DrawTool.h
        GUI/GUI_h/Drawing/Mouse/DrawPointTool.h
        GUI/GUI_h/Drawing/Mouse/DrawLineTool.h
        GUI/GUI_h/Drawing/Mouse/DrawCircleTool.h
        GUI/GUI_h/Drawing/Mouse/DrawArcTool.h
        GUI/GUI_h/Drawing/Mouse/MouseDrawingManager.h


        GUI/GUI_h/MainFile/Mainwindow.h
        GUI/GUI_h/MainFile/ui_mainwindow.h

        GUI/GUI_h/Unnecessary/FrameOverlay.h
        GUI/GUI_h/Unnecessary/DrawMouse.h

        GUI/GUI_h/Logger/GUI_Logger.h

        GUI/GUI_h/Settings/Settings.h

)

set_target_properties(ourpaint PROPERTIES UNITY_BUILD OFF)

add_subdirectory(tests)

find_package(OpenMP REQUIRED)

target_link_libraries(ourpaint PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Eigen3::Eigen
        Qt6::Svg
        Math
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
        target_compile_options(ourpaint PRIVATE -Wall -Wextra -Wpedantic)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(ourpaint PRIVATE /W4)
    endif ()
endif ()

