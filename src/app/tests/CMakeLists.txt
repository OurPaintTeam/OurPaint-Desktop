enable_testing()
include(GoogleTest)

add_ourpaint_test(ConstraintActionsGTEST ConstraintActionsGTEST.cc ../editor/ConstraintActions.cc)
target_link_libraries(ConstraintActionsGTEST PRIVATE OurPaint::Sketch)
