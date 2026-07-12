#include "DimensionTool.h"

#include "Document.h"

DimensionTool::DimensionTool(Document& document, Camera2D& camera, Cpu2dPicker& picker, OverlayModel& overlay)
    : document_(document), camera_(camera), picker_(picker), overlay_(overlay) {}

void DimensionTool::onMouseMove(const input::MouseMoveEvent& e) {
    (void)e;
}

void DimensionTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (!(e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press)) {
        return;
    }

    std::optional<PickResult> pickRes = picker_.pickAtScreenLogical(e.x, e.y);
    if (!pickRes.has_value()) {
        return;
    }

    ID pickedId = pickRes->id;

    if (pickRes->type == ObjType::ET_LINE) {
        tryApplyDimensionToObject(pickedId);
        return;
    }

    if (pickRes->type != ObjType::ET_POINT) {
        return;
    }

    if (step_ == Step::WaitingFirstInput) {
        objects_.push_back(pickedId);
        step_ = Step::WaitingSecondInput;
        return;
    }

    if (step_ == Step::WaitingSecondInput) {
        tryApplyDimensionToTwoObjects(objects_[0], pickedId);
        return;
    }
}

void DimensionTool::onKey(const input::KeyEvent& e) {
    (void)e;
}

bool DimensionTool::cancel() {
    reset();
    return true;
}

glm::dvec2 DimensionTool::screenToWorld(double x, double y) const {
    return camera_.screenLogicalToWorld({x, y});
}

void DimensionTool::setDimension(double value) {
    dimension_ = value;

    const std::vector<ID> objects = overlay_.selection_.model.items();

    if (objects.size() == 1) {
        tryApplyDimensionToObject(objects[0]);
        return;
    }

    if (objects.size() == 2) {
        tryApplyDimensionToTwoObjects(objects[0], objects[1]);
        return;
    }
}

bool DimensionTool::tryApplyDimensionToObject(ID id) {
    Scene& scene = document_.scene();
    ObjectData line = scene.getObjectData(id);

    if (line.et != ObjType::ET_LINE) {
        return false;
    }

    if (line.subObjects.size() < 2) {
        return false;
    }

    ID p1 = line.subObjects[0];
    ID p2 = line.subObjects[1];

    return tryApplyDimensionToTwoObjects(p1, p2);
}

bool DimensionTool::tryApplyDimensionToTwoObjects(ID id1, ID id2) {
    if (id1 == id2) {
        return false;
    }

    Scene& scene = document_.scene();

    ObjectData od1 = scene.getObjectData(id1);
    ObjectData od2 = scene.getObjectData(id2);

    if (od1.et != ObjType::ET_POINT || od2.et != ObjType::ET_POINT) {
        return false;
    }

    Requirement req;
    req.type = ReqType::ET_POINTPOINTDIST;
    req.obj1 = id1;
    req.obj2 = id2;
    req.param = dimension_;

    scene.addRequirement(req);
    reset();
    return true;
}

void DimensionTool::reset() {
    points_.clear();
    objects_.clear();
    overlay_.clear();
    step_ = Step::WaitingFirstInput;
}