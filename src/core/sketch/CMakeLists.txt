# Keep Sketch independent of the legacy Core/Scene target and its consumers.
add_library(ourpaint_sketch STATIC Sketch.cc)
add_library(OurPaint::Sketch ALIAS ourpaint_sketch)
target_compile_features(ourpaint_sketch PUBLIC cxx_std_20)
target_include_directories(ourpaint_sketch PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(ourpaint_sketch PRIVATE OurPaintDCM)
