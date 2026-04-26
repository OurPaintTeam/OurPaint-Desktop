#ifndef OURPAINT_APPLICATION_CIRCLE_TOOL_H_
#define OURPAINT_APPLICATION_CIRCLE_TOOL_H_

#include <vector>
#include <optional>

#include <glm/glm.hpp>

#include "DocumentManager.h"
#include "Camera2D.h"
#include "IInteractionTool.h"
#include "objects/Objects.h"
#include "../Cpu2dPicker.h"
#include "../OverlayModel.h"

class CircleTool : public IInteractionTool {
public:
    enum class Mode {
        CenterRadius,
        CenterDiameter,
        DiameterTwoPoints,
        ThreePoints,
        TangentTwoObjectsRadius,
        TangentThreeObjects
    };

public:
    explicit CircleTool(DocumentManager& documentManager,
                        Camera2D& camera,
                        Cpu2dPicker& picker,
                        OverlayModel& overlay);

    void setMode(Mode mode);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    enum class Step {
        WaitingFirstInput,
        WaitingSecondInput,
        WaitingThirdInput
    };

private:
    glm::dvec2 screenToWorld(double x, double y) const;
    void reset();

    struct Circle {
        double cx;
        double cy;
        double r;
    };
    static Circle buildCircleFromThreePoints(const glm::dvec2& p0, const glm::dvec2& p1, const glm::dvec2& p2);
    void pushCircleToModel(const Circle& c) const;



private:
    Mode mode_ = Mode::CenterRadius;
    Step step_ = Step::WaitingFirstInput;

    std::vector<glm::dvec2> points_;
    std::vector<ID> objects_;

    DocumentManager& documentManager_;
    Camera2D& camera_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;
};

#endif // ! OURPAINT_APPLICATION_CIRCLE_TOOL_H_