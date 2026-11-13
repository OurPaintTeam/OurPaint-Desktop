set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets)

add_executable(ourpaint_qt
        main.cc
        Application.cc
        QtPlatformRuntime.cc
        QtViewportHost.cc
        QtMainWindowBinder.cc
        UIObserver.cc
        ProjectPersistence.cc
        QtPath.h
        CommandConsole.cc
        CommandConsole.h
)

set_target_properties(ourpaint_qt PROPERTIES UNITY_BUILD OFF)

target_link_libraries(ourpaint_qt PRIVATE
        Eigen3::Eigen
        ourpaint_project
        ourpaint_ui
)

set(TYPE_OF_COMPILING BOOL 1)
if (TYPE_OF_COMPILING)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(ourpaint_qt PRIVATE -Wall -Wextra -Wpedantic)# -Werror)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        #target_compile_options(ourpaint_qt PRIVATE /W4)
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
