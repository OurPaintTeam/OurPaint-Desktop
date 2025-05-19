#include "TreeNode.h"

TreeNode::TreeNode(const QVariant& data, TreeNode* parent)
        : dataChild(data), _parent(parent),
          editable(false), doubleClick(false), literal(false), number(false),
          enabled(true), dropEnabled(false), selected(false) {}

TreeNode::~TreeNode() {
    qDeleteAll(vec_child);
    _linkedValue = nullptr;
}

void TreeNode::addChild(TreeNode* child) {
    vec_child.append(child);
}

TreeNode* TreeNode::child(int row) const {
    return vec_child.value(row);
}

int TreeNode::childCount() const {
    return vec_child.count();
}

int TreeNode::row() const {
    if (_parent) {
        return _parent->vec_child.indexOf(const_cast<TreeNode*>(this));
    }
    return -1;
}

void TreeNode::removeChildAt(int index) {
    if (index >= 0 && index < vec_child.size()) {
        delete vec_child[index];
        vec_child.remove(index);
    }
}

bool TreeNode::removeChild(TreeNode* child) {
    int index = vec_child.indexOf(child);
    if (index >= 0) {
        child->deleteRecursively();
        vec_child.remove(index);
        return true;
    }
    return false;
}

void TreeNode::deleteAll() {
    for (TreeNode* child : vec_child) {
        if (child) {
            child->deleteRecursively();
        }
    }
    vec_child.clear();
}

void TreeNode::deleteRecursively() {
    for (TreeNode* child : vec_child) {
        if (child) {
            child->deleteRecursively();
        }
    }
    vec_child.clear();
}

QVariant TreeNode::data(int column) const {
    if (column == 0) {
        return dataChild;
    }
    return {};
}

void TreeNode::setData(int column, const QVariant& value) {
    if (column == 0)
        dataChild = value;
}

QString TreeNode::getNameOnly() const {
    QString text = dataChild.toString();
    int idx = text.indexOf(':');
    if (idx != -1)
        return text.left(idx).trimmed();
    return text;
}

QString TreeNode::getName() const {
    return dataChild.toString();
}

void TreeNode::setName(const QString& name) {
    dataChild = name;
}

QFont TreeNode::getFont() const {
    return childFont;
}

void TreeNode::setFont(const QFont& font) {
    childFont = font;
}

QIcon TreeNode::getIcon() const {
    return icon;
}

void TreeNode::setIcon(const QIcon& newIcon) {
    icon = newIcon;
}

void TreeNode::setLinkedValue(const double* ptr) {
    _linkedValue = ptr;
    _isLinkedNumber = true;
}

const double* TreeNode::linkedValue() const {
    return _linkedValue;
}

bool TreeNode::isLinkedNumber() const {
    return _isLinkedNumber;
}

void TreeNode::updateTextFromLinkedValue() {
    if (_linkedValue && _isLinkedNumber) {
        setData(0, QString("%1: %2").arg(getNameOnly()).arg(*_linkedValue, 0, 'g', 10));
    } else if (!_linkedValue && _isLinkedNumber) {
        if (_parent) {
            _parent->deleteAll();
        }
    }
}

TreeNode* TreeNode::parent() {
    return _parent;
}

int TreeNode::depth() const {
    int d = 0;
    TreeNode* p = _parent;
    while (p) {
        ++d;
        p = p->_parent;
    }
    return d;
}

void TreeNode::setEditable(bool e) {
    editable = e;
}

bool TreeNode::isEditable() const {
    return editable;
}

void TreeNode::setDoubleClick(bool e) {
    doubleClick = e;
}

bool TreeNode::isDoubleClick() const {
    return doubleClick;
}

void TreeNode::setLiteral(bool e) {
    literal = e;
}

bool TreeNode::isLiteral() const {
    return literal;
}

void TreeNode::setNumber(bool e) {
    number = e;
}

bool TreeNode::isNumber() const {
    return number;
}

void TreeNode::setEnable(bool e) {
    enabled = e;
}

bool TreeNode::isEnable() const {
    return enabled;
}

void TreeNode::setSelected(bool e) {
    selected = e;
}

bool TreeNode::isSelected() const {
    return selected;
}

void TreeNode::setDropEnabled(bool e) {
    dropEnabled = e;
}

bool TreeNode::isDropEnabled() const {
    return dropEnabled;
}
