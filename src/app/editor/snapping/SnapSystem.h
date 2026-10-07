#ifndef OURPAINT_APP_SNAP_SYSTEM_H_
#define OURPAINT_APP_SNAP_SYSTEM_H_
#include <cstdint>
#include <optional>
#include <span>
#include <variant>
#include <vector>


namespace snap {
struct Point {
    double x = 0.0;
    double y = 0.0;
};

using ObjectId = std::uint64_t;

struct Line {
    Point start;
    Point end;
};

struct Circle {
    Point center;
    double radius = 0.0;
};

struct Arc {
    Point center;

    double radius = 0.0;

    double startAngle = 0.0;
    double endAngle = 0.0;
};

using Geometry = std::variant<Point, Line, Circle, Arc>;

struct Object {
    ObjectId id;
    Geometry geometry;
};

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

enum class SnapType {
    None,  // Нет snap

    Point,  // Точка

    Intersection,          // Пересечение объектов
    ExtendedIntersection,  // Пересечение продолжений объектов

    // Фиксированный угол относительно
    // координатных осей.
    AxisAngle,

    // Фиксированный угол относительно
    // существующего объекта.
    ObjectAngle,

    Tangent,  // Касание / касательная

    Midpoint  // Середина отрезка
};

enum class DrawState
{
    Idle,       // Ничего не начато

    FirstPoint, // Первый клик уже сделан
    SecondPoint // Ожидается второй клик
};

struct SnapRequest {
    // Положение логического курсора
    Point cursor;

    // Все объекты сцены и их ID
    std::span<const Object> objects;

    // ID объектов, которые нельзя использовать
    std::span<const ObjectId> excludedObjects;

    // Тип активного инструмента
    ToolType tool;

    // Состояние отрисовки инструмента (1 / 2 нажатие)
    DrawState drawState;
};

struct SnapConstraint {
    // Угол привязки.
    //
    // AxisAngle:
    //     относительно координатной оси.
    //
    // ObjectAngle:
    //     относительно направления объекта.
    double angle = 0.0;

    // Объект, относительно которого
    // рассчитывается угол.
    //
    // Используется для ObjectAngle.
    ObjectId objectId{};
};

struct SnapResult {
    // Был ли найден snap
    bool snapped = false;

    // Координаты точки snap
    Point point{};

    // Вид snap
    SnapType type = SnapType::None;

    // Реальные объекты, участвующие в snap
    //
    // Point:
    //     { objectId }
    //
    // Intersection:
    //     { lineA, lineB }
    //
    // ExtendedIntersection:
    //     { lineA, lineB }
    //
    // ObjectAngle:
    //     { sourceObject }
    std::vector<ObjectId> objectIds;

    // Виртуальная геометрия:
    // продолжения, направляющие и т.д.
    std::vector<Line> guideLines;

    // Виртуальный объект для визуализации
    // результата snap.
    std::optional<Object> previewObject;

    // Дополнительные параметры snap.
    std::optional<SnapConstraint> constraint;
};


class SnapSystem {
public:
    SnapResult getSnapCandidate(
        const SnapRequest& request
    ) const;

private:
    struct SnapCandidate {
        Point point;
        double distance = 0.0;

        SnapType type = SnapType::None;

        std::vector<ObjectId> objectIds;

        std::vector<Line> guideLines;

        std::optional<Object> previewObject;

        std::optional<SnapConstraint> constraint;
    };

private:
    std::optional<SnapCandidate> findPointCandidate(
        const SnapRequest& request
    ) const;

    static bool isExcluded(
        ObjectId objectId,
        std::span<const ObjectId> excludedObjects
    );

    static SnapResult makeResult(
        const SnapCandidate& candidate
    );
};


} // namespace snap

#endif  // ! OURPAINT_APP_SNAP_SYSTEM_H_