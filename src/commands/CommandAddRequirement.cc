#include "CommandAddRequirement.h"

bool CommandAddRequirement::Execute() {
    try {
        _reqID = _scene.addRequirement(_reqData);
        return true;
    } catch (...) {
        return false;
    }
}

bool CommandAddRequirement::Undo() {
    try {
        _reqData = _scene.getRequirementData(_reqID);
        return _scene.deleteRequirement(_reqID);
    } catch (...) {
        return false;
    }
}

bool CommandAddRequirement::Redo() {
    try {
        return _scene.tryRestoreRequirement(_reqData, _reqID);
    } catch (...) {
        return false;
    }
}




