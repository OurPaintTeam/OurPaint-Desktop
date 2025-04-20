#include "TreeModelLazy.h"

TreeModelLazy::TreeModelLazy(QObject* parent)
        : QAbstractItemModel(parent) {
    rootNode = new TreeNode("Root");
}

TreeModelLazy::~TreeModelLazy() {
    delete rootNode;
}

// Возвращает данные для конкретной ячейки
QVariant TreeModelLazy::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) { return QVariant(); }


    TreeNode* item = static_cast<TreeNode*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }

    else if (role == Qt::FontRole) {
        return item->getFont();
    }

    return QVariant();
}

// Определяет, какие действия возможны с элементом
Qt::ItemFlags TreeModelLazy::flags(const QModelIndex& index) const {
    if (!index.isValid()) { return Qt::NoItemFlags; }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

// Заголовок столбца
QVariant TreeModelLazy::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) { return QString("Title"); }
    return QVariant();
}

// Создаёт QModelIndex для элемента по координатам row, column и родителю
QModelIndex TreeModelLazy::index(int row, int column, const QModelIndex& parent) const {
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
QModelIndex TreeModelLazy::parent(const QModelIndex& index) const {
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
int TreeModelLazy::rowCount(const QModelIndex& parent) const {
    TreeNode* parentItem;

    // Если родитель не задан — корень
    if (!parent.isValid()) { parentItem = rootNode; }
    else { parentItem = static_cast<TreeNode *>(parent.internalPointer()); }

    return parentItem->childCount();
}

// Возвращает количество колонок
int TreeModelLazy::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);  // Параметр не используется
    return 1;
}

TreeNode* TreeModelLazy::getRootNode() const {
    return rootNode;
}
