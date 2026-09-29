#include "CommandDeleteRequirement.h"

namespace UndoRedo {

    bool CommandDeleteRequirement::doExecute() noexcept {
        try {
            reqData_ = scene_.getRequirementData(reqID_);
            return scene_.deleteRequirement(reqID_);
        } catch (...) {
            return false;
        }
    }

    bool CommandDeleteRequirement::doUndo() noexcept {
        // try {
        //     return scene_.tryRestoreRequirement(reqData_, reqID_);
        // } catch (...) {
            return false;
        //}
    }

    bool CommandDeleteRequirement::doRedo() noexcept {
        try {
            reqData_ = scene_.getRequirementData(reqID_);
            return scene_.deleteRequirement(reqID_);
        } catch (...) {
            return false;
        }
    }

}
