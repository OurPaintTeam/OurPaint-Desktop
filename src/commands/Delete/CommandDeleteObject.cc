#include "CommandDeleteObject.h"

bool CommandDeleteObject::Execute() {
    try {
        _reqData = _scene.getObjectRequirementsWithConnectedObjects(_id);
        _data = _scene.getRootObjectData(_id);
        return _scene.deleteObject(_id);
    } catch (...) {
        return false;
    }
}

bool CommandDeleteObject::Undo() {
    try {
        if (_scene.tryRestoreObject(_data, _data.id)) {
            for (auto& rd : _reqData) {
                if (!_scene.tryRestoreRequirement(rd, rd.id)) {
                    return false;
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}
