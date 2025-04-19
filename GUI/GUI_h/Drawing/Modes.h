#ifndef OURPAINT_MODES_H
#define OURPAINT_MODES_H

#include <bitset>

// Перечисления для разных режимов
enum class CursorMode {
    InArea,
};

enum class SaveFileMode {
    Save,
};

enum class WorkModes {
    Point,
    Section,
    Circle,
    Move,
    Resize,
    Rotate,
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

enum class SettingsMode {
    Cell,
    Axis
};

class ModeManager {
private:
    // Статические битовые наборы для режимов
    static std::bitset<static_cast<size_t>(WorkModes::Editor) + 1> workModes;
    static std::bitset<static_cast<size_t>(MouseMode::DoubleClickLeft) + 1> mouseModes;
    static std::bitset<static_cast<size_t>(KeyMode::ReleasingShift) + 1> keyModes;

    // Переменные для флагов состояния
    static bool cellEnabled;
    static bool axisEnabled;
    static bool cursorInArea;
    static bool saveFileMode;

public:
    // Методы для работы с режимами
    static void setActiveMode(WorkModes mode);
    static bool getActiveMode(WorkModes mode);

    static void setNotActiveMode(MouseMode mode);
    static void setActiveMode(MouseMode mode);
    static bool getActiveMode(MouseMode mode);

    static void setActiveMode(KeyMode mode);
    static bool getActiveMode(KeyMode mode);

    // Методы для работы с флагами состояния
    static void setCell(bool flag);
    static bool getCell();
    static void setAxis(bool flag);
    static bool getAxis();

    static void setCursor(bool flag);
    static bool getCursor();

    static void setSave(bool flag);
    static bool getSave();
};

#endif // OURPAINT_MODES_H
