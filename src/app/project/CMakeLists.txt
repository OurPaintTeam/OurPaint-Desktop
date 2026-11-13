add_library(ourpaint_project_io STATIC
        Project.cc
        ProjectSaveLoad.cc
        NativeFile.cc
)
target_include_directories(ourpaint_project_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ourpaint_project_io PUBLIC ourpaint_document PRIVATE nlohmann_json::nlohmann_json)

# Compose editor views and project storage behind the client-facing controllers.
add_library(ourpaint_project STATIC
        DocumentView.cc
)
target_link_libraries(ourpaint_project PUBLIC
        ourpaint_project_io
        ourpaint_editor
)
