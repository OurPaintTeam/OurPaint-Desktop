#ifndef OURPAINT_APPLICATION_OVERLAY_MODEL_H_
#define OURPAINT_APPLICATION_OVERLAY_MODEL_H_

#include "SelectionModel.h"
#include <optional>
#include <vector>
#include <string>

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
    std::vector<std::string> texts_;

    std::string pos;
    int posX{};
    int posY{};

    std::string lp;
    int lpX{};
    int lpY{};
};

#endif // ! OURPAINT_APPLICATION_OVERLAY_MODEL_H_