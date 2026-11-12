add_ourpaint_test(ProjectSaveLoadGTEST ProjectSaveLoadGTEST.cc)
target_link_libraries(ProjectSaveLoadGTEST PRIVATE ourpaint_project_io)

add_ourpaint_test(SnapServiceGTEST
        SnapServiceGTEST.cc
        SnapQueriesGTEST.cc
        ../../src/app/editor/snap/SnapSystem.cc
        ../../src/render/Camera2D.cc
)
target_link_libraries(SnapServiceGTEST PRIVATE OurPaint::Sketch glm::glm)
target_include_directories(SnapServiceGTEST PRIVATE ../../src/app/editor ../../render/render)

#add_ourpaint_test(CursorToolGTEST
#    CursorToolGTEST.cc
#    ../editor/tools/CursorTool.cc
#    ../editor/snapping/SnapSystem.cc
#    ../viewport/picking/Cpu2dPicker.cc
#    ../viewport/constraints/ConstraintLayout.cc
#    ../viewport/constraints/DimensionLayout.cc
#    ../viewport/SelectionModel.cc
#    ../../render/render/Camera2D.cc
#)
#target_link_libraries(CursorToolGTEST PRIVATE ourpaint_document glm::glm)
#target_include_directories(CursorToolGTEST PRIVATE ../editor ../viewport ../../core/sketch ../../render/render)

#add_ourpaint_test(ConstraintToolsGTEST
#    ConstraintToolsGTEST.cc
#    ../editor/tools/ConstraintTool.cc
#    ../editor/tools/DimensionTool.cc
#    ../viewport/picking/Cpu2dPicker.cc
#    ../viewport/SelectionModel.cc
#    ../../render/render/Camera2D.cc
#)
#target_link_libraries(ConstraintToolsGTEST PRIVATE OurPaint::Sketch glm::glm)
#target_include_directories(ConstraintToolsGTEST PRIVATE ../../core/sketch ../../render/render)

add_ourpaint_test(SnapFeedbackGTEST
        SnapFeedbackGTEST.cc
        ../../src/app/editor/render/SnapFeedbackBuilder.cc
        ../../src/app/editor/SelectionModel.cc
        ../../src/render/Camera2D.cc
)
target_link_libraries(SnapFeedbackGTEST PRIVATE OurPaint::Sketch glm::glm)
target_include_directories(SnapFeedbackGTEST PRIVATE ../../src/core/sketch ../../render/render)

add_ourpaint_test(ConstraintLayoutGTEST
        ConstraintLayoutGTEST.cc
        DimensionLayoutGTEST.cc
        ../../src/app/editor/render/constraints/ConstraintLayout.cc
        ../../src/app/editor/render/constraints/DimensionLayout.cc
        ../../src/render/Camera2D.cc
)
target_link_libraries(ConstraintLayoutGTEST PRIVATE OurPaint::Sketch glm::glm)
target_include_directories(ConstraintLayoutGTEST PRIVATE ../../src/core/sketch ../../render/render)
