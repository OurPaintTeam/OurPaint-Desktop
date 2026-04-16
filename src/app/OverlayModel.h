#ifndef OURPAINT_APPLICATION_OVERLAY_MODEL_H_
#define OURPAINT_APPLICATION_OVERLAY_MODEL_H_

#include "SelectionModel.h"
#include <optional>

class OverlayModel {
public:
    struct Rect {
        float xMin;
        float yMin;
        float xMax;
        float yMax;
    };

    SelectionModel selection_;
    std::optional<Rect> rect_;
};

#endif // ! OURPAINT_APPLICATION_OVERLAY_MODEL_H_