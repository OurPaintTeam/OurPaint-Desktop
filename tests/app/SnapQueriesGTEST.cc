#include <gtest/gtest.h>

#include <array>

#include "sketch/SketchTypes.h"
#include "snap/algorithms/SnapIntersectionQuery.h"
#include "snap/algorithms/SnapQuery.h"

namespace {

using core::sketch::Circle2;
using core::sketch::EntityId;
using core::sketch::Line2;
using core::sketch::Point2;
using core::sketch::SketchEntity;
using core::sketch::SubElement;
using core::sketch::Vec2;

constexpr double EPS = 1e-9;

// ============================================================
// Midpoint / line center
// ============================================================

TEST(GeometryAlgorithms, IsLineCenterWithinTolerance) {
    const Line2 line{{0.0, 0.0}, {10.0, 4.0}};

    EXPECT_TRUE(geometry::isLineCenter(Vec2{5.0, 2.0}, line, EPS));
    EXPECT_FALSE(geometry::isLineCenter(Vec2{5.0, 3.0}, line, 0.5));
}

TEST(GeometryAlgorithms, IsLineCenterRejectsDegenerateSegment) {
    const Line2 line{{3.0, 7.0}, {3.0, 7.0}};

    EXPECT_FALSE(geometry::isLineCenter(Vec2{3.0, 7.0}, line, 1.0));
}

TEST(GeometryAlgorithms, IsLineCenterRejectsNegativeTolerance) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    EXPECT_FALSE(geometry::isLineCenter(Vec2{5.0, 0.0}, line, -1.0));
}

// ============================================================
// Nearest standalone point
// ============================================================

TEST(GeometryAlgorithms, FindNearestStandalonePoint) {
    const std::array<SketchEntity, 2> entities{{
        {
            EntityId{1},
            Point2{{1.0, 2.0}},
            false
        },
        {
            EntityId{2},
            Point2{{8.0, 9.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestStandalonePoint(Vec2{1.1, 2.1}, entities);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 1.0, EPS);
    EXPECT_NEAR(result->point.y, 2.0, EPS);
    EXPECT_NEAR(result->distanceSquared, 0.02, EPS);
    EXPECT_EQ(result->source.entity, entities[0].id);
    EXPECT_EQ(result->source.sub, SubElement::Whole);
}

TEST(GeometryAlgorithms, FindNearestStandalonePointIgnoresOtherGeometry) {
    const std::array<SketchEntity, 1> entities{{
        {
            EntityId{1},
            Line2{{0.0, 0.0}, {10.0, 0.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestStandalonePoint(Vec2{5.0, 1.0}, entities);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// Nearest line
// ============================================================

TEST(GeometryAlgorithms, FindNearestLineInterior) {
    const std::array<SketchEntity, 1> entities{{
        {
            EntityId{1},
            Line2{{0.0, 0.0}, {10.0, 0.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestLine(Vec2{4.0, 3.0}, entities);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 4.0, EPS);
    EXPECT_NEAR(result->point.y, 0.0, EPS);
    EXPECT_EQ(result->source.sub, SubElement::Whole);
}

TEST(GeometryAlgorithms, FindNearestLineStartEndpoint) {
    const std::array<SketchEntity, 1> entities{{
        {
            EntityId{1},
            Line2{{0.0, 0.0}, {10.0, 0.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestLine(Vec2{-2.0, 0.0}, entities);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->source.sub, SubElement::Start);
}

TEST(GeometryAlgorithms, FindNearestLineEndEndpoint) {
    const std::array<SketchEntity, 1> entities{{
        {
            EntityId{1},
            Line2{{0.0, 0.0}, {10.0, 0.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestLine(Vec2{12.0, 0.0}, entities);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->source.sub, SubElement::End);
}

// ============================================================
// Nearest circle
// ============================================================

TEST(GeometryAlgorithms, FindNearestCircleBoundary) {
    const std::array<SketchEntity, 1> entities{{
        {
            EntityId{1},
            Circle2{{0.0, 0.0}, 5.0},
            false
        }
    }};

    const auto result =
        geometry::findNearestCircle(Vec2{10.0, 0.0}, entities);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 5.0, EPS);
    EXPECT_NEAR(result->point.y, 0.0, EPS);
    EXPECT_EQ(result->source.sub, SubElement::Whole);
}

TEST(GeometryAlgorithms, FindNearestCircleCenter) {
    const std::array<SketchEntity, 1> entities{{
        {
            EntityId{1},
            Circle2{{0.0, 0.0}, 5.0},
            false
        }
    }};

    const auto result =
        geometry::findNearestCircle(Vec2{0.5, 0.0}, entities);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 0.0, EPS);
    EXPECT_NEAR(result->point.y, 0.0, EPS);
    EXPECT_EQ(result->source.sub, SubElement::Center);
}

// ============================================================
// Real intersections
// ============================================================

TEST(GeometryAlgorithms, FindNearestLineLineIntersection) {
    const std::array<SketchEntity, 2> entities{{
        {
            EntityId{1},
            Line2{{0.0, 0.0}, {10.0, 10.0}},
            false
        },
        {
            EntityId{2},
            Line2{{0.0, 10.0}, {10.0, 0.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestIntersection(Vec2{5.0, 5.0}, entities);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 5.0, EPS);
    EXPECT_NEAR(result->point.y, 5.0, EPS);
    EXPECT_EQ(result->first.entity, entities[0].id);
    EXPECT_EQ(result->second.entity, entities[1].id);
}

TEST(GeometryAlgorithms, FindNearestLineCircleIntersection) {
    const std::array<SketchEntity, 2> entities{{
        {
            EntityId{1},
            Line2{{-10.0, 0.0}, {10.0, 0.0}},
            false
        },
        {
            EntityId{2},
            Circle2{{0.0, 0.0}, 5.0},
            false
        }
    }};

    const auto result =
        geometry::findNearestIntersection(Vec2{4.9, 0.0}, entities);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 5.0, EPS);
    EXPECT_NEAR(result->point.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, FindNearestCircleCircleIntersection) {
    const std::array<SketchEntity, 2> entities{{
        {
            EntityId{1},
            Circle2{{0.0, 0.0}, 5.0},
            false
        },
        {
            EntityId{2},
            Circle2{{8.0, 0.0}, 5.0},
            false
        }
    }};

    const auto result =
        geometry::findNearestIntersection(Vec2{4.0, 3.0}, entities);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 4.0, EPS);
    EXPECT_NEAR(result->point.y, 3.0, EPS);
}

TEST(GeometryAlgorithms, FindNearestIntersectionIgnoresNonIntersectingSegments) {
    const std::array<SketchEntity, 2> entities{{
        {
            EntityId{1},
            Line2{{0.0, 0.0}, {2.0, 0.0}},
            false
        },
        {
            EntityId{2},
            Line2{{5.0, -1.0}, {5.0, 1.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestIntersection(Vec2{1.0, 1.0}, entities);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// Extended line intersections
// ============================================================

TEST(GeometryAlgorithms, FindNearestExtendedLineIntersection) {
    const std::array<SketchEntity, 2> entities{{
        {
            EntityId{1},
            Line2{{0.0, 0.0}, {1.0, 1.0}},
            false
        },
        {
            EntityId{2},
            Line2{{2.0, 0.0}, {3.0, -1.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestExtendedLineIntersection(Vec2{1.0, 1.0}, entities);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 1.0, EPS);
    EXPECT_NEAR(result->point.y, 1.0, EPS);
}

TEST(GeometryAlgorithms, NoExtendedIntersectionForParallelLines2) {
    const std::array<SketchEntity, 2> entities{{
        {
            EntityId{1},
            Line2{{0.0, 0.0}, {1.0, 0.0}},
            false
        },
        {
            EntityId{2},
            Line2{{0.0, 5.0}, {1.0, 5.0}},
            false
        }
    }};

    const auto result =
        geometry::findNearestExtendedLineIntersection(Vec2{0.0, 0.0}, entities);

    EXPECT_FALSE(result.has_value());
}

} // namespace
