#ifndef OURPAINT_COLORS_H
#define OURPAINT_COLORS_H

#include <QString>
#include <QColor>
#include <QMap>

enum class Color {
    White,
    Black,
    Red,
    DarkRed,
    Green,
    DarkGreen,
    Blue,
    DarkBlue,
    Cyan,
    DarkCyan,
    Magenta,
    DarkMagenta,
    Yellow,
    DarkYellow,
    Gray,
    DarkGray,
    LightGray,
    Purple
};

inline Color stringToColor(const QString& name) {
    static const QMap<QString, Color> map = {
            {"White", Color::White},
            {"Black", Color::Black},
            {"Red", Color::Red},
            {"DarkRed", Color::DarkRed},
            {"Green", Color::Green},
            {"DarkGreen", Color::DarkGreen},
            {"Blue", Color::Blue},
            {"DarkBlue", Color::DarkBlue},
            {"Cyan", Color::Cyan},
            {"DarkCyan", Color::DarkCyan},
            {"Magenta", Color::Magenta},
            {"DarkMagenta", Color::DarkMagenta},
            {"Yellow", Color::Yellow},
            {"DarkYellow", Color::DarkYellow},
            {"Gray", Color::Gray},
            {"DarkGray", Color::DarkGray},
            {"LightGray", Color::LightGray},
            {"Purple", Color::Purple}
    };
    return map.value(name, Color::Black);
}

inline const QColor& ColorToQColor(Color color) {
    static const QColor colorMap[] = {
            QColor(Qt::white),
            QColor(Qt::black),
            QColor(Qt::red),
            QColor(139, 0, 0),
            QColor(Qt::green),
            QColor(0, 100, 0),
            QColor(Qt::blue),
            QColor(0, 0, 139),
            QColor(Qt::cyan),
            QColor(0, 139, 139),
            QColor(Qt::magenta),
            QColor(139, 0, 139),
            QColor(Qt::yellow),
            QColor(204, 204, 0),
            QColor(Qt::gray),
            QColor(Qt::darkGray),
            QColor(Qt::lightGray),
            QColor(128, 0, 128),
    };
    return colorMap[static_cast<std::size_t>(color)];
}

inline QColor colorToGlow(Color color) {
    switch (color) {
        case Color::Blue:   return QColor(30, 120, 200);
        case Color::Purple: return QColor(128, 0, 128);
        case Color::Cyan:   return QColor(0, 255, 255);
        default:            return QColor(0, 0, 0, 0);
    }
}

#endif //OURPAINT_COLORS_H
