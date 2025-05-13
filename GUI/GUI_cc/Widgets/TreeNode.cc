#include "TreeNode.h"

TreeNode::TreeNode(const QVariant& data, TreeNode* parent)
        : dataChild(data), parent(parent) {}

TreeNode::~TreeNode() {
    qDeleteAll(vec_child);
}

// Добавляет дочерний узел
void TreeNode::addChild(TreeNode* child) {
    vec_child.append(child);
}

// Возвращает указатель на дочерний элемент по индексу
TreeNode* TreeNode::child(int row) const {
    return vec_child.value(row);
}

// Возвращает количество дочерних узлов
int TreeNode::childCount() const {
    return vec_child.count();
}

// Возвращает данные узла
QVariant TreeNode::data(int column) const {
    if (column == 0) { return dataChild; }
    return QVariant();  // пустое значение для других колонок
}

void TreeNode::setIcon(const QIcon& newIcon)
{ icon = newIcon; }

QIcon TreeNode::getIcon() const
{ return icon; }

// Назначить данные обьекта
void TreeNode::setData(int column, const QVariant& value) {
    if (column == 0)
        dataChild = value;
}

// Возвращает позицию текущего узла в списке детей родителя
int TreeNode::row() const {
    if (parent) { return parent->vec_child.indexOf(const_cast<TreeNode *>(this)); }
    return 0;
}

// Возвращает родительский узел
TreeNode* TreeNode::parentItem() {
    return parent;
}

void TreeNode::setFont(const QFont& font) {
    childFont = font;
}

QFont TreeNode::getFont() const {
    return childFont;
}

// Удаляет все дочерние узлы
void TreeNode::deleteAll() {
    qDeleteAll(vec_child);
    vec_child.clear();
}

// Удаление одного
bool TreeNode::removeChild(TreeNode* child) {
    int index = vec_child.indexOf(child);
    if (index >= 0) {
        delete vec_child[index];
        vec_child.remove(index);
        return true;
    }
    return false;
}

QString TreeNode::getName() const {
    return dataChild.toString();
}

void TreeNode::setName(const QString& name) {
    dataChild = name;
}


