#include "CommandAddObject.h"

bool CommandAddObject::Execute() {
    try {
        _id = _scene.addObject(_data);
        if (_id == ID(-1)) {
            return false;
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool CommandAddObject::Undo() {
    try {
        _data = _scene.getObjectData(_id);
        return _scene.deleteObject(_id);
    } catch (...) {
        return false;
    }
}

bool CommandAddObject::Redo() {
    try {
        _id = _scene.addObject(_data, _id);
        if (_id == ID(-1)) {
            return false;
        }
    } catch (...) {
        return false;
    }
    return true;
}

