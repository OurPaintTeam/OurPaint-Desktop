#include "UndoRedo.h"

namespace UndoRedo {

    UndoRedoManager::UndoRedoManager(unsigned int steps) : maxSteps(steps) {}

    void UndoRedoManager::push(Transaction &&tnx) {
        if (!tnx.isCommitted()) {
            tnx.commit();
        }
        if (transactions_undo.size() >= maxSteps) {
            transactions_undo.pop_front();
        }
        transactions_undo.push_back(std::move(tnx));
        transactions_redo = std::stack<Transaction>();
    }


    bool UndoRedoManager::undo() {
        if (transactions_undo.empty()) {
            return false;
        }
        Transaction &txn_ref = transactions_undo.back();
        if (txn_ref.undo()) {
            Transaction txn = std::move(txn_ref);
            transactions_undo.pop_back();
            transactions_redo.push(std::move(txn));
            return true;
        }
        return false;
    }

    bool UndoRedoManager::redo() {
        if (transactions_redo.empty()) {
            return false;
        }
        Transaction &txn_ref = transactions_redo.top();
        if (txn_ref.redo()) {
            Transaction txn = std::move(txn_ref);
            transactions_redo.pop();
            transactions_undo.push_back(std::move(txn));
            return true;
        }
        return false;
    }

    void UndoRedoManager::setMaxUndoSteps(unsigned int steps) {
        maxSteps = steps;
        while (transactions_undo.size() > maxSteps) {
            transactions_undo.pop_front();
        }
    }

    bool UndoRedoManager::canUndo() const {
        return !transactions_undo.empty();
    }

    bool UndoRedoManager::canRedo() const {
        return !transactions_redo.empty();
    }

}