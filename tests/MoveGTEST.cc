#include <gtest/gtest.h>
#include "Scene.h"
#include "FileOurP.h"

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
}

