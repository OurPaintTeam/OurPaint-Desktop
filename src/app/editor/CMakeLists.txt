# Shared editor/view code contains no Qt or ImGui dependencies.
add_library(ourpaint_editor STATIC
        render/RenderSceneBuilder.cc
        render/SnapFeedbackBuilder.cc
        render/constraints/ConstraintLayout.cc
        render/constraints/DimensionLayout.cc
        ViewportController.cc
        SketchEditor.cc
        snap/SnapSystem.cc
        tool/PointTool.cc
        tool/LineTool.cc
        tool/CircleTool.cc
        tool/CursorTool.cc
        tool/NurbsTool.cc
        tool/ArcTool.cc
        tool/ConstraintTool.cc
        pick/Cpu2dPicker.cc
        SelectionModel.cc
        render/AxisTexts.cc
        render/ViewportStyle.cc
        UIController.cc
)
target_compile_features(ourpaint_editor PUBLIC cxx_std_20)
target_include_directories(ourpaint_editor PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        pick
        render
        snap
        tool
)
target_link_libraries(ourpaint_editor PUBLIC
        ourpaint_document
        ourpaint_project
        ourpaint_project_io
        ourpaint_render
)
