#include <gtest/gtest.h>

#include "../../src/app/editor/snap/SnapSystem.h"
#include "Camera2D.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"

using namespace core::sketch;
using namespace snap;

TEST(SnapSystem, FindsSinglePoint) {
    Sketch sketch;
    Camera2D camera;
    camera.setZoom(1.0);

    const auto pointIdResult = sketch.addPoint(Vec2{10.0, 20.0});
    ASSERT_TRUE(pointIdResult);

    const auto pointId = pointIdResult.value();

    SnapSystem system(sketch, camera);

    SnapRequest request{};
    request.cursor = Vec2{10.4, 19.7};
    request.tool = ToolType::Point;
    request.drawState = DrawState::Idle;

    const SnapResult result = system.getSnapCandidate(request);

    ASSERT_TRUE(result.snapped);
    EXPECT_EQ(result.type, SnapKind::StandalonePoint);

    EXPECT_DOUBLE_EQ(result.point.position.x, 10.0);
    EXPECT_DOUBLE_EQ(result.point.position.y, 20.0);

    ASSERT_EQ(result.objects.size(), 1u);

    const auto& ref = result.objects.front();

    EXPECT_EQ(ref.entity, pointId);
    EXPECT_EQ(ref.sub, SubElement::Whole);
}

TEST(SnapSystem, EmptySketch) {
    Sketch sketch;
    Camera2D camera;
    SnapSystem system(sketch, camera);

    SnapRequest request{};
    request.cursor = Vec2{10.0, 20.0};
    request.tool = ToolType::Point;
    request.drawState = DrawState::Idle;

    const SnapResult result = system.getSnapCandidate(request);

    EXPECT_FALSE(result.snapped);
    EXPECT_EQ(result.type, SnapKind::None);

    EXPECT_TRUE(result.objects.empty());
    EXPECT_TRUE(result.guideLines.empty());
}
