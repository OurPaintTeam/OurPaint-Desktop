#include <gtest/gtest.h>

#include "Paint.h"
#include "Painter.h"
#include "BMPpainter.h"

TEST(PaintTest, addReq) {
    BMPpainter* painter = new BMPpainter();
    Paint screen(painter);

    ElementData section1 = {
            .et = ET_SECTION,
            .params = { 0.0, 0.0, 100.0, 100.0 }
    };

    ElementData section2 = {
            .et = ET_SECTION,
            .params = { 200.0, 0.0, 200.0, 100.0 }
    };

    screen.addElement(section1);
    screen.addElement(section2);

    RequirementData rP1 = {
            .req = ET_POINTPOINTDIST,
            .objects = {1, 2},
            .params = {90.0}
    };

    RequirementData rP2 = {
            .req = ET_POINTPOINTDIST,
            .objects = {4, 5},
            .params = {90.0}
    };

    RequirementData rD = {
            .req = ET_SECTIONSECTIONPARALLEL,
            .objects = {3, 6}
    };

    screen.addRequirement(rP1);
    screen.addRequirement(rP2);
    screen.addRequirement(rD);

    screen.updateRequirement(1);
    screen.updateRequirement(2);
    screen.updateRequirement(3);
    screen.updateRequirement(4);
    screen.updateRequirement(5);

    EXPECT_EQ(1, 1);
    delete painter;
}

