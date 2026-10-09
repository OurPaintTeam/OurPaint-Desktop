#ifndef OURPAINT_APPLICATION_CIRCLE_TOOL_H_
#define OURPAINT_APPLICATION_CIRCLE_TOOL_H_

#include <glm/glm.hpp>
#include <optional>
#include <vector>

#include "../../../core/Document.h"
#include "../../viewport/OverlayModel.h"
#include "../../viewport/picking/Cpu2dPicker.h"
#include "Camera2D.h"
#include "IInteractionTool.h"

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
    explicit CircleTool(Document& document,
                        Camera2D& camera,
                        Cpu2dPicker& picker,
                        OverlayModel& overlay);

    void setMode(Mode mode);

    std::optional<ActionReport> onMouseMove(const input::MouseMoveEvent& e) override;
    std::optional<ActionReport> onMouseButton(const input::MouseButtonEvent& e) override;
    std::optional<ActionReport> onKey(const input::KeyEvent& e) override;
    ToolCancellation cancel() override;

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
    static std::optional<Circle> buildCircleFromThreePoints(const glm::dvec2& p0, const glm::dvec2& p1, const glm::dvec2& p2);
    ActionReport pushCircleToModel(const Circle& c) const;



private:
    Mode mode_ = Mode::CenterRadius;
    Step step_ = Step::WaitingFirstInput;

    std::vector<glm::dvec2> points_;
    std::vector<sketch::EntityId> objects_;

    Document& document_;
    Camera2D& camera_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;
};

#endif // ! OURPAINT_APPLICATION_CIRCLE_TOOL_H_
