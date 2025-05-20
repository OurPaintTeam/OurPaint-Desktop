#include "TreeModel.h"

TreeModel::TreeModel(QObject* parent)
        : QAbstractItemModel(parent) {
    rootNode = new TreeNode("Root");
}

TreeModel::~TreeModel() {
    delete rootNode;
}

QModelIndex TreeModel::indexFromNode(TreeNode* node) const {
    if (!node || node == rootNode) {
        return QModelIndex();
    }

    TreeNode* parent = node->parent();
    if (!parent) {
        return QModelIndex();
    }

    qint32 row = node->row();
    return createIndex(row, 0, node);
}

void TreeModel::safelyResetModel() {
    beginResetModel();
    endResetModel();
}

void TreeModel::removeNode(TreeNode* parent, TreeNode* child) {
    if (!parent || !child) {
        return;
    }

    qint32 row = -1;
    for (qsizetype i = 0; i < parent->childCount(); ++i) {
        if (parent->child(i) == child) {
            row = i;
            break;
        }
    }

    if (row < 0) {
        return;
    }

    QModelIndex parentIndex = indexFromNode(parent);

    beginRemoveRows(parentIndex, row, row);
    parent->removeChildAt(row);
    endRemoveRows();
}

QVariant TreeModel::data(const QModelIndex& index, qint32 role) const {
    if (!index.isValid() || index.column() != 0) {
        return QVariant();
    }

    TreeNode* item = static_cast<TreeNode*>(index.internalPointer());
    if (!item) {
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return item->data(0);
        case Qt::FontRole:
            return item->getFont();
        case Qt::DecorationRole:
            return item->getIcon();
        default:
            return QVariant();
    }
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, qint32 role) {
    if (!index.isValid() || role != Qt::EditRole || index.column() != 0) {
        return false;
    }

    TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
    if (!node) return false;

    QString oldText = node->data(0).toString();

    if (node->isNumber()) {
        QString name = oldText.section(": ", 0, 0);
        bool ok = false;
        double number = value.toString().toDouble(&ok);
        if (!ok) return false;

        QString newText = QString("%1: %2").arg(name).arg(number, 0, 'g', 10);
        node->setName(newText.isEmpty() ? oldText : newText);

    } else if (node->isLiteral()) {
        QString str = value.toString();
        node->setName(str.isEmpty() ? oldText : str);
    }

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    if (node->isNumber() && node->parent()) {
        emit treeModelChanged(node->parent());
    }

    return true;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
    TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
    if (!node) return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::NoItemFlags;
    if (node->isEnable()) {
        flags |= Qt::ItemIsEnabled;
    }
    if (node->isSelected()){
        flags |= Qt::ItemIsSelectable;
    }
    if (node->isEditable()){
        flags |= Qt::ItemIsEditable;
    }
    if (node->isDropEnabled()) {
        flags |= Qt::ItemIsDropEnabled;
    }
    return flags;
}

QVariant TreeModel::headerData(qint32 section, Qt::Orientation orientation, qint32 role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return QString("Title");
    }
    return QVariant();
}

QModelIndex TreeModel::index(qint32 row, qint32 column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeNode* parentItem = parent.isValid()
                           ? static_cast<TreeNode*>(parent.internalPointer())
                           : rootNode;

    TreeNode* childItem = parentItem->child(row);
    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()){
        return QModelIndex();
    }

    TreeNode* childItem = static_cast<TreeNode*>(index.internalPointer());
    if (!childItem) {
        return QModelIndex();
    }

    TreeNode* parentItem = childItem->parent();
    if (!parentItem || parentItem == rootNode) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

qint32 TreeModel::rowCount(const QModelIndex& parent) const {
    TreeNode* parentItem = parent.isValid()
                           ? static_cast<TreeNode*>(parent.internalPointer())
                           : rootNode;

    return parentItem ? parentItem->childCount() : 0;
}

qint32 TreeModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return 1;
}

TreeNode* TreeModel::getRootNode() const {
    return rootNode;
}
