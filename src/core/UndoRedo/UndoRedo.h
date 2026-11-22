#ifndef OURPAINT_HEADERS_UNDOREDO_UNDOREDOMANAGER_H_
#define OURPAINT_HEADERS_UNDOREDO_UNDOREDOMANAGER_H_

#include <stack>
#include <deque>

#include "Transaction.h"

namespace UndoRedo {

class UndoRedoManager {
private:
    unsigned int maxSteps_;
    std::deque<Transaction> undo_;
    std::deque<Transaction> redo_;

public:
    UndoRedoManager(unsigned int maxSteps = 100);

    UndoRedoManager(const UndoRedoManager&) = delete;
    UndoRedoManager(UndoRedoManager&&) = delete;
    UndoRedoManager& operator=(const UndoRedoManager&) = delete;
    UndoRedoManager& operator=(UndoRedoManager&&) = delete;

    bool push(Transaction&& txn);
    bool undo() noexcept;
    bool redo() noexcept;

    void setMaxUndoSteps(unsigned int);

    bool canUndo() const;

    bool canRedo() const;
};

}

#endif // ! OURPAINT_HEADERS_UNDOREDO_UNDOREDOMANAGER_H_