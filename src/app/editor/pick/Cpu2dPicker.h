#ifndef OURPAINT_APPLICATION_CPU_2D_PICKER_H_
#define OURPAINT_APPLICATION_CPU_2D_PICKER_H_

#include <optional>
#include <variant>
#include <vector>

#include "../../../render/Camera2D.h"
#include "../RectSelectionMode.h"
#include "Sketch.h"

using namespace core;

namespace app {
class ConstraintLayout;
}

struct GeometryPickResult {
    sketch::GeometryRef ref;
};

struct DimensionPickResult {
    enum class Part { Label, Body };
    sketch::ConstraintId constraint;
    Part part;
    glm::dvec2 labelCenter;
    double distancePx;
};

using PickResult = std::variant<GeometryPickResult, DimensionPickResult>;

class Cpu2dPicker {
public:
    explicit Cpu2dPicker(sketch::Sketch& sketch, render::Camera2D& camera);

    // A cursor hit can be a dimension label, a sketch point, a dimension body or a curve, in that order.
    std::optional<PickResult> pickAtScreenLogical(double screenX, double screenY, const app::ConstraintLayout& layout) const;
    // Only the winning dimension, for hover feedback without scanning unrelated curves.
    std::optional<DimensionPickResult> pickSelectableDimensionAtScreenLogical(double screenX, double screenY, const app::ConstraintLayout& layout) const;
    std::optional<DimensionPickResult> pickDimensionAtScreenLogical(double screenX, double screenY, const app::ConstraintLayout& layout) const;

    std::vector<sketch::GeometryRef> pickInRectAtScreenLogical(double x1, double y1, double x2, double y2,
                                                              app::RectSelectionMode mode = app::RectSelectionMode::Crossing) const;
    std::vector<sketch::GeometryRef> pickInRectAtScreenFramebuffer(double x1, double y1, double x2, double y2,
                                                                  app::RectSelectionMode mode = app::RectSelectionMode::Crossing) const;
    std::vector<sketch::GeometryRef> pickInRectAtWorld(double x1, double y1, double x2, double y2,
                                                      app::RectSelectionMode mode = app::RectSelectionMode::Crossing) const;

    std::optional<GeometryPickResult> pickGeometryAtScreenLogical(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickCurveAtScreenLogical(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickPointAtScreenLogical(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickLineAtScreenLogical(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickCircleAtScreenLogical(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickArcAtScreenLogical(double screenX, double screenY) const;

    std::optional<GeometryPickResult> pickAtScreenFramebuffer(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickPointAtScreenFramebuffer(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickLineAtScreenFramebuffer(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickCircleAtScreenFramebuffer(double screenX, double screenY) const;
    std::optional<GeometryPickResult> pickArcAtScreenFramebuffer(double screenX, double screenY) const;

    std::optional<GeometryPickResult> pickAtWorld(double worldX, double worldY) const;
    std::optional<GeometryPickResult> pickPointAtWorld(double worldX, double worldY) const;
    std::optional<GeometryPickResult> pickLineAtWorld(double worldX, double worldY) const;
    std::optional<GeometryPickResult> pickCircleAtWorld(double worldX, double worldY) const;
    std::optional<GeometryPickResult> pickArcAtWorld(double worldX, double worldY) const;

private:
    std::optional<GeometryPickResult> pickAt(double worldX, double worldY) const;
    std::optional<GeometryPickResult> pickPointAt(double worldX, double worldY) const;
    std::optional<GeometryPickResult> pickLineAt(double worldX, double worldY) const;
    std::optional<GeometryPickResult> pickCircleAt(double worldX, double worldY) const;
    std::optional<GeometryPickResult> pickArcAt(double worldX, double worldY) const;

    static bool lineIntersectsRectFast(double x1, double y1, double x2, double y2, double xMin, double yMin, double xMax, double yMax);

    sketch::Sketch& sketch_;
    render::Camera2D& camera_;
};

#endif // ! OURPAINT_APPLICATION_CPU_2D_PICKER_H_
