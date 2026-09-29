#include "CommandAddObject.h"

namespace UndoRedo {

    bool CommandAddObject::doExecute() noexcept {
        try {
            id_ = scene_.addObject(data_);
            if (id_ == ID(-1)) {
                return false;
            }
        } catch (...) {
            return false;
        }
        return true;
    }

    bool CommandAddObject::doUndo() noexcept {
        try {
            data_ = scene_.getObjectData(id_);
            return scene_.deleteObject(id_);
        } catch (...) {
            return false;
        }
    }

    bool CommandAddObject::doRedo() noexcept {
        id_ = scene_.addObject(data_);
        // try {
        //     return _scene.tryRestoreObject(_data, _id);
        // } catch (...) {
            return false;
        // }
    }

}

