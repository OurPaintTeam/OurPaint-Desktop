#include "CursorTool.h"

#include "Document.h"
#include "Scene.h"
#include "glm/ext/scalar_constants.hpp"

CursorTool::CursorTool(DocumentManager& documentManager, Camera2D& camera, Cpu2dPicker& picker, OverlayModel& overlay)
    : documentManager_(documentManager), camera_(camera), picker_(picker), overlay_(overlay) {}

#include <iomanip>

void CursorTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenToWorld({e.x, e.y});

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3)
        << std::setw(10) << v.x
        << " "
        << std::setw(10) << v.y;
    overlay_.pos = oss.str();
    overlay_.posX = 1.5 * e.x - 150;
    overlay_.posY = 1.5 * -e.y + 650;

    if (input::has_flag(e.buttons, input::MouseButton::Left)) {
        if (state_ == State::DraggingSelection) {
            const std::vector<SceneObjects::ID> ids = overlay_.selection_.items();
            if (!ids.empty()) {
                double dx = v.x - lastPos_.x;
                double dy = v.y - lastPos_.y;
                documentManager_.getActiveDocument()->scene().moveObjects(ids, dx, dy);
                lastPos_ = v;
            }
        }
        else if (state_ == State::MarqueeSelection) {
            if (overlay_.rect_.has_value()) {
                overlay_.rect_->xMax = v.x;
                overlay_.rect_->yMax = v.y;

                const bool shift = input::has_flag(e.modifiers, input::Modifiers::Shift);

                const std::vector<SceneObjects::ID> pickedIds = picker_.pickInRect(
                    lastScreenPos_.x,
                    lastScreenPos_.y,
                    e.x,
                    e.y);
                if (!pickedIds.empty()) {
                    if (shift) {
                        overlay_.selection_.add(pickedIds);
                    } else {
                        overlay_.selection_.replace(pickedIds);
                    }
                } else {
                    if (!shift) {
                        overlay_.selection_.clear();
                    }
                }
                lastPos_ = v;
            }
        }
    }
    lastCursorPos_ = v;
}

void CursorTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {

        bool shift = input::has_flag(e.modifiers, input::Modifiers::Shift);
        std::optional<PickResult> pickRes = picker_.pickAt(e.x, e.y);

        if (pickRes.has_value()) {
            const SceneObjects::ID id = pickRes->id;

            if (shift) {
                overlay_.selection_.toggle(id);
                state_ = State::Idle;
            } else {
                if (!overlay_.selection_.contains(id)) {
                    overlay_.selection_.replace({id});
                }
                state_ = State::DraggingSelection;
                lastPos_ = camera_.screenToWorld({e.x, e.y});
            }
        } else {
            state_ = State::MarqueeSelection;

            marqueeBaseSelection_ = shift
                ? overlay_.selection_.items()
                : std::vector<SceneObjects::ID>{};

            if (!shift) {
                overlay_.selection_.clear();
            }

            glm::dvec2 worldPos = camera_.screenToWorld({e.x, e.y});
            overlay_.rect_ = OverlayModel::Rect(worldPos.x, worldPos.y, worldPos.x, worldPos.y);
            lastScreenPos_ = {e.x, e.y};
            pressWorldPos_ = worldPos;
        }

        return;
    }

    if (e.button == input::MouseButton::Left &&
        e.action == input::MouseButtonAction::Release) {
        overlay_.rect_.reset();
        marqueeBaseSelection_.clear();
        state_ = State::Idle;
    }
}

void CursorTool::onKey(const input::KeyEvent& e) {
    if (e.key == input::KeyCode::Delete && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        for (const auto& id : overlay_.selection_.items()) {
            scene.deleteObject(id);
        }
        overlay_.selection_.clear();
    }
    else if (e.key == input::KeyCode::Num4 && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        std::vector<SceneObjects::ID> ids = overlay_.selection_.items();
        if (ids.size() >= 2) {
            Requirement reqData;
            reqData.type = ReqType::ET_POINTONPOINT;
            reqData.obj1 = SceneObjects::ID(ids[0].get());
            reqData.obj2 = SceneObjects::ID(ids[1].get());
            scene.addRequirement(reqData);
        }
    }
    else if (e.modifiers == input::Modifiers::Ctrl && e.key == input::KeyCode::C && e.action == input::KeyAction::Press) {
        copiedObjects_ = overlay_.selection_.items();

        Scene& scene = documentManager_.getActiveDocument()->scene();

        bool first = true;

        double minX, maxX, minY, maxY;

        for (const auto id : copiedObjects_) {
            ObjectData od = scene.getObjectData(id);
            if (od.et == ObjType::ET_POINT) {
                double x = od.params[0];
                double y = od.params[1];

                if (first) {
                    minX = maxX = x;
                    minY = maxY = y;
                    first = false;
                } else {
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                    minY = std::min(minY, y);
                    maxY = std::max(maxY, y);
                }
            }
        }
        double midX = minX + (maxX - minX) / 2;
        double midY = minY + (maxY - minY) / 2;
        copiedPos_ = {midX, midY};
    }
    else if (e.modifiers == input::Modifiers::Ctrl && e.key == input::KeyCode::V && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        std::vector<SceneObjects::ID> copiedObjectsNewIDs;
        for (const auto id : copiedObjects_) {
            ObjectData od = scene.getObjectData(id);
            copiedObjectsNewIDs.push_back(scene.addObject(od));
        }
        scene.moveObjects(copiedObjectsNewIDs, lastCursorPos_.x - copiedPos_.x, lastCursorPos_.y - copiedPos_.y);
        overlay_.selection_.replace(copiedObjectsNewIDs);
        state_ = State::MarqueeSelection;
    }
    else if (e.key == input::KeyCode::Enter && e.action == input::KeyAction::Press) {
    }
}

bool CursorTool::cancel() {
    overlay_.selection_.clear();
    overlay_.rect_.reset();
    marqueeBaseSelection_.clear();
    state_ = State::Idle;
    return true;
}



