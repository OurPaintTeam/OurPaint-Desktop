#include "UndoRedo.h"

namespace UndoRedo {
    UndoRedoManager::UndoRedoManager(unsigned int steps) : maxSteps_(steps) {}

    bool UndoRedoManager::push(Transaction&& tnx) {
        if (maxSteps_ == 0) {
            return false;
        }

        if (tnx.isBuilding()) {
            if (!tnx.commit()) {
                return false;
            };
        }

        redo_.clear();

        if (undo_.size() >= maxSteps_) {
            undo_.pop_front();
        }

        undo_.push_back(std::move(tnx));

        return true;
    }

    bool UndoRedoManager::undo() noexcept {
        if (undo_.empty()) {
            return false;
        }

        if (!undo_.back().undo()) {
            return false;
        }

        redo_.push_back(std::move(undo_.back()));
        undo_.pop_back();

        return true;
    }

    bool UndoRedoManager::redo() noexcept {
        if (redo_.empty()) {
            return false;
        }

        if (!redo_.back().redo()) {
            return false;
        }

        undo_.push_back(std::move(redo_.back()));
        redo_.pop_back();

        return true;
    }

    void UndoRedoManager::setMaxUndoSteps(unsigned int steps) {
        maxSteps_ = steps;
        while (undo_.size() > maxSteps_) {
            undo_.pop_front();
        }
    }

    bool UndoRedoManager::canUndo() const {
        return !undo_.empty();
    }

    bool UndoRedoManager::canRedo() const {
        return !redo_.empty();
    }

}