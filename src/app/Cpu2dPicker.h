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

    std::optional<PickResult> pickAt(double screenX, double screenY) const;
    std::vector<core::ID> pickInRect(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const;

    std::optional<PickResult> pickPointAt(double screenX, double screenY) const;
    std::optional<PickResult> pickLineAt(double screenX, double screenY) const;
    std::optional<PickResult> pickCircleAt(double screenX, double screenY) const;

private:
    core::Scene& scene_;
    Camera2D& camera_;
};

#endif // ! OURPAINT_APPLICATION_CPU_2D_PICKER_H_