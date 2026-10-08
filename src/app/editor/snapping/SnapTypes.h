#pragma once
namespace snap {

/// Активный инструмент редактора.
enum class ToolType {
    Select,

    Point,
    Line,
    Circle,
    Arc,

    Move,
    Rotate,
    Scale
};

/// Тип позиционной привязки курсора.
enum class SnapType {
    None,                  // Нет
    Point,                 // Точка
    Intersection,          // Пересечение
    ExtendedIntersection,  // Пересечение с продолжением
    AxisAngle,             // Угол к оси
    ObjectAngle,           // Угол к объекту
    Tangent,               // Касание
    Midpoint               // Середина
};

/// Состояние отрисовки инструмента.
enum class DrawState {
    Idle,        // Ожидание
    FirstPoint,  // Задана первая точка
    SecondPoint  // Задана вторая точка
};

}  // namespace snap