#include "CommandMove.h"

bool UndoRedo::CommandMove::Execute() {
    const Component& c = _scene.findComponentByID(_pre_move_object_states.at(0).id);
    for (auto& id : c.objectIDs()) {
        _post_move_object_states.push_back(_scene.getObjectData(id));
    }
    return true;
}

bool UndoRedo::CommandMove::Undo() {
    for (auto& objData : _pre_move_object_states) {
        if (objData.et == Element::ET_POINT) {
            _scene.setPoint(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    false
                    );
        }
        else if (objData.et == Element::ET_SECTION) {
            _scene.setSection(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    objData.params[3],
                    false
                    );
        }
        else if (objData.et == Element::ET_CIRCLE) {
            _scene.setCircle(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    false
                    );
        }
        else if (objData.et == Element::ET_ARC) {
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

bool UndoRedo::CommandMove::Redo() {
    for (auto& objData : _post_move_object_states) {
        if (objData.et == Element::ET_POINT) {
            _scene.setPoint(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    false
            );
        }
        else if (objData.et == Element::ET_SECTION) {
            _scene.setSection(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    objData.params[3],
                    false
            );
        }
        else if (objData.et == Element::ET_CIRCLE) {
            _scene.setCircle(
                    objData.id,
                    objData.params[0],
                    objData.params[1],
                    objData.params[2],
                    false
            );
        }
        else if (objData.et == Element::ET_ARC) {
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
