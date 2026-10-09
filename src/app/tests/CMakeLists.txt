enable_testing()
include(GoogleTest)

add_ourpaint_test(ConstraintActionsGTEST ConstraintActionsGTEST.cc ../editor/ConstraintActions.cc)
target_link_libraries(ConstraintActionsGTEST PRIVATE OurPaint::Sketch)

add_ourpaint_test(ActionReportGTEST ActionReportGTEST.cc ../ui/ActionReportPresenter.cc)
target_link_libraries(ActionReportGTEST PRIVATE ourpaint_core ourpaint_ui)

add_ourpaint_test(CursorToolGTEST
    CursorToolGTEST.cc
    ../editor/tools/CursorTool.cc
    ../editor/ConstraintActions.cc
    ../viewport/picking/Cpu2dPicker.cc
    ../viewport/SelectionModel.cc
    ../../core/Document.cc
    ../../render/render/Camera2D.cc
)
target_link_libraries(CursorToolGTEST PRIVATE OurPaint::Sketch glm::glm)
target_include_directories(CursorToolGTEST PRIVATE ../../core/sketch ../../render/render)

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
target_include_directories(ConstraintToolsGTEST PRIVATE ../../render/render)
