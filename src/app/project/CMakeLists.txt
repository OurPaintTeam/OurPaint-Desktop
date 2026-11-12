add_library(ourpaint_project STATIC
        Project.cc
        ProjectSaveLoad.cc
        NativeFile.cc
        DocumentView.cc
)
target_include_directories(ourpaint_project PUBLIC
        /
)
target_link_libraries(ourpaint_project
        PUBLIC ourpaint_editor
        PRIVATE nlohmann_json::nlohmann_json
)
