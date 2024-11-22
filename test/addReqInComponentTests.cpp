#include <gtest/gtest.h>
#include "objects.h"
#include "requirements.h"
#include "paint.h"

TEST(PaintTest, ConnectedComponentsMerge) {
    Painter *painter = nullptr;
    Paint screen(painter);

    // First component
    ElementData data_point1;
    data_point1.et = ET_POINT;
    data_point1.params[0] = 0;  // x1
    data_point1.params[1] = 0;  // y1
    ID id_point1 = screen.addElement(data_point1);

    ElementData data_point2;
    data_point2.et = ET_POINT;
    data_point2.params[0] = 10; // x2
    data_point2.params[1] = 0;  // y2
    ID id_point2 = screen.addElement(data_point2);

    RequirementData req1;
    req1.req = ET_POINTPOINTDIST;
    req1.objects.push_back(id_point1);
    req1.objects.push_back(id_point2);
    req1.params.push_back(10.0); // distance between two points should be 10
    screen.addRequirement(req1);

    // Second Component
    ElementData data_point3;
    data_point3.et = ET_POINT;
    data_point3.params[0] = 0;  // x3
    data_point3.params[1] = 10; // y3
    ID id_point3 = screen.addElement(data_point3);

    ElementData data_point4;
    data_point4.et = ET_POINT;
    data_point4.params[0] = 10; // x4
    data_point4.params[1] = 10; // y4
    ID id_point4 = screen.addElement(data_point4);

    RequirementData req2;
    req2.req = ET_POINTPOINTDIST;
    req2.objects.push_back(id_point3);
    req2.objects.push_back(id_point4);
    req2.params.push_back(10.0); // distance between two points should be 10
    screen.addRequirement(req2);

    // New req
    RequirementData req3;
    req3.req = ET_POINTPOINTDIST;
    req3.objects.push_back(id_point2); // Point 2 from first component
    req3.objects.push_back(id_point3); // Point 3 from second component
    req3.params.push_back(14.1421356237); // distance between two points should be 14.142 (sqrt(200))
    screen.addRequirement(req3);

    double x1 = screen.getElementInfo(id_point1).params[0];
    double y1 = screen.getElementInfo(id_point1).params[1];

    double x2 = screen.getElementInfo(id_point2).params[0];
    double y2 = screen.getElementInfo(id_point2).params[1];

    double x3 = screen.getElementInfo(id_point3).params[0];
    double y3 = screen.getElementInfo(id_point3).params[1];

    double x4 = screen.getElementInfo(id_point4).params[0];
    double y4 = screen.getElementInfo(id_point4).params[1];


    // Check distance point 1 and point 2
    double dist12 = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
    EXPECT_NEAR(dist12, 10.0, 1e-6);

    // Check distance point 3 and point 4
    double dist34 = sqrt((x3 - x4)*(x3 - x4) + (y3 - y4)*(y3 - y4));
    EXPECT_NEAR(dist34, 10.0, 1e-6);

    // Check distance point 2 and point 3
    double dist23 = sqrt((x2 - x3)*(x2 - x3) + (y2 - y3)*(y2 - y3));
    EXPECT_NEAR(dist23, 14.1421356237, 1e-6);
}
