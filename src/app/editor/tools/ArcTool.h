#ifndef OURPAINT_APPLICATION_ARC_TOOL_H_
#define OURPAINT_APPLICATION_ARC_TOOL_H_

#include "../../viewport/OverlayModel.h"
#include "../../viewport/picking/Cpu2dPicker.h"
#include "Camera2D.h"
#include "Document.h"
#include "IInteractionTool.h"

class ArcTool : public IInteractionTool {
public:
    enum class Mode {
        ThreePoints
    };

public:
    explicit ArcTool(Document& document,
                     Camera2D& camera,
                     Cpu2dPicker& picker,
                     OverlayModel& overlay);

    void setMode(Mode mode);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    glm::dvec2 screenToWorld(double x, double y) const;
    void reset();


    struct Arc {
        double cx, cy, r;
        double startAngle, endAngle;
        bool valid;
    };

    static Arc buildArcFromThreePoints(const glm::dvec2& p0, const glm::dvec2& p1, const glm::dvec2& p2);

private:
    enum class Step {
        WaitingFirstInput,
        WaitingSecondInput,
        WaitingThirdInput
    };

    Mode mode_ = Mode::ThreePoints;
    Step step_ = Step::WaitingFirstInput;

    std::vector<glm::dvec2> points_;

    Document& document_;
    Camera2D& camera_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;
};

#endif // ! OURPAINT_APPLICATION_ARC_TOOL_H_