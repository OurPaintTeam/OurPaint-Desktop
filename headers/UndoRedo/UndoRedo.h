#ifndef OURPAINT_HEADERS_UNDOREDO_UNDOREDOMANAGER_H_
#define OURPAINT_HEADERS_UNDOREDO_UNDOREDOMANAGER_H_

#include "Transaction.h"
#include <stack>

namespace UndoRedo {

    class UndoRedoManager {
    private:
        unsigned int maxSteps;
        std::deque<Transaction> transactions_undo;
        std::stack<Transaction> transactions_redo;

    public:
        UndoRedoManager(unsigned int maxSteps = 100);
        UndoRedoManager(const UndoRedoManager &) = delete;
        UndoRedoManager(UndoRedoManager &&) = delete;
        UndoRedoManager &operator=(const UndoRedoManager &) = delete;
        UndoRedoManager operator=(UndoRedoManager &&) = delete;

        void push(Transaction &&); // Добавить транзакцию
        bool undo(); // Отменить последнюю транзакцию
        bool redo(); // Повторить отмененную транзакцию

        void setMaxUndoSteps(unsigned int); // Лимит отмены

        bool canUndo() const;

        bool canRedo() const;
    };

}

#endif // ! OURPAINT_HEADERS_UNDOREDO_UNDOREDOMANAGER_H_