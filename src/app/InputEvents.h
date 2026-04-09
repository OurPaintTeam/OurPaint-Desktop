#ifndef OURPAINT_APPLICATION_INPUT_EVENTS_H_
#define OURPAINT_APPLICATION_INPUT_EVENTS_H_

#include <cstdint>

namespace input {

enum class MouseButton : uint32_t {
    None   = 0,
    Left   = 1 << 0,
    Middle = 1 << 1,
    Right  = 1 << 2,
};

enum class MouseButtonAction {
    Press,
    Release
};

enum class Modifiers : uint32_t {
    None   = 0,
    Shift  = 1 << 0,
    Ctrl   = 1 << 1,
    Alt    = 1 << 2,
    Meta   = 1 << 3
};

struct MouseMoveEvent {
    double x = 0.0; // device-independent pixels
    double y = 0.0; // device-independent pixels
    MouseButton buttons = MouseButton::None;
    Modifiers modifiers = Modifiers::None;
    uint64_t timestampMs = 0;
};

struct MouseButtonEvent {
    double x = 0.0; // device-independent pixels
    double y = 0.0; // device-independent pixels
    MouseButton button = MouseButton::None;
    MouseButtonAction action = MouseButtonAction::Press;
    MouseButton buttons = MouseButton::None;
    Modifiers modifiers = Modifiers::None;
    uint64_t timestampMs = 0;
};

inline MouseButton operator|(MouseButton a, MouseButton b) {
    return static_cast<MouseButton>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

inline MouseButton operator&(MouseButton a, MouseButton b) {
    return static_cast<MouseButton>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
    );
}

inline Modifiers operator|(Modifiers a, Modifiers b) {
    return static_cast<Modifiers>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

inline Modifiers operator&(Modifiers a, Modifiers b) {
    return static_cast<Modifiers>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
    );
}

enum class KeyAction {
    Press,
    Release,
    Repeat
};

enum class KeyCode : uint32_t {
    Unknown = 0,

    // Control keys
    Escape,
    Tab,
    Enter,
    Space,
    Backspace,
    Delete,
    Insert,

    // Navigation
    Home,
    End,
    PageUp,
    PageDown,
    Left, Right, Up, Down,

    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // Numbers (main keyboard)
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,

    // Symbols
    Comma,      // ,
    Period,     // .
    Slash,      // /
    Backslash,
    LeftBracket,   // [
    RightBracket,
    Minus,      // -
    Plus,       // +
    Asterisk,   // *

    // Function keys
    F1, F2, F3, F4, F5, F6,
    F7, F8, F9, F10, F11, F12,
};

struct KeyEvent {
    KeyCode key = KeyCode::Unknown;
    KeyAction action = KeyAction::Press;
    Modifiers modifiers = Modifiers::None;
    uint32_t nativeScanCode = 0;
    bool isAutoRepeat = false;
    uint64_t timestampMs = 0;
};

struct WheelEvent {
    double x = 0.0; // device-independent pixels
    double y = 0.0; // device-independent pixels
    double deltaX = 0.0;
    double deltaY = 0.0;
    Modifiers modifiers = Modifiers::None;
    uint64_t timestampMs = 0;
    bool isPixelPrecise = false;
};

struct ResizeEvent {
    int width = 0;
    int height = 0;
    float devicePixelRatio = 1.0f;
};

}

#endif // ! OURPAINT_APPLICATION_INPUT_EVENTS_H_