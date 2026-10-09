#ifndef OURPAINT_APP_SNAP_SYSTEM_H_
#define OURPAINT_APP_SNAP_SYSTEM_H_

#include <optional>
#include <span>
#include <vector>

#include "SnapTypes.h"
#include "objects/GeometricObjects.h"
#include "sketch/SketchTypes.h"

namespace core::sketch {
class Sketch;
}

namespace snap {

/// Запрос на поиск позиционной привязки.
struct SnapRequest {
    // Положение логического курсора.
    core::sketch::Vec2 cursor;

    // Тип активного инструмента.
    ToolType tool;

    // Состояние отрисовки инструмента.
    DrawState drawState;
};

/// Дополнительное ограничение, накладываемое привязкой.
struct SnapConstraint {
    // Угол привязки:
    // AxisAngle   — относительно координатной оси;
    // ObjectAngle — относительно направления объекта.
    double angle = 0.0;

    // Объекты, задающие направление угла.
    std::vector<core::sketch::GeometryRef> objectIds{};
};

/// Результат поиска позиционной привязки.
struct SnapResult {
    // Был ли найден snap.
    bool snapped = false;

    // Расстояние от курсора до точки snap (для UI и отладки).
    double distance = 0.0;

    // Оценка кандидата для выбора лучшего.
    // Пока совпадает с distance; далее — с учётом типа и приоритетов.
    double score = 0.0;

    // Координаты точки привязки.
    Point point{};

    // Тип найденного snap.
    SnapType type = SnapType::None;

    // Реальные объекты Sketch, участвующие в snap:
    // Point        — { pointId }
    // Intersection — { lineAId, lineBId }
    // Tangent      — { circleId, lineId }
    // ObjectAngle  — { objectId }
    std::vector<core::sketch::GeometryRef> objectIds;

    // Виртуальная геометрия для визуализации привязки
    // (продолжения, направляющие, перпендикуляры).
    // НЕ объекты Sketch.
    std::vector<core::sketch::SketchGeometry> guideLines;

    // Виртуальный объект предпросмотра результата.
    std::optional<core::sketch::SketchGeometry> previewObject;

    // Дополнительные параметры snap.
    std::optional<SnapConstraint> constraint;
};

/// Система поиска позиционных привязок.
class SnapSystem {
public:
    explicit SnapSystem(const core::sketch::Sketch& sketch) : sketch_(sketch) {}

    SnapResult getSnapCandidate(const SnapRequest& request) const;

private:
    const core::sketch::Sketch& sketch_;

    // Поиск ближайшей самостоятельной точки.
    std::optional<SnapResult> findPointCandidate(const SnapRequest& request) const;

    // Поиск ближайшей середины отрезка.
    std::optional<SnapResult> findMidpointCandidate(const SnapRequest& request) const;

    // Поиск пересечения двух отрезков.
    std::optional<SnapResult> findIntersectionCandidate(const SnapRequest& request) const;

    // Поиск пересечения продолжений двух отрезков.
    std::optional<SnapResult> findExtendedIntersectionCandidate(const SnapRequest& request) const;
};

}  // namespace snap

#endif  // OURPAINT_APP_SNAP_SYSTEM_H_