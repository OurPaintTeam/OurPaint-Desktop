#ifndef OURPAINT_APPLICATION_POINT_TOOL_H_
#define OURPAINT_APPLICATION_POINT_TOOL_H_

#include "Camera2D.h"
#include "DocumentManager.h"
#include "IInteractionTool.h"

class SnapEngine;
class PointTool : public IInteractionTool {
public:
    explicit PointTool(DocumentManager& documentManager, Camera2D& camera, SnapEngine& snapEngine);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    DocumentManager& documentManager_;
    Camera2D& camera_;
    SnapEngine& snapEngine_;
};

#endif // ! OURPAINT_APPLICATION_POINT_TOOL_H_