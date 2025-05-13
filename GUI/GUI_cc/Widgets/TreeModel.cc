#include "TreeModel.h"

TreeModel::TreeModel(QObject* parent)
        : QAbstractItemModel(parent) {
    rootNode = new TreeNode("Root");
}

TreeModel::~TreeModel() {
    delete rootNode;
}

// Возвращает данные для конкретной ячейки
QVariant TreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    TreeNode* item = static_cast<TreeNode*>(index.internalPointer());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return item->data(index.column());
    } else if (role == Qt::FontRole) {
        return item->getFont();
    } else if (role == Qt::DecorationRole) {
        return item->getIcon();
    }

    return QVariant();
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
    if (!node)
        return false;

    node->setName(value.toString());
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

// Определяет, какие действия возможны с элементом
Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) { return Qt::NoItemFlags; }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

// Заголовок столбца
QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) { return QString("Title"); }
    return QVariant();
}

// Создаёт QModelIndex для элемента по координатам row, column и родителю
QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) { return QModelIndex(); }

    TreeNode* parentItem;

    if (!parent.isValid()) { parentItem = rootNode; }
    else { parentItem = static_cast<TreeNode *>(parent.internalPointer()); }

    // Получаем нужного ребёнка
    TreeNode* childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);  // Создаём индекс, привязанный к узлу
         }
    return QModelIndex();
}

// Возвращает QModelIndex родителя текущего элемента
QModelIndex TreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) { return QModelIndex(); }

    // Получаем текущий и родительский узлы
    TreeNode* childItem = static_cast<TreeNode*>(index.internalPointer());
    TreeNode* parentItem = childItem->parentItem();

    // Если родитель — корень или nullptr, значит, это верхний уровень — нет родителя
    if (parentItem == rootNode || parentItem == nullptr) { return QModelIndex(); }

    // Возвращаем индекс родителя
    return createIndex(parentItem->row(), 0, parentItem);
}

// Количество дочерних элементов у родителя
int TreeModel::rowCount(const QModelIndex& parent) const {
    TreeNode* parentItem;

    // Если родитель не задан — корень
    if (!parent.isValid()) { parentItem = rootNode; }
    else { parentItem = static_cast<TreeNode *>(parent.internalPointer()); }

    return parentItem->childCount();
}

// Возвращает количество колонок
int TreeModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);  // Параметр не используется
    return 1;
}

TreeNode* TreeModel::getRootNode() const {
    return rootNode;
}
