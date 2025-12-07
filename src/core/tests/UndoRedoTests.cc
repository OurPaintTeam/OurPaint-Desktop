#include "Command.h"
#include "Transaction.h"
#include "UndoRedo.h"
#include <gtest/gtest.h>

class TestStructure {
private:
    int data1;
    int data2;
    int data3;

public:
    void changeData1(int value) {
        data1 = value;
    }

    void changeData2(int value) {
        data2 = value;
    }

    void changeData3(int value) {
        data3 = value;
    }

    int getData1() const { return data1; }

    int getData2() const { return data2; }

    int getData3() const { return data3; }
};

class ChangeData1Command : public UndoRedo::Command {
private:
    TestStructure *target;
    int oldValue;
    int newValue;
protected:

    bool Execute() override {
        target->changeData1(newValue);
        return true;
    }

    bool Undo() override {
        target->changeData1(oldValue);
        return true;
    }

    bool Redo() override {
        target->changeData1(newValue);
        return true;
    }

    std::string description() const override {
        return "it changes TestStructure data1";
    }

public:
    ChangeData1Command(TestStructure *obj, int value) : target(obj), oldValue(obj->getData1()), newValue(value) {}
};

class ChangeData2Command : public UndoRedo::Command {
    TestStructure *target;
    int oldValue;
    int newValue;
public:
    ChangeData2Command(TestStructure *obj, int value) : target(obj), oldValue(obj->getData2()), newValue(value) {}

    bool Execute() override {
        target->changeData2(newValue);
        return true;
    }

    bool Undo() override {
        target->changeData2(oldValue);
        return true;
    }

    bool Redo() override {
        target->changeData2(newValue);
        return true;
    }

    std::string description() const override {
        return "it changes TestStructure data2";
    }
};

class ChangeData3Command : public UndoRedo::Command {
    TestStructure *target;
    int oldValue;
    int newValue;
public:
    ChangeData3Command(TestStructure *obj, int value) : target(obj), oldValue(obj->getData3()), newValue(value) {}

    bool Execute() override {
        target->changeData3(newValue);
        return true;
    }

    bool Undo() override {
        target->changeData3(oldValue);
        return true;
    }

    bool Redo() override {
        target->changeData3(newValue);
        return true;
    }

    std::string description() const override {
        return "it changes TestStructure data3";
    }
};


TEST(UndoRedoTests, UR) {
    TestStructure obj;
    obj.changeData1(0);
    obj.changeData2(0);
    obj.changeData3(0);

    UndoRedo::UndoRedoManager manager;

    // Первая транзакция
    {
        UndoRedo::Command *cmd1 = new ChangeData1Command(&obj, 1);
        UndoRedo::Command *cmd2 = new ChangeData2Command(&obj, 2);
        UndoRedo::Command *cmd3 = new ChangeData3Command(&obj, 3);
        UndoRedo::Transaction txn("data1 = 1, data2 = 2, data3 = 3");
        txn.addCommand(cmd1);
        txn.addCommand(cmd2);
        txn.addCommand(cmd3);
        txn.commit();
        manager.push(std::move(txn));
    }

    EXPECT_EQ(obj.getData1(), 1);
    EXPECT_EQ(obj.getData2(), 2);
    EXPECT_EQ(obj.getData3(), 3);

    // Вторая транзакция
    {
        UndoRedo::Command *cmd1 = new ChangeData1Command(&obj, 10);
        UndoRedo::Command *cmd2 = new ChangeData2Command(&obj, 20);
        UndoRedo::Command *cmd3 = new ChangeData3Command(&obj, 30);
        UndoRedo::Transaction txn("data1 = 10, data2 = 20, data3 = 30");
        txn.addCommand(cmd1);
        txn.addCommand(cmd2);
        txn.addCommand(cmd3);
        txn.commit();
        manager.push(std::move(txn));
    }

    EXPECT_EQ(obj.getData1(), 10);
    EXPECT_EQ(obj.getData2(), 20);
    EXPECT_EQ(obj.getData3(), 30);

    // Третья транзакция
    {
        UndoRedo::Command *cmd1 = new ChangeData1Command(&obj, 100);
        UndoRedo::Command *cmd2 = new ChangeData2Command(&obj, 200);
        UndoRedo::Command *cmd3 = new ChangeData3Command(&obj, 300);
        UndoRedo::Transaction txn("data1 = 100, data2 = 200, data3 = 300");
        txn.addCommand(cmd1);
        txn.addCommand(cmd2);
        txn.addCommand(cmd3);
        txn.commit();
        manager.push(std::move(txn));
    }

    EXPECT_EQ(obj.getData1(), 100);
    EXPECT_EQ(obj.getData2(), 200);
    EXPECT_EQ(obj.getData3(), 300);

    manager.undo();

    EXPECT_EQ(obj.getData1(), 10);
    EXPECT_EQ(obj.getData2(), 20);
    EXPECT_EQ(obj.getData3(), 30);

    manager.undo();

    EXPECT_EQ(obj.getData1(), 1);
    EXPECT_EQ(obj.getData2(), 2);
    EXPECT_EQ(obj.getData3(), 3);

    manager.undo();

    EXPECT_EQ(obj.getData1(), 0);
    EXPECT_EQ(obj.getData2(), 0);
    EXPECT_EQ(obj.getData3(), 0);

    bool isUndo = manager.undo();
    EXPECT_FALSE(isUndo);

    // True
    manager.redo();
    manager.redo();
    manager.redo();

    // False
    manager.redo();
    manager.redo();
    manager.redo();

    EXPECT_EQ(obj.getData1(), 100);
    EXPECT_EQ(obj.getData2(), 200);
    EXPECT_EQ(obj.getData3(), 300);

    // Мы отменили третью транзакцию
    manager.undo();

    EXPECT_EQ(obj.getData1(), 10);
    EXPECT_EQ(obj.getData2(), 20);
    EXPECT_EQ(obj.getData3(), 30);

    // Четвертая транзакция
    {
        UndoRedo::Command *cmd1 = new ChangeData1Command(&obj, 1000);
        UndoRedo::Command *cmd2 = new ChangeData2Command(&obj, 2000);
        UndoRedo::Command *cmd3 = new ChangeData3Command(&obj, 3000);
        UndoRedo::Transaction txn("data1 = 1000, data2 = 2000, data3 = 3000");
        txn.addCommand(cmd1);
        txn.addCommand(cmd2);
        txn.addCommand(cmd3);
        txn.commit();
        manager.push(std::move(txn));
    }

    EXPECT_EQ(obj.getData1(), 1000);
    EXPECT_EQ(obj.getData2(), 2000);
    EXPECT_EQ(obj.getData3(), 3000);

    manager.undo();

    EXPECT_EQ(obj.getData1(), 10);
    EXPECT_EQ(obj.getData2(), 20);
    EXPECT_EQ(obj.getData3(), 30);

    manager.redo();

    EXPECT_EQ(obj.getData1(), 1000);
    EXPECT_EQ(obj.getData2(), 2000);
    EXPECT_EQ(obj.getData3(), 3000);
}