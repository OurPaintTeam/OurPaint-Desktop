#ifndef OURPAINT_APPLICATION_POINT_TOOL_H_
#define OURPAINT_APPLICATION_POINT_TOOL_H_

#include "DocumentManager.h"
#include "IInteractionTool.h"
#include "Camera2D.h"

class PointTool : public IInteractionTool {
public:
    explicit PointTool(DocumentManager& documentManager_, Camera2D& camera);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    void onCancel() override;

private:
    DocumentManager& documentManager_;
    Camera2D& camera_;
};

#endif // ! OURPAINT_APPLICATION_POINT_TOOL_H_