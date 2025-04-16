cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(ourpaint)
# Для использования этого приложения необходимо установить QT6. Если у вас есть вопросы, обращайтесь к @ardrasss
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include(FetchContent)

# Загрузка и подключение Eigen
FetchContent_Declare(
        Eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0  # Замените на нужную версию Eigen
)

FetchContent_MakeAvailable(Eigen)

include_directories(headers)
include_directories(headers/painters)
include_directories(headers/saveload)

include_directories(GUI/GUI_h)
include_directories(GUI/GUI_h/client_server)
include_directories(GUI/GUI_h/Windows)
include_directories(GUI/GUI_h/Events)
include_directories(GUI/GUI_h/Drawing)
include_directories(GUI/GUI_h/MainFile)
include_directories(GUI/GUI_h/Widgets)

add_executable(ourpaint
        main.cc
        src/Paint.cc
        src/GeometricObjects.cc
        src/Requirements.cc
        src/ID.cc

        src/painters/BMPpainter.cc

        src/saveload/objectInFile.cc
        src/saveload/FileOurP.cc
        src/saveload/requirementInFile.cc
        src/saveload/SaveSettingsApplications.cc
        src/saveload/LoadSettingsApplications.cc

        src/Application.cc
        src/BMPfile.cc

        headers/Enums.h
        headers/GeometricObjects.h
        headers/Paint.h
        headers/Requirements.h
        headers/ID.h

        headers/painters/Painter.h
        headers/painters/BMPpainter.h

        headers/saveload/FileOurP.h
        headers/saveload/requirementInFile.h
        headers/saveload/LoadSettingsApplications.h
        headers/saveload/objectInFile.h
        headers/saveload/SaveSettingsApplications.h

        headers/Application.h
        headers/UndoRedo.h
        headers/BMPfile.h
        headers/GeometricObjects.h


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

        GUI/GUI_cc/Events/SmileRightClickFilter.cc
        GUI/GUI_cc/Events/KeyWorkWindow.cc
        GUI/GUI_cc/Events/MouseEventWorkWindow.cc

        GUI/GUI_cc/Widgets/LeftMenuBar.cc
        GUI/GUI_cc/Widgets/CastomeConsole.cc
        GUI/GUI_cc/Widgets/EmojiWidget.cc

        GUI/GUI_cc/client_server/Server.cc
        GUI/GUI_cc/client_server/Client.cc
        GUI/GUI_cc/client_server/LocalScanner.cc
        GUI/GUI_cc/client_server/PortCheckTask.cc

        GUI/GUI_cc/Windows/Help.cc

        GUI/GUI_cc/Unnecessary/FrameOverlay.cc


        GUI/GUI_h/client_server/Client.h
        GUI/GUI_h/client_server/LocalScanner.h
        GUI/GUI_h/client_server/PortCheckTask.h
        GUI/GUI_h/client_server/Server.h

        GUI/GUI_h/Windows/CastomeWindowError.h
        GUI/GUI_h/Windows/CastomeWindowWarning.h
        GUI/GUI_h/Windows/CastomeWindowSuccessful.h
        GUI/GUI_h/Windows/Help.h
        GUI/GUI_h/Windows/CastomeIpListWindow.h
        GUI/GUI_h/Windows/WindowServer.h
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

        GUI/GUI_h/Drawing/Scaling.h
        GUI/GUI_h/Drawing/QTPainter.h
        GUI/GUI_h/Drawing/DrawMouse.h
        GUI/GUI_h/Drawing/DrawMode.h

        GUI/GUI_h/MainFile/Mainwindow.h
        GUI/GUI_h/MainFile/ui_mainwindow.h

        GUI/GUI_h/Unnecessary/FrameOverlay.h

)


include_directories(math/headers)
include_directories(math/headers/decomposition)
include_directories(math/headers/Optimizers)
include_directories(math/headers/graph)

add_subdirectory(tests)

add_subdirectory(Math)

target_link_libraries(ourpaint
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Eigen3::Eigen
)

target_link_libraries(ourpaint Math)