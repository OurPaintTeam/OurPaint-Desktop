enable_testing()
include(GoogleTest)

add_ourpaint_test(ConstraintActionsGTEST ConstraintActionsGTEST.cc ../editor/ConstraintActions.cc)
target_link_libraries(ConstraintActionsGTEST PRIVATE OurPaint::Sketch)

add_ourpaint_test(SnapServiceGTEST.cc SnapServiceGTEST.cc ../editor/snapping/SnapSystem.cc)
target_link_libraries(SnapServiceGTEST.cc PRIVATE OurPaint::Sketch)