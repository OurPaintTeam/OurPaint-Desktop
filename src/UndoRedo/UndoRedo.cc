#include "UndoRedo.h"

UndoRedo::UndoRedo(unsigned int steps) : maxSteps(steps) {}

void UndoRedo::push(Transaction &&tnx) {
    if (!tnx.isCommitted()) {
        throw std::invalid_argument("Transaction must be committed");
    }
    if (transactions_undo.size() >= maxSteps) {
        transactions_undo.pop();
    }
    transactions_undo.push(std::move(tnx));
    while (!transactions_redo.empty()) {
        transactions_redo.pop();
    }
}


bool UndoRedo::undo() {
    if (!transactions_undo.empty()) {
        Transaction txn = std::move(transactions_undo.top());
        transactions_undo.pop();
        txn.undo();
        transactions_redo.push(std::move(txn));
        return true;
    }
    return false;
}

bool UndoRedo::redo() {
    if (!transactions_redo.empty()) {
        Transaction txn = std::move(transactions_redo.top());
        transactions_redo.pop();
        txn.redo();
        transactions_undo.push(std::move(txn));
        return true;
    }
    return false;
}

void UndoRedo::setMaxUndoSteps(unsigned int steps) {
    maxSteps = steps;
    while (transactions_undo.size() > maxSteps) {
        transactions_undo.pop();
    }
}

bool UndoRedo::canUndo() const {
    return !transactions_undo.empty();
}

bool UndoRedo::canRedo() const {
    return !transactions_redo.empty();
}