#include <gtest/gtest.h>
#include "Arc.h"
#include "Point2D.h"
#include <cmath>
using namespace OurPaintDCM::Figures;

TEST(ArcTest, ConstructorAndDefaults) {
    Point2D p1({0.0, 0.0});
    Point2D p2({1.0, 0.0});
    Point2D center({0.5, 0.5});

    Arc<Point2D> arc(&p1, &p2, &center);

    EXPECT_EQ(arc.p1, &p1);
    EXPECT_EQ(arc.p2, &p2);
    EXPECT_EQ(arc.p_center, &center);
}

TEST(ArcTest, RadiusConsistency) {
    Point2D p1({1.0, 0.0});
    Point2D p2({0.0, 1.0});
    Point2D center({0.0, 0.0});

    Arc<Point2D> arc(&p1, &p2, &center);

    double r1 = std::sqrt((p1[0] - center[0])*(p1[0] - center[0]) + (p1[1] - center[1])*(p1[1] - center[1]));
    double r2 = std::sqrt((p2[0] - center[0])*(p2[0] - center[0]) + (p2[1] - center[1])*(p2[1] - center[1]));

    EXPECT_DOUBLE_EQ(r1, r2);
}


TEST(ArcTest, PointModification) {
    Point2D p1({0.0, 0.0});
    Point2D p2({1.0, 0.0});
    Point2D center({0.5, 0.5});

    Arc<Point2D> arc(&p1, &p2, &center);

    p1[0] = 2.0;
    p1[1] = 3.0;

    EXPECT_DOUBLE_EQ(arc.p1->coords[0], 2.0);
    EXPECT_DOUBLE_EQ(arc.p1->coords[1], 3.0);
}
