#include <gtest/gtest.h>

#include "../editor/snapping/SnapSystem.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"

using namespace core::sketch;
using namespace snap;

TEST(SnapSystem, FindsSinglePoint) {
    auto sketchResult = Sketch::create();
    ASSERT_TRUE(sketchResult);

    std::unique_ptr<Sketch> sketch = std::move(sketchResult.value());

    const auto pointIdResult = sketch->addPoint(Vec2{10.0, 20.0});

    ASSERT_TRUE(pointIdResult);

    const auto pointId = pointIdResult.value();

    SnapSystem system(*sketch);

    SnapRequest request{};
    request.cursor = Vec2{10.4, 19.7};

    const SnapResult result = system.getSnapCandidate(request);

    EXPECT_TRUE(result.snapped);
    EXPECT_EQ(result.type, SnapType::Point);

    EXPECT_DOUBLE_EQ(result.point.x, 10.0);
    EXPECT_DOUBLE_EQ(result.point.y, 20.0);

    EXPECT_NEAR(result.distance, 0.5, 1e-9);

    ASSERT_EQ(result.objectIds.size(), 1u);

    const core::ID objectId(pointId.get());

    EXPECT_EQ(result.objectIds.front(), objectId);
}

TEST(SnapSystem, EmptySketch) {
    auto sketchResult = Sketch::create();
    ASSERT_TRUE(sketchResult);

    std::unique_ptr<Sketch> sketch = std::move(sketchResult.value());

    SnapSystem system(*sketch);

    SnapRequest request{};
    request.cursor = Vec2{10.0, 20.0};

    const SnapResult result = system.getSnapCandidate(request);

    EXPECT_FALSE(result.snapped);
    EXPECT_EQ(result.type, SnapType::None);

    EXPECT_TRUE(result.objectIds.empty());
    EXPECT_TRUE(result.guideLines.empty());
    EXPECT_FALSE(result.previewObject.has_value());
    EXPECT_FALSE(result.constraint.has_value());
}
