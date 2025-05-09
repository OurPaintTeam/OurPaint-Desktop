#ifndef OURPAINT_HEADERS_UNDOREDO_H_
#define OURPAINT_HEADERS_UNDOREDO_H_

#include <stack>
#include <stdexcept>

namespace UndoRedo {

    template<typename T>
    class UndoRedoT {
        std::stack<T> m_undoData;
        std::stack<T> m_redoData;

    public:
        UndoRedoT();
        UndoRedoT(const UndoRedoT &other);
        UndoRedoT(UndoRedoT &&other) noexcept;
        UndoRedoT &operator=(const UndoRedoT &other);
        UndoRedoT &operator=(UndoRedoT &&other) noexcept;

        void add(const T &data);

        T undo();
        T redo();
    };

}

#endif // ! OURPAINT_HEADERS_UNDOREDO_H_