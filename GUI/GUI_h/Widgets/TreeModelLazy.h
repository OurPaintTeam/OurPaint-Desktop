#ifndef OURPAINT_TREEMODELLAZY_H
#define OURPAINT_TREEMODELLAZY_H

#include <QAbstractItemModel>  // Базовый абстрактный класс
#include "TreeNode.h"          // класс узла дерева

// Класс TreeModelLazy реализует ленивую модель дерева на основе QAbstractItemModel
// Он используется в QTreeView для отображения иерархических структур

class TreeModelLazy : public QAbstractItemModel {
Q_OBJECT
private:
    TreeNode* rootNode;  // Корневой узел дерева

public:
    explicit TreeModelLazy(QObject* parent = nullptr);

    ~TreeModelLazy() override;

    // Возвращает данные
    QVariant data(const QModelIndex& index, int role) const override;

    // Возвращает флаги элемента
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Возвращает заголовок столбца или строки
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // Создаёт QModelIndex для указанного элемента по строке, колонке и родителю
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    // Возвращает родительский QModelIndex для указанного элемента
    QModelIndex parent(const QModelIndex& index) const override;

    // Возвращает количество дочерних элементов у указанного родителя
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    // Возвращает количество колонок
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    TreeNode* getRootNode() const;

};

#endif // OURPAINT_TREEMODELLAZY_H
