add_ourpaint_test(ProjectSaveLoadGTEST ProjectSaveLoadGTEST.cc)
target_link_libraries(ProjectSaveLoadGTEST PRIVATE ourpaint_project_io)

add_ourpaint_test(ConstraintActionsGTEST ConstraintActionsGTEST.cc ../editor/ConstraintActions.cc)
target_link_libraries(ConstraintActionsGTEST PRIVATE OurPaint::Sketch)

add_ourpaint_test(SnapServiceGTEST
    SnapServiceGTEST.cc
    ../editor/snapping/SnapSystem.cc
    ../../render/render/Camera2D.cc
)
target_link_libraries(SnapServiceGTEST PRIVATE OurPaint::Sketch glm::glm)
target_include_directories(SnapServiceGTEST PRIVATE ../../render/render)

add_ourpaint_test(ActionReportGTEST ActionReportGTEST.cc ../ui/ActionReportPresenter.cc)
target_link_libraries(ActionReportGTEST PRIVATE ourpaint_core ourpaint_ui)

add_ourpaint_test(CursorToolGTEST
    CursorToolGTEST.cc
    DimensionInteractionGTEST.cc
    ../editor/tools/CursorTool.cc
    ../editor/ConstraintActions.cc
    ../editor/snapping/SnapSystem.cc
    ../viewport/picking/Cpu2dPicker.cc
    ../viewport/constraints/ConstraintLayout.cc
    ../viewport/constraints/DimensionLayout.cc
    ../viewport/SelectionModel.cc
    ../../render/render/Camera2D.cc
)
target_link_libraries(CursorToolGTEST PRIVATE ourpaint_document glm::glm)
target_include_directories(CursorToolGTEST PRIVATE ../editor ../viewport ../../core/sketch ../../render/render)

add_ourpaint_test(ConstraintToolsGTEST
    ConstraintToolsGTEST.cc
    ../editor/ConstraintActions.cc
    ../editor/tools/ConstraintTool.cc
    ../editor/tools/DimensionTool.cc
    ../viewport/picking/Cpu2dPicker.cc
    ../viewport/SelectionModel.cc
    ../../render/render/Camera2D.cc
)
target_link_libraries(ConstraintToolsGTEST PRIVATE OurPaint::Sketch glm::glm)
target_include_directories(ConstraintToolsGTEST PRIVATE ../../core/sketch ../../render/render)

add_ourpaint_test(SnapFeedbackGTEST
    SnapFeedbackGTEST.cc
    ../viewport/render/SnapFeedbackBuilder.cc
    ../viewport/SelectionModel.cc
    ../../render/render/Camera2D.cc
)
target_link_libraries(SnapFeedbackGTEST PRIVATE OurPaint::Sketch glm::glm)
target_include_directories(SnapFeedbackGTEST PRIVATE ../../core/sketch ../../render/render)

add_ourpaint_test(ConstraintLayoutGTEST
    ConstraintLayoutGTEST.cc
    DimensionLayoutGTEST.cc
    ../viewport/constraints/ConstraintLayout.cc
    ../viewport/constraints/DimensionLayout.cc
    ../../render/render/Camera2D.cc
)
target_link_libraries(ConstraintLayoutGTEST PRIVATE OurPaint::Sketch glm::glm)
target_include_directories(ConstraintLayoutGTEST PRIVATE ../../core/sketch ../../render/render)
