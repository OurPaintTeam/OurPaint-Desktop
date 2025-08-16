#ifndef OURPAINT_MODES_H
#define OURPAINT_MODES_H

#include <bitset>

// Enumerations for different modes
enum class WorkModes {
    Point,
    Section,
    Circle,
    Arc,
    Move,
    Selected,
    Editor,
};

enum class MouseMode {
    LeftClick,
    ReleasingLeft,
    RightClick,
    ReleasingRight,
    MouseMove,
    DoubleClickLeft,
};

enum class KeyMode {
    Enter,
    Tab,
    Shift,
    ReleasingEnter,
    ReleasingTab,
    ReleasingShift,
};

class ModeManager {
private:
    // Static bitsets for modes
    static std::bitset<static_cast<size_t>(WorkModes::Editor) + 1> workModes;
    static std::bitset<static_cast<size_t>(MouseMode::DoubleClickLeft) + 1> mouseModes;
    static std::bitset<static_cast<size_t>(KeyMode::ReleasingShift) + 1> keyModes;

    // Variables for status flags
    static bool cellEnabled;
    static bool axisEnabled;
    static bool cursorInArea;
    static bool saveFileMode;
    static bool isServer;
    static bool isConnected;

public:
    // Methods for working with modes
    static void setActiveMode(WorkModes mode);
    static bool getActiveMode(WorkModes mode);

    static void setNotActiveMode(MouseMode mode);
    static void setActiveMode(MouseMode mode);

    static bool getActiveMode(MouseMode mode);
    static void setActiveMode(KeyMode mode);

    static bool getActiveMode(KeyMode mode);

    // Methods for working with status flags
    static void setCell(bool flag);
    static bool getCell();

    static void setAxis(bool flag);
    static bool getAxis();

    static void setCursor(bool flag);
    static bool getCursor();

    static void setSave(bool flag);
    static bool getSave();

    static void setConnection(bool flag);
    static bool getConnection();

    static void setFlagServer(bool flag);
    static bool getFlagServer();
};

#endif // OURPAINT_MODES_H
