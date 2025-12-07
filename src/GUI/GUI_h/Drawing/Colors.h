#ifndef OURPAINT_COLORS_H
#define OURPAINT_COLORS_H
#include <QString>
#include <QColor>

enum class Color {
    Blue,
    Purple,
    Cyan
};

inline QString colorToString(Color color) {
    switch (color) {
        case Color::Blue:   return "Blue";
        case Color::Purple: return "Purple";
        case Color::Cyan:   return "Cyan";
        default:            return "Unknown";
    }
}

inline QColor colorToQColor(Color color) {
    switch (color) {
        case Color::Blue:   return QColor(30, 120, 200);
        case Color::Purple: return QColor(128, 0, 128);
        case Color::Cyan:   return QColor(0, 255, 255);
        default:            return QColor(0, 0, 0);
    }
}

#endif //OURPAINT_COLORS_H
