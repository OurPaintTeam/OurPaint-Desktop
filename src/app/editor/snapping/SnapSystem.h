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

// Запрос на поиск привязки.
struct SnapRequest {
    core::sketch::Vec2 cursor;
    ToolType tool;
    DrawState drawState;

    // Опорная точка, если инструмент уже задал первую точку.
    std::vector<core::sketch::Point2> anchors;
};

// Дополнительное угловое ограничение.
struct SnapConstraintAngle {
    double angle = 0.0;
    std::vector<core::sketch::GeometryRef> objects{};
};

// Результат поиска привязки.
struct SnapResult {
    bool snapped = false;
    core::sketch::Point2 point;
    SnapKind type = SnapKind::None;

    std::vector<core::sketch::GeometryRef> objects;
    std::vector<core::sketch::Line2> guideLines;
};

// Правила поиска для текущего инструмента и состояния.
struct SnapRule {
    SnapMask mask = SnapMask::None;

    // Максимальное расстояние поиска в координатах курсора.
    double maxRadius = 0.0;
};

// Кандидат до преобразования в публичный результат.
struct SnapCandidate {
    SnapResult result;
    double distanceSquared = 0.0;
    int priority = 0;
};

class SnapSystem {
private:
    const core::sketch::Sketch& sketch_;

public:
    explicit SnapSystem(const core::sketch::Sketch& sketch);
    SnapResult getSnapCandidate(const SnapRequest& request) const;

private:
    static SnapRule ruleFor(ToolType tool, DrawState state);

    SnapResult parser(const SnapRequest& request) const;

    std::optional<SnapCandidate> findBestStandalonePoint(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestLineCandidate(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestCircleCandidate(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestIntersection(const SnapRequest& request, const SnapRule& rule) const;

    static bool isAllowed(SnapKind kind, double distance, const SnapRule& rule);

    static bool isBetterCandidate(const SnapCandidate& candidate, const SnapCandidate& current);
};

}  // namespace snap

#endif  // OURPAINT_APP_SNAP_SYSTEM_H_
