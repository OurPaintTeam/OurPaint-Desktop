#ifndef OURPAINT_APPLICATION_CPU_2D_PICKER_H_
#define OURPAINT_APPLICATION_CPU_2D_PICKER_H_

#include <optional>
#include <vector>
#include "Scene.h"
#include "Camera2D.h"

struct PickResult {
    core::ID id;
    core::ObjType type;
};

class Cpu2dPicker {
public:
    explicit Cpu2dPicker(core::Scene& scene, Camera2D& camera);

    std::vector<core::ID> pickInRectAtScreenLogical(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const;
    std::vector<core::ID> pickInRectAtScreenFramebuffer(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const;
    std::vector<core::ID> pickInRectAtWorld(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const;

    std::optional<PickResult> pickAtScreenLogical(double screenX, double screenY) const;
    std::optional<PickResult> pickPointAtScreenLogical(double screenX, double screenY) const;
    std::optional<PickResult> pickLineAtScreenLogical(double screenX, double screenY) const;
    std::optional<PickResult> pickCircleAtScreenLogical(double screenX, double screenY) const;

    std::optional<PickResult> pickAtScreenFramebuffer(double screenX, double screenY) const;
    std::optional<PickResult> pickPointAtScreenFramebuffer(double screenX, double screenY) const;
    std::optional<PickResult> pickLineAtScreenFramebuffer(double screenX, double screenY) const;
    std::optional<PickResult> pickCircleAtScreenFramebuffer(double screenX, double screenY) const;

    std::optional<PickResult> pickAtWorld(double worldX, double worldY) const;
    std::optional<PickResult> pickPointAtWorld(double worldX, double worldY) const;
    std::optional<PickResult> pickLineAtWorld(double worldX, double worldY) const;
    std::optional<PickResult> pickCircleAtWorld(double worldX, double worldY) const;

private:
    std::optional<PickResult> pickAt(double worldX, double worldY) const;
    std::optional<PickResult> pickPointAt(double worldX, double worldY) const;
    std::optional<PickResult> pickLineAt(double worldX, double worldY) const;
    std::optional<PickResult> pickCircleAt(double worldX, double worldY) const;

    std::vector<core::ID> pickInRect(double worldMinX, double worldMinY, double worldMaxX, double worldMaxY) const;

    static bool lineIntersectsRectFast(double x1, double y1, double x2, double y2,
                            double xMin, double yMin, double xMax, double yMax);

    core::Scene& scene_;
    Camera2D& camera_;
};

#endif // ! OURPAINT_APPLICATION_CPU_2D_PICKER_H_