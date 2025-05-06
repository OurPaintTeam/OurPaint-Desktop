#include "CommandDeleteObject.h"

bool CommandDeleteObject::Execute() {
    try {
        _data = _scene.getObjectData(_id);
        return _scene.deleteObject(_id);
    } catch (...) {
        return false;
    }
}

bool CommandDeleteObject::Undo() {
    try {
        _id = _scene.addObject(_data, _id);
        return true;
    } catch (...) {
        return false;
    }
}

bool CommandDeleteObject::Redo() {
    try {
        _data = _scene.getObjectData(_id);
        return _scene.deleteObject(_id);
    } catch (...) {
        return false;
    }
}

