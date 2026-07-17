#ifndef OURPAINT_APPLICATION_DIMENSION_TOOL_H_
#define OURPAINT_APPLICATION_DIMENSION_TOOL_H_

#include "../../viewport/OverlayModel.h"
#include "../../viewport/picking/Cpu2dPicker.h"
#include "IInteractionTool.h"
#include "Camera2D.h"
#include "Document.h"
#include "objects/Objects.h"

class DimensionTool : public IInteractionTool {
public:
    explicit DimensionTool(Document& document,
                        Camera2D& camera,
                        Cpu2dPicker& picker,
                        OverlayModel& overlay);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

    void setDimension(double value);

    bool tryApplyDimensionToObject(ID id);
    bool tryApplyDimensionToTwoObjects(ID id1, ID id2);

private:
    enum class Step {
        WaitingFirstInput,
        WaitingSecondInput,
        WaitingThirdInput
    };

    glm::dvec2 screenToWorld(double x, double y) const;
    void reset();

private:
    Document& document_;
    Camera2D& camera_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;

    std::vector<glm::dvec2> points_;
    std::vector<ID> objects_;

    glm::dvec2 lastPos_{};

    Step step_ = Step::WaitingFirstInput;

    double dimension_ = 1.0;
};

#endif // ! OURPAINT_APPLICATION_DIMENSION_TOOL_H_