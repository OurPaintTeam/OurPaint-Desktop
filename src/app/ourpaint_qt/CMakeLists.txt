add_library(ourpaint_project_io STATIC
        ../project/Project.h
        ../project/Project.cc
        ../project/ProjectSaveLoad.h
        ../project/ProjectSaveLoad.cc
        ../project/NativeFile.h
        ../project/NativeFile.cc
)
target_compile_features(ourpaint_project_io PUBLIC cxx_std_20)
target_include_directories(ourpaint_project_io PUBLIC "../project")
target_link_libraries(ourpaint_project_io PUBLIC ourpaint_document PRIVATE nlohmann_json::nlohmann_json)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

set(OURPAINT_LIB_DIR "${CMAKE_SOURCE_DIR}/third_party/UI/Lib")


if (WIN32)
    set(APP_ICON_RC "app_icon.rc")
    if (EXISTS ${APP_ICON_RC})
        set_source_files_properties(${APP_ICON_RC} PROPERTIES LANGUAGE RC)
    endif()
endif()

add_executable(ourpaint
        ${APP_ICON_RC}

        ${OURPAINT_LIB_DIR}/resources.qrc
        ${OURPAINT_LIB_DIR}/translations.qrc

        main.cc
        Application.cc

        platform/QtPlatformRuntime.cc
        viewport/QtViewportHost.cc
        controllers/QtUIController.cc
        controllers/CustomConsole.cc
        controllers/CommandConsole.cc
        controllers/CommandConsole.cc


        ../editor/DocumentController.cc
        ../editor/SketchEditor.cc
        ../editor/snapping/SnapSystem.cc
        ../editor/tools/PointTool.cc
        ../editor/tools/LineTool.cc
        ../editor/tools/CircleTool.cc
        ../editor/tools/CursorTool.cc
        ../editor/tools/NurbsTool.cc
        ../editor/tools/ArcTool.cc
        ../editor/tools/ConstraintTool.cc

        ../viewport/picking/Cpu2dPicker.cc
        ../viewport/SelectionModel.cc
        ../viewport/render/RenderSceneBuilder.cc
        ../viewport/render/SnapFeedbackBuilder.cc
        ../viewport/constraints/ConstraintLayout.cc
        ../viewport/constraints/DimensionLayout.cc
        ../viewport/ViewportController.cc
        ../viewport/AxisTexts.cc
        ../viewport/ViewportStyle.cc

        ../project/DocumentView.cc
)

set_target_properties(ourpaint PROPERTIES UNITY_BUILD OFF)

target_link_libraries(ourpaint PRIVATE
        ourpaint_core
        ourpaint_project_io
        ourpaint_render
        ourpaint_ui
)

target_include_directories(ourpaint PRIVATE
        ../editor
        ../platform
        ../platform
        ../ui
        ../../core/

        ../viewport
        ../viewport/constraints
        ../viewport/picking
        ../viewport/render


        controllers/
        platform/
        viewport/
)

install(TARGETS ourpaint
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set(TYPE_OF_COMPILING BOOL 1)
if (TYPE_OF_COMPILING)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(ourpaint PRIVATE -Wall -Wextra -Wpedantic)# -Werror)
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
            ${CMAKE_SOURCE_DIR}/third_party/fonts
            $<TARGET_FILE_DIR:ourpaint>/fonts
    COMMENT "Copy shaders and fonts"
)
