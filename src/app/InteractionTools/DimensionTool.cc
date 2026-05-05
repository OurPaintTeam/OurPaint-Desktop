#include "DimensionTool.h"

#include "Document.h"

DimensionTool::DimensionTool(DocumentManager& documentManager, Camera2D& camera, Cpu2dPicker& picker, OverlayModel& overlay)
    : documentManager_(documentManager), camera_(camera), picker_(picker), overlay_(overlay) {}

void DimensionTool::onMouseMove(const input::MouseMoveEvent& e) {
    (void)e;
}

void DimensionTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (!(e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press)) {
        return;
    }

    const glm::dvec2 cursor = screenToWorld(e.x, e.y);
    if (step_ == Step::WaitingFirstInput) {
        std::optional<PickResult> pickRes = picker_.pickAtScreenLogical(e.x, e.y);
        if (!pickRes.has_value()) {
            return;
        }

        if (pickRes.value().type == ObjType::ET_POINT) {
            objects_.push_back(pickRes.value().id);
            points_.push_back(cursor);
            step_ = Step::WaitingSecondInput;
        }
    }
    else if (step_ == Step::WaitingSecondInput) {
        std::optional<PickResult> pickRes = picker_.pickAtScreenLogical(e.x, e.y);
        if (!pickRes.has_value()) {
            return;
        }

        if (pickRes.value().type == ObjType::ET_POINT) {
            objects_.push_back(pickRes.value().id);
            points_.push_back(cursor);
        }

        Scene& scene = documentManager_.getActiveDocument()->scene();
        Requirement req;
        req.type = ReqType::ET_POINTPOINTDIST;
        req.obj1 = objects_[0];
        req.obj2 = objects_[1];
        req.param = dimension_;
        scene.addRequirement(req);

        reset();
    }
}

void DimensionTool::onKey(const input::KeyEvent& e) {
    (void)e;
}

bool DimensionTool::cancel() {
    return true;
}

glm::dvec2 DimensionTool::screenToWorld(double x, double y) const {
    return camera_.screenLogicalToWorld({x, y});
}

void DimensionTool::setDimension(double value) {
    dimension_ = value;

    std::vector<ID> objects = overlay_.selection_.model.items();
    if (objects.size() != 2) {
        return;
    }

    // Push
    Scene& scene = documentManager_.getActiveDocument()->scene();

    ObjectData odP1 = scene.getObjectData(objects[0]);
    ObjectData odP2 = scene.getObjectData(objects[0]);
    if (odP1.et != ObjType::ET_POINT || odP2.et != ObjType::ET_POINT) {
        return;
    }

    Requirement req;
    req.type = ReqType::ET_POINTPOINTDIST;
    req.obj1 = objects[0];
    req.obj2 = objects[1];
    req.param = dimension_;

    scene.addRequirement(req);

    reset();
}

void DimensionTool::reset() {
    points_.clear();
    objects_.clear();
    overlay_.clear();
    step_ = Step::WaitingFirstInput;
}