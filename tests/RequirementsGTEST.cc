#include <gtest/gtest.h>
#include "Requirements.h"

class RequirementTest : public ::testing::Test {
protected:
    void SetUp() override {
        VarsStorage::clearVars();
    }
    void TearDown() override {
        VarsStorage::clearVars();
    }
};

TEST_F(RequirementTest, ReqPointSecDist) {
    Point p; p.x = 1.0; p.y = 2.0;
    Point beg; beg.x = 0.0; beg.y = 0.0;
    Point end; end.x = 3.0; end.y = 4.0;
    Section s; s.beg = &beg; s.end = &end;
    ReqPointSecDist req(&p, &s, 5.0);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 6);
}

TEST_F(RequirementTest, ReqPointOnSec) {
    Point p; p.x = 1.0; p.y = 2.0;
    Point beg; beg.x = 0.0; beg.y = 0.0;
    Point end; end.x = 3.0; end.y = 4.0;
    Section s; s.beg = &beg; s.end = &end;
    ReqPointOnSec req(&p, &s);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 6);
}

TEST_F(RequirementTest, ReqPointPointDist) {
    Point p1; p1.x = 1.0; p1.y = 2.0;
    Point p2; p2.x = 3.0; p2.y = 4.0;
    ReqPointPointDist req(&p1, &p2, 5.0);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 4);
}

TEST_F(RequirementTest, ReqPointOnPoint) {
    Point p1; p1.x = 1.0; p1.y = 2.0;
    Point p2; p2.x = 3.0; p2.y = 4.0;
    ReqPointOnPoint req(&p1, &p2);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 4);
}

TEST_F(RequirementTest, ReqSecCircleDist) {
    Point beg; beg.x = 0.0; beg.y = 0.0;
    Point end; end.x = 3.0; end.y = 4.0;
    Section s; s.beg = &beg; s.end = &end;
    Point center; center.x = 5.0; center.y = 5.0;
    Circle c; c.center = &center; c.R = 2.0;
    ReqSecCircleDist req(&s, &c, 3.0);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 7);
}

TEST_F(RequirementTest, ReqSecOnCircle) {
    Point beg; beg.x = 0.0; beg.y = 0.0;
    Point end; end.x = 3.0; end.y = 4.0;
    Section s; s.beg = &beg; s.end = &end;
    Point center; center.x = 5.0; center.y = 5.0;
    Circle c; c.center = &center; c.R = 2.0;
    ReqSecOnCircle req(&s, &c);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 7);
}

TEST_F(RequirementTest, ReqSecSecParallel) {
    Point beg1; beg1.x = 0.0; beg1.y = 0.0;
    Point end1; end1.x = 3.0; end1.y = 4.0;
    Section s1; s1.beg = &beg1; s1.end = &end1;

    Point beg2; beg2.x = 1.0; beg2.y = 1.0;
    Point end2; end2.x = 4.0; end2.y = 5.0;
    Section s2; s2.beg = &beg2; s2.end = &end2;

    ReqSecSecParallel req(&s1, &s2);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 8);
}

TEST_F(RequirementTest, ReqSecSecPerpendicular) {
    Point beg1; beg1.x = 0.0; beg1.y = 0.0;
    Point end1; end1.x = 3.0; end1.y = 0.0;
    Section s1; s1.beg = &beg1; s1.end = &end1;

    Point beg2; beg2.x = 1.5; beg2.y = -2.0;
    Point end2; end2.x = 1.5; end2.y = 2.0;
    Section s2; s2.beg = &beg2; s2.end = &end2;

    ReqSecSecPerpendicular req(&s1, &s2);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 8);
}

TEST_F(RequirementTest, ReqSecSecAngel) {
    Point beg1; beg1.x = 0.0; beg1.y = 0.0;
    Point end1; end1.x = 3.0; end1.y = 4.0;
    Section s1; s1.beg = &beg1; s1.end = &end1;

    Point beg2; beg2.x = 1.0; beg2.y = 1.0;
    Point end2; end2.x = 4.0; end2.y = 1.0;
    Section s2; s2.beg = &beg2; s2.end = &end2;

    ReqSecSecAngel req(&s1, &s2, 45.0);

    EXPECT_NE(req.getFunction(), nullptr);
    EXPECT_EQ(req.getParams().size(), 8);
}
