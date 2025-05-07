cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(ourpaint)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Для использования этого приложения необходимо установить QT6. Если у вас есть вопросы, обращайтесь к @ardrasss
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Svg)

include(FetchContent)
FetchContent_Declare(
        Eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0  # Замените на нужную версию Eigen
)
FetchContent_MakeAvailable(Eigen)

include_directories(ourpaint
        headers
        headers/painters
        headers/saveload
        headers/UndoRedo
        headers/objects
        headers/commands
        GUI/GUI_h
        GUI/GUI_h/Unnecessary
        GUI/GUI_h/client_server
        GUI/GUI_h/Windows
        GUI/GUI_h/Events
        GUI/GUI_h/Drawing
        GUI/GUI_h/MainFile
        GUI/GUI_h/Widgets
        GUI/GUI_h/Logger
        GUI/GUI_h/Settings
        Math/headers
        Math/headers/decomposition
        Math/headers/Optimizers
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

        headers/commands/CommandAddObject.h
        headers/commands/CommandAddPoint.h
        headers/commands/CommandAddSection.h
        headers/commands/CommandAddCircle.h
        headers/commands/CommandAddArc.h

        src/commands/CommandAddObject.cc

        ###########  GUI  ############

        GUI/GUI_cc/MainFile/Mainwindow.cc

        GUI/GUI_cc/Drawing/Scaling.cc
        GUI/GUI_cc/Drawing/ClosesPoint.cc
        GUI/GUI_cc/Drawing/DrawAddirionalInf.cc
        GUI/GUI_cc/Drawing/DrawBackgrond.cc
        GUI/GUI_cc/Drawing/DrawFigures.cc
        GUI/GUI_cc/Drawing/Modes.cc
        GUI/GUI_cc/Drawing/DrawMouse.cc
        GUI/GUI_cc/Drawing/QTPainter.cc
        GUI/GUI_cc/Drawing/SelectedRectangle.cc

        GUI/GUI_cc/Events/SmileRightClickFilter.cc
        GUI/GUI_cc/Events/KeyWorkWindow.cc
        GUI/GUI_cc/Events/MouseEventWorkWindow.cc

        GUI/GUI_cc/Widgets/LeftMenuBar.cc
        GUI/GUI_cc/Widgets/CastomeConsole.cc
        GUI/GUI_cc/Widgets/EmojiWidget.cc
        GUI/GUI_cc/Widgets/TreeNode.cc
        GUI/GUI_cc/Widgets/TreeModelLazy.cc

        GUI/GUI_cc/client_server/Server.cc
        GUI/GUI_cc/client_server/Client.cc
        GUI/GUI_cc/client_server/LocalScanner.cc
        GUI/GUI_cc/client_server/PortCheckTask.cc

        GUI/GUI_cc/Windows/Help.cc

        GUI/GUI_cc/Unnecessary/FrameOverlay.cc

        GUI/GUI_cc/Logger/GUI_Logger.cc

        GUI/GUI_cc/Settings/Settings.cc

        GUI/GUI_h/client_server/Client.h
        GUI/GUI_h/client_server/LocalScanner.h
        GUI/GUI_h/client_server/PortCheckTask.h
        GUI/GUI_h/client_server/Server.h

        GUI/GUI_h/Windows/CastomeWindowError.h
        GUI/GUI_h/Windows/CastomeWindowWarning.h
        GUI/GUI_h/Windows/CastomeWindowSuccessful.h
        GUI/GUI_h/Windows/Help.h
        GUI/GUI_h/Windows/CastomeIpListWindow.h
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
        GUI/GUI_h/Widgets/CastomeConsole.h
        GUI/GUI_h/Widgets/EmojiWidget.h
        GUI/GUI_h/Widgets/TreeNode.h
        GUI/GUI_h/Widgets/TreeModelLazy.h

        GUI/GUI_h/Drawing/Scaling.h
        GUI/GUI_h/Drawing/QTPainter.h
        GUI/GUI_h/Drawing/DrawMouse.h
        GUI/GUI_h/Drawing/SelectedRectangle.h


        GUI/GUI_h/MainFile/Mainwindow.h
        GUI/GUI_h/MainFile/ui_mainwindow.h

        GUI/GUI_h/Unnecessary/FrameOverlay.h

        GUI/GUI_h/Logger/GUI_Logger.h

        GUI/GUI_h/Settings/Settings.h
)

set_target_properties(ourpaint PROPERTIES UNITY_BUILD ON)

add_subdirectory(tests)

target_link_libraries(ourpaint
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Eigen3::Eigen
        Qt6::Svg
        Math
)