#ifndef OURPAINT_APPLICATION_POINT_TOOL_H_
#define OURPAINT_APPLICATION_POINT_TOOL_H_

#include "Camera2D.h"
#include "DocumentManager.h"
#include "IInteractionTool.h"

class PointTool : public IInteractionTool {
public:
    explicit PointTool(Document& document, Camera2D& camera);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    Document& document_;
    Camera2D& camera_;
};

#endif // ! OURPAINT_APPLICATION_POINT_TOOL_H_