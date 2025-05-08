#include "UndoRedo.h"

UndoRedo::UndoRedo(unsigned int steps) : maxSteps(steps) {}

void UndoRedo::push(Transaction &&tnx) {
    if (!tnx.isCommitted()) {
        tnx.commit();
    }
    if (transactions_undo.size() >= maxSteps) {
        transactions_undo.pop_front();
    }
    transactions_undo.push_back(std::move(tnx));
    transactions_redo = std::stack<Transaction>();
}


bool UndoRedo::undo() {
    if (transactions_undo.empty()) {
        return false;
    }
    Transaction& txn_ref = transactions_undo.back();
    if (txn_ref.undo()) {
        Transaction txn = std::move(txn_ref);
        transactions_undo.pop_back();
        transactions_redo.push(std::move(txn));
        return true;
    }
    return false;
}

bool UndoRedo::redo() {
    if (transactions_redo.empty()) {
        return false;
    }
    Transaction& txn_ref = transactions_redo.top();
    if (txn_ref.redo()) {
        Transaction txn = std::move(txn_ref);
        transactions_redo.pop();
        transactions_undo.push_back(std::move(txn));
        return true;
    }
    return false;
}

void UndoRedo::setMaxUndoSteps(unsigned int steps) {
    maxSteps = steps;
    while (transactions_undo.size() > maxSteps) {
        transactions_undo.pop_front();
    }
}

bool UndoRedo::canUndo() const {
    return !transactions_undo.empty();
}

bool UndoRedo::canRedo() const {
    return !transactions_redo.empty();
}