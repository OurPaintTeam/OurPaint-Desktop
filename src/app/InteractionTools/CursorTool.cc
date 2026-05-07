#include "CursorTool.h"

#include "Document.h"
#include "Scene.h"

CursorTool::CursorTool(DocumentManager& documentManager, Camera2D& camera, Cpu2dPicker& picker, OverlayModel& overlay)
    : documentManager_(documentManager), camera_(camera), picker_(picker), overlay_(overlay) {}

//#include <iomanip>

void CursorTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});

    // std::ostringstream oss;
    // oss << std::fixed << std::setprecision(3)
    //     << std::setw(10) << v.x
    //     << " "
    //     << std::setw(10) << v.y;
    // overlay_.pos = oss.str();
    // overlay_.posX = 1.5 * e.x - 150;
    // overlay_.posY = 1.5 * -e.y + 650;

    if (input::has_flag(e.buttons, input::MouseButton::Left)) {
        if (state_ == State::DraggingSelection) {
            const std::vector<ID> ids = overlay_.selection_.model.items();
            if (!ids.empty()) {
                Scene& scene = documentManager_.getActiveDocument()->scene();
                ObjectData od = scene.getObjectData(ids[0]);
                if (ids.size() == 1 && od.et == ObjType::ET_CIRCLE) {
                    double radius = std::hypot(v.x - od.params[0], v.y - od.params[1]);
                    scene.resizeCircle(ids[0], radius);
                } else {
                    double dx = v.x - lastPos_.x;
                    double dy = v.y - lastPos_.y;
                    documentManager_.getActiveDocument()->scene().moveObjects(ids, dx, dy);
                }
                lastPos_ = v;
            }
        }
        else if (state_ == State::MarqueeSelection) {
            if (overlay_.selectionRect_.has_value()) {
                overlay_.selectionRect_->xMax = v.x;
                overlay_.selectionRect_->yMax = v.y;

                const bool shift = input::has_flag(e.modifiers, input::Modifiers::Shift);

                const std::vector<ID> pickedIds = picker_.pickInRectAtScreenLogical(
                    lastScreenPos_.x,
                    lastScreenPos_.y,
                    e.x,
                    e.y);
                if (!pickedIds.empty()) {
                    if (shift) {
                        overlay_.selection_.model.add(pickedIds);
                    } else {
                        overlay_.selection_.model.replace(pickedIds);
                    }
                } else {
                    if (!shift) {
                        overlay_.selection_.model.clear();
                    }
                }
                lastPos_ = v;
            }
        }
    }
    lastCursorWorldPos_ = v;
}

void CursorTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {

        bool shift = input::has_flag(e.modifiers, input::Modifiers::Shift);
        std::optional<PickResult> pickRes = picker_.pickAtScreenLogical(e.x, e.y);

        if (pickRes.has_value()) {
            const ID id = pickRes->id;

            if (shift) {
                overlay_.selection_.model.toggle(id);
                state_ = State::Idle;
            } else {
                if (!overlay_.selection_.model.contains(id)) {
                    overlay_.selection_.model.replace({id});
                }
                state_ = State::DraggingSelection;
                lastPos_ = camera_.screenLogicalToWorld({e.x, e.y});
            }
        } else {
            state_ = State::MarqueeSelection;

            marqueeBaseSelection_ = shift
                ? overlay_.selection_.model.items()
                : std::vector<ID>{};

            if (!shift) {
                overlay_.selection_.model.clear();
            }

            glm::dvec2 worldPos = camera_.screenLogicalToWorld({e.x, e.y});
            overlay_.selectionRect_ = OverlayModel::Rect(worldPos.x, worldPos.y, worldPos.x, worldPos.y);
            lastScreenPos_ = {e.x, e.y};
            pressWorldPos_ = worldPos;
        }

        return;
    }

    if (e.button == input::MouseButton::Left &&
        e.action == input::MouseButtonAction::Release) {
        overlay_.selectionRect_.reset();
        marqueeBaseSelection_.clear();
        state_ = State::Idle;
    }
}

void CursorTool::onKey(const input::KeyEvent& e) {
    if (e.key == input::KeyCode::Delete && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        for (const auto& id : overlay_.selection_.model.items()) {
            scene.deleteObject(id);
        }
        overlay_.selection_.model.clear();
    }
    else if (e.key == input::KeyCode::Num4 && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        std::vector<ID> ids = overlay_.selection_.model.items();
        if (ids.size() >= 2) {
            Requirement reqData;
            reqData.type = ReqType::ET_POINTONPOINT;
            reqData.obj1 = ID(ids[0].get());
            reqData.obj2 = ID(ids[1].get());
            scene.addRequirement(reqData);
        }
    }
    else if (e.key == input::KeyCode::Num8 && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        std::vector<ID> ids = overlay_.selection_.model.items();
        ID firstLine(-1);
        ID secondLine(-1);
        for (const auto id : ids) {
            ObjectData od = scene.getObjectData(id);

            if (od.et == ObjType::ET_LINE) {
                if (firstLine.get() == -1) {
                    firstLine = od.id;
                } else {
                    secondLine = od.id;
                    scene.addRequirement({ID(-1), ReqType::ET_LINELINEPARALLEL, firstLine, secondLine});
                    break;
                }
            }
        }
    }
    else if (e.key == input::KeyCode::Num9 && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        std::vector<ID> ids = overlay_.selection_.model.items();
        ID firstLine(-1);
        ID secondLine(-1);
        for (const auto id : ids) {
            ObjectData od = scene.getObjectData(id);

            if (od.et == ObjType::ET_LINE) {
                if (firstLine.get() == -1) {
                    firstLine = od.id;
                } else {
                    secondLine = od.id;
                    scene.addRequirement({ID(-1), ReqType::ET_LINELINEPERPENDICULAR, firstLine, secondLine});
                    break;
                }
            }
        }
    }
    else if (e.key == input::KeyCode::Num5 && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        std::vector<ID> ids = overlay_.selection_.model.items();
        ID line;
        std::vector<ID> points;
        for (const auto id : ids) {
            ObjectData od = scene.getObjectData(id);
            if (od.et == ObjType::ET_LINE) {
                line = od.id;
                scene.addRequirement({ID(-1), ReqType::ET_HORIZONTAL, line});
                break;
            }
            // else if (od.et == ObjType::ET_POINT) {
            //     points.push_back(od.id);
            //     if (points.size() == 2) {
            //         scene.addRequirement({ID(-1), ReqType::ET_HORIZONTAL, points[0], points[1]});
            //     }
            // }
        }
    }
    else if (e.key == input::KeyCode::Num6 && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        std::vector<ID> ids = overlay_.selection_.model.items();
        ID line;
        for (const auto id : ids) {
            ObjectData od = scene.getObjectData(id);
            if (od.et == ObjType::ET_LINE) {
                line = od.id;
                scene.addRequirement({ID(-1), ReqType::ET_VERTICAL, line});
                break;
            }
        }
    }
    else if (e.modifiers == input::Modifiers::Ctrl && e.key == input::KeyCode::C && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        data_ = scene.copyFragment(overlay_.selection_.model.items());
    }
    else if (e.modifiers == input::Modifiers::Ctrl && e.key == input::KeyCode::V && e.action == input::KeyAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        scene.pasteFragment(data_, lastCursorWorldPos_.x, lastCursorWorldPos_.y);
        state_ = State::MarqueeSelection;
    }
    else if (e.key == input::KeyCode::Enter && e.action == input::KeyAction::Press) {

    }
}

bool CursorTool::cancel() {
    //overlay_.selection_.model.clear(); // need for Dimension tool maybe
    overlay_.selectionRect_.reset();
    marqueeBaseSelection_.clear();
    state_ = State::Idle;
    return true;
}



