#include "CommandMove.h"
#include "objects/Component.h"
#include "Scene.h"

bool UndoRedo::CommandMove::doExecute() noexcept {
    return false;
}

bool UndoRedo::CommandMove::doUndo() noexcept {
    for (auto& objData : _pre_move_object_states) {
        if (objData.et == core::ObjType::ET_POINT) {
            _scene.setPoint(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    false
                    );
        }
        else if (objData.et == core::ObjType::ET_LINE) {
            _scene.setSection(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    objData.params[3],
                    false
                    );
        }
        else if (objData.et == core::ObjType::ET_CIRCLE) {
            _scene.setCircle(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    false
                    );
        }
        else if (objData.et == core::ObjType::ET_ARC) {
            _scene.setArc(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    objData.params[3],
                    objData.params[4],
                    objData.params[5],
                    objData.params[6],
                    false
                    );
        }
        else {
            return false;
        }
    }
    _scene.updateRequirements(_pre_move_object_states[0].id);
    return true;
}

bool UndoRedo::CommandMove::doRedo() noexcept {
    for (auto& objData : _post_move_object_states) {
        if (objData.et == core::ObjType::ET_POINT) {
            _scene.setPoint(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    false
            );
        }
        else if (objData.et == core::ObjType::ET_LINE) {
            _scene.setSection(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    objData.params[3],
                    false
            );
        }
        else if (objData.et == core::ObjType::ET_CIRCLE) {
            _scene.setCircle(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    false
            );
        }
        else if (objData.et == core::ObjType::ET_ARC) {
            _scene.setArc(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    objData.params[3],
                    objData.params[4],
                    objData.params[5],
                    objData.params[6],
                    false
            );
        }
        else {
            return false;
        }
    }
    _scene.updateRequirements(_post_move_object_states[0].id);
    return true;
}
