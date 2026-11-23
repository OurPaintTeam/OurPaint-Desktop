add_ourpaint_test(ProjectSaveLoadGTEST ProjectSaveLoadGTEST.cc)
target_link_libraries(ProjectSaveLoadGTEST PRIVATE ourpaint_project_io)

add_ourpaint_test(CursorToolGTEST CursorToolGTEST.cc)
target_link_libraries(CursorToolGTEST PRIVATE ourpaint_editor)

add_ourpaint_test(ConstraintToolGTEST ConstraintToolGTEST.cc)
target_link_libraries(ConstraintToolGTEST PRIVATE ourpaint_editor)

add_ourpaint_test(LineToolGTEST LineToolGTEST.cc)
target_link_libraries(LineToolGTEST PRIVATE ourpaint_project)

add_ourpaint_test(ConstraintLayoutGTEST
        ConstraintLayoutGTEST.cc
        DimensionLayoutGTEST.cc
)
target_link_libraries(ConstraintLayoutGTEST PRIVATE OurPaint::Sketch ourpaint_project ourpaint_render glm::glm)
target_include_directories(ConstraintLayoutGTEST PRIVATE ../../src/core/sketch ../../render/render)

add_ourpaint_test(UIControllerViewsGTEST UIControllerViewsGTEST.cc UIControllerDocumentsGTEST.cc)
target_link_libraries(UIControllerViewsGTEST PRIVATE ourpaint_project)
