#include "UndoRedoTemplate.h"

template<typename T>
void UndoRedoT<T>::add(const T &data) {
    if (!m_redoData.empty()){
        std::stack<T> tmp;
        std::swap(m_redoData, tmp);
    }
    m_undoData.push(data);
}

template<typename T>
T UndoRedoT<T>::undo() {
    if (m_undoData.empty()) {
        throw std::runtime_error("Nothing to undo");
    }
    T v = m_undoData.top();
    m_redoData.push(v);
    m_undoData.pop();
    return v;
}
template<typename T>
T UndoRedoT<T>::redo(){
    if (m_redoData.empty()) {
        throw std::runtime_error("Nothing to redo");
    }
    T v = m_redoData.top();
    m_undoData.push(v);
    m_redoData.pop();
    return v;
}

template<typename T>
UndoRedoT<T> &UndoRedoT<T>::operator=(UndoRedoT &&other)  noexcept {
    if(this != &other){
        std::swap(m_undoData,other.m_undoData);
        std::swap(m_redoData,other.m_redoData);
    }
    return *this;
}

template<typename T>
UndoRedoT<T> &UndoRedoT<T>::operator=(const UndoRedoT &other) {
    if(this != &other){
        m_undoData = other.m_undoData;
        m_redoData = other.m_redoData;
    }
    return *this;
}

template<typename T>
UndoRedoT<T>::UndoRedoT(UndoRedoT&& other) noexcept: m_redoData(std::move(other.m_redoData)), m_undoData(std::move(other.m_undoData)) {}

template<typename T>
UndoRedoT<T>::UndoRedoT(const UndoRedoT &other): m_undoData(other.m_undoData), m_redoData(other.m_redoData) {}

template<typename T>
UndoRedoT<T>::UndoRedoT() {}