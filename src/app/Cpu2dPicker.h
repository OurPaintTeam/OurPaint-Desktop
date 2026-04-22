#ifndef OURPAINT_APPLICATION_CPU_2D_PICKER_H_
#define OURPAINT_APPLICATION_CPU_2D_PICKER_H_

#include <optional>
#include <vector>
#include "Scene.h"
#include "Camera2D.h"

struct PickResult {
    ID id;
    ObjType type;
};

class Cpu2dPicker {
public:
    explicit Cpu2dPicker(Scene& scene, Camera2D& camera);

    std::optional<PickResult> pickAt(double screenX, double screenY) const;
    std::vector<ID> pickInRect(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const;

private:
    Scene& scene_;
    Camera2D& camera_;
};

#endif // ! OURPAINT_APPLICATION_CPU_2D_PICKER_H_