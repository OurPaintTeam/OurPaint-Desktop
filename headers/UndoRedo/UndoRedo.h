#ifndef OURPAINT_HEADERS_UNDOREDO_HISTORYMANAGER_H_
#define OURPAINT_HEADERS_UNDOREDO_HISTORYMANAGER_H_

#include "Transaction.h"
#include <stack>

class UndoRedo {
private:
    unsigned int maxSteps;
    std::deque<Transaction> transactions_undo;
    std::stack<Transaction> transactions_redo;

public:
    UndoRedo(unsigned int maxSteps = 100);

    UndoRedo(const UndoRedo&) = delete;
    UndoRedo(UndoRedo&&) = delete;
    UndoRedo& operator=(const UndoRedo&) = delete;
    UndoRedo operator=(UndoRedo&&) = delete;

    void push(Transaction&&); // Добавить транзакцию
    bool undo(); // Отменить последнюю транзакцию
    bool redo(); // Повторить отмененную транзакцию

    void setMaxUndoSteps(unsigned int); // Лимит отмены

    bool canUndo() const;
    bool canRedo() const;
};

#endif // ! OURPAINT_HEADERS_UNDOREDO_HISTORYMANAGER_H_