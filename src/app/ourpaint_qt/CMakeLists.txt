set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets OpenGL Network)

add_executable(ourpaint_qt
        main.cc
        Application.cc
        QtUIController.cc
        QtViewport.cc
        UIObserver.cc
)

get_target_property(qt_core_type Qt6::Core TYPE)
if (MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND qt_core_type STREQUAL "STATIC_LIBRARY")
    target_link_options(ourpaint_qt PRIVATE -static)
    # Keep the implicit MinGW runtime libraries at the end of the link line static.
    set_target_properties(ourpaint_qt PROPERTIES LINK_SEARCH_START_STATIC ON LINK_SEARCH_END_STATIC ON)
endif ()

set_target_properties(ourpaint_qt PROPERTIES UNITY_BUILD OFF)

target_link_libraries(ourpaint_qt PRIVATE
        Eigen3::Eigen
        ourpaint_project
        ourpaint_ui
        Qt6::Widgets
        Qt6::OpenGL
        Qt6::Network
)

target_include_directories(ourpaint_qt PRIVATE
        controllers
        viewport
)


include(GNUInstallDirs)

if (WIN32)
    set(APP_ICON_RC "app_icon.rc")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${APP_ICON_RC}")
        target_sources(ourpaint_qt PRIVATE ${APP_ICON_RC})
        set_source_files_properties(${APP_ICON_RC} PROPERTIES LANGUAGE RC)
    endif()
endif()

set(TYPE_OF_COMPILING BOOL 1)
if (TYPE_OF_COMPILING)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(ourpaint_qt PRIVATE -Wall -Wextra -Wpedantic)# -Werror)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        #target_compile_options(ourpaint_qt PRIVATE /W4)
    endif()
endif ()

set(TYPE_OF_OPTIMIZATION BOOL 1)
if (TYPE_OF_OPTIMIZATION)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    endif ()
endif ()

add_custom_command(TARGET ourpaint_qt
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/render/shaders
            $<TARGET_FILE_DIR:ourpaint_qt>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/third_party/fonts
            $<TARGET_FILE_DIR:ourpaint_qt>/fonts
    COMMENT "Copy shaders and fonts"
)
