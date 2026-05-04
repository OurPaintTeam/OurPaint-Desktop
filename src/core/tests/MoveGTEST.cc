#include <gtest/gtest.h>
/*
#include "Scene.h"
#include "FileOurP.h"

TEST(ComponentTest, ctest) {
    Scene scene(nullptr);

    ObjectData sec1;
    sec1.params = {0, 0, 10, 10};
    sec1.et = ObjType::ET_SECTION;

    ObjectData sec2;
    sec2.params = {0, 0, 20, 10};
    sec2.et = ObjType::ET_SECTION;

    scene.addObject(sec1);
    scene.addObject(sec2);

    RequirementData req1;
    req1.req = ET_POINTPOINTDIST;
    req1.objects = { ID(1), ID{2} };
    req1.params = {100};

    RequirementData req2;
    req2.req = ET_POINTPOINTDIST;
    req2.objects = { ID(4), ID{5} };
    req2.params = {100};

    RequirementData req3;
    req3.req = ET_POINTONPOINT;
    req3.objects = { ID(1), ID{4} };

    scene.addRequirement(req1, false);
    scene.addRequirement(req2, false);
    scene.addRequirement(req3);
    SUCCEED();
}

TEST(MoveTest, LoadAndMove) {
    Scene scene(nullptr);
    
    scene.loadFromFile("./testStructures/homeFINAL2.ourp");
    
    try {
        scene.moveObject(ID(1), 10.0, 10.0);
        
        scene.moveObject(ID(3), 5.0, 5.0);
        scene.moveObject(ID(92), -5.0, -5.0);
    } catch (const std::exception& e) {
        FAIL() << "Exception occurred: " << e.what();
    }
    SUCCEED();
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}*/

