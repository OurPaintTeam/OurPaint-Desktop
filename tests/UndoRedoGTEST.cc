#include "UndoRedo/UndoRedoTemplate.h"
#include <gtest/gtest.h>

TEST(UndoRedoTest, AddAndUndo) {
    UndoRedoT<int> undoRedo;
    undoRedo.add(10);
    undoRedo.add(20);

    EXPECT_EQ(undoRedo.undo(), 20);
    EXPECT_EQ(undoRedo.undo(), 10);
}

TEST(UndoRedoTest, UndoRedoOperations) {
    UndoRedoT<int> undoRedo;
    undoRedo.add(5);
    undoRedo.add(15);

    EXPECT_EQ(undoRedo.undo(), 15);
    EXPECT_EQ(undoRedo.redo(), 15);
}

TEST(UndoRedoTest, AddAfterUndo) {
    UndoRedoT<int> undoRedo;
    undoRedo.add(1);
    undoRedo.add(2);
    undoRedo.undo();
    undoRedo.add(3);

    EXPECT_EQ(undoRedo.undo(), 3);
    EXPECT_EQ(undoRedo.undo(), 1);
}

TEST(UndoRedoTest, UndoRedoExceptions) {
    UndoRedoT<int> undoRedo;

    EXPECT_THROW(undoRedo.undo(), std::runtime_error);
    EXPECT_THROW(undoRedo.redo(), std::runtime_error);
}

TEST(UndoRedoTest, CopyConstructor) {
    UndoRedoT<int> undoRedo1;
    undoRedo1.add(10);
    undoRedo1.add(20);

    UndoRedoT<int> undoRedo2(undoRedo1);
    EXPECT_EQ(undoRedo2.undo(), 20);
    EXPECT_EQ(undoRedo2.undo(), 10);
}

TEST(UndoRedoTest, MoveConstructor) {
    UndoRedoT<int> undoRedo1;
    undoRedo1.add(30);
    undoRedo1.add(40);

    UndoRedoT<int> undoRedo2(std::move(undoRedo1));
    EXPECT_EQ(undoRedo2.undo(), 40);
    EXPECT_EQ(undoRedo2.undo(), 30);
}

TEST(UndoRedoTest, CopyAssignment) {
    UndoRedoT<int> undoRedo1;
    undoRedo1.add(50);
    undoRedo1.add(60);

    UndoRedoT<int> undoRedo2;
    undoRedo2 = undoRedo1;

    EXPECT_EQ(undoRedo2.undo(), 60);
    EXPECT_EQ(undoRedo2.undo(), 50);
}

TEST(UndoRedoTest, MoveAssignment) {
    UndoRedoT<int> undoRedo1;
    undoRedo1.add(70);
    undoRedo1.add(80);

    UndoRedoT<int> undoRedo2;
    undoRedo2 = std::move(undoRedo1);

    EXPECT_EQ(undoRedo2.undo(), 80);
    EXPECT_EQ(undoRedo2.undo(), 70);
}
