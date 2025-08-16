#include "CommandDeleteRequirement.h"

namespace UndoRedo {

    bool CommandDeleteRequirement::Execute() {
        try {
            _reqData = _scene.getRequirementData(_reqID);
            return _scene.deleteRequirement(_reqID);
        } catch (...) {
            return false;
        }
    }

    bool CommandDeleteRequirement::Undo() {
        try {
            return _scene.tryRestoreRequirement(_reqData, _reqID);
        } catch (...) {
            return false;
        }
    }

    bool CommandDeleteRequirement::Redo() {
        try {
            _reqData = _scene.getRequirementData(_reqID);
            return _scene.deleteRequirement(_reqID);
        } catch (...) {
            return false;
        }
    }

}
