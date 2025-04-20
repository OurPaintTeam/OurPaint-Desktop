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
include_directories(headers/client_server)
include_directories(headers/GUI)
include_directories(headers/GUI/Windows)
include_directories(headers/UndoRedo)

add_executable(ourpaint
        main.cc
        src/Scene.cc
        src/Requirements.cc
        src/Scaling.cc
        src/objects/GeometricObjects.cc
        src/objects/BoundBox.cc

        src/painters/BMPpainter.cc
        src/painters/QTPainter.cc

        src/GUI/mainwindow.cc
        src/GUI/FrameOverlay.cc
        src/GUI/CastomeConsole.cc
        src/GUI/RightClickFilter.cc
        src/GUI/EmojiWidget.cc

        src/GUI/Windows/help.cc

        src/saveload/objectInFile.cc
        src/saveload/FileOurP.cc
        src/saveload/requirementInFile.cc
        src/saveload/SaveSettingsApplications.cc
        src/saveload/LoadSettingsApplications.cc

        src/UndoRedo/Transactions.cc
        src/UndoRedo/UndoRedoTemplate.cc
        src/UndoRedo/UndoRedo.cc

        src/client_server/Server.cc
        src/client_server/Client.cc
        src/client_server/LocalScanner.cc
        src/client_server/PortCheckTask.cc

        src/Application.cc
        src/BMPfile.cc

        headers/UndoRedo/Command.h
        headers/UndoRedo/Transactions.h
        headers/UndoRedo/UndoRedoTemplate.h
        headers/UndoRedo/UndoRedo.h

        headers/objects/Enums.h
        headers/objects/Objects.h
        headers/objects/GeometricObjects.h
        headers/Requirements.h
        headers/Scaling.h
        headers/objects/ID.h
        headers/Scene.h
        headers/objects/BoundBox.h

        headers/client_server/Client.h
        headers/client_server/LocalScanner.h
        headers/client_server/PortCheckTask.h
        headers/client_server/Server.h

        headers/GUI/Windows/CastomeWindowError.h
        headers/GUI/Windows/CastomeWindowWarning.h
        headers/GUI/Windows/CastomeWindowSuccessful.h
        headers/GUI/Windows/help.h
        headers/GUI/Windows/CastomeIpListWindow.h
        headers/GUI/Windows/WindowServer.h
        headers/GUI/Windows/SaveDialog.h
        headers/GUI/Windows/ui_help.h
        headers/GUI/DrawMode.h

        headers/GUI/CastomeConsole.h
        headers/GUI/EmojiWidget.h
        headers/GUI/FrameOverlay.h
        headers/GUI/mainwindow.h
        headers/GUI/RightClickFilter.h
        headers/GUI/ui_mainwindow.h

        headers/painters/Painter.h
        headers/painters/QTPainter.h
        headers/painters/BMPpainter.h

        headers/saveload/FileOurP.h
        headers/saveload/requirementInFile.h
        headers/saveload/LoadSettingsApplications.h
        headers/saveload/objectInFile.h
        headers/saveload/SaveSettingsApplications.h

        headers/Application.h
        headers/UndoRedo/UndoRedoTemplate.h
        headers/BMPfile.h
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
