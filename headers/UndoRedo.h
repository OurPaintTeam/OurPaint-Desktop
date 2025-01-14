#ifndef OURPAINT_HEADERS_UNDOREDO_H_
#define OURPAINT_HEADERS_UNDOREDO_H_

#include <stack>
#include <stdexcept>

template <typename T>
class UndoRedo {
    std::stack<T> m_undoData;
    std::stack<T> m_redoData;
public:
    UndoRedo();

    UndoRedo(const UndoRedo & other);
    UndoRedo(UndoRedo&& other) noexcept ;
    UndoRedo& operator=(const UndoRedo& other);
    UndoRedo& operator=(UndoRedo&& other) noexcept ;

    void add(const T& data);
    T undo();
    T redo();
};

template<typename T>
void UndoRedo<T>::add(const T &data) {
    if (!m_redoData.empty()){
        std::stack<T> tmp;
        std::swap(m_redoData, tmp);
    }
    m_undoData.push(data);
}

template<typename T>
T UndoRedo<T>::undo() {
    if (m_undoData.empty()) {
        throw std::runtime_error("Nothing to undo");
    }
    T v = m_undoData.top();
    m_redoData.push(v);
    m_undoData.pop();
    return v;
}
template<typename T>
T UndoRedo<T>::redo(){
    if (m_redoData.empty()) {
        throw std::runtime_error("Nothing to redo");
    }
    T v = m_redoData.top();
    m_undoData.push(v);
    m_redoData.pop();
    return v;
}

template<typename T>
UndoRedo<T> &UndoRedo<T>::operator=(UndoRedo &&other)  noexcept {
    if(this != &other){
        std::swap(m_undoData,other.m_undoData);
        std::swap(m_redoData,other.m_redoData);
    }
    return *this;
}

template<typename T>
UndoRedo<T> &UndoRedo<T>::operator=(const UndoRedo &other) {
    if(this != &other){
        m_undoData = other.m_undoData;
        m_redoData = other.m_redoData;
    }
    return *this;
}

template<typename T>
UndoRedo<T>::UndoRedo(UndoRedo &&other) noexcept: m_redoData(std::move(other.m_redoData)), m_undoData(std::move(other.m_undoData)) {}

template<typename T>
UndoRedo<T>::UndoRedo(const UndoRedo &other): m_undoData(other.m_undoData), m_redoData(other.m_redoData) {}

template<typename T>
UndoRedo<T>::UndoRedo() {}

#endif // ! OURPAINT_HEADERS_UNDOREDO_H_