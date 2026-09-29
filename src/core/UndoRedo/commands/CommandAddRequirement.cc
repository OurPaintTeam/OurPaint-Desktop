#include "CommandAddRequirement.h"

namespace UndoRedo {

    bool CommandAddRequirement::doExecute() noexcept {
        try {
            _reqID = _scene.addRequirement(_reqData);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool CommandAddRequirement::doUndo() noexcept {
        try {
            _reqData = _scene.getRequirementData(_reqID);
            return _scene.deleteRequirement(_reqID);
        } catch (...) {
            return false;
        }
    }

    bool CommandAddRequirement::doRedo() noexcept {
        // try {
        //     return _scene.tryRestoreRequirement(_reqData, _reqID);
        // } catch (...) {
            return false;
        // }
    }

}

