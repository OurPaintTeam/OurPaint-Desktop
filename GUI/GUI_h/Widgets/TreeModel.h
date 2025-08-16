#ifndef OURPAINT_TREEMODEL_H
#define OURPAINT_TREEMODEL_H

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include "TreeNode.h"

// The TreeModelLazy class implements a lazy tree model based on QAbstractItemModel
// It is used in QTreeView to display hierarchical structures

class TreeModel : public QAbstractItemModel {
Q_OBJECT
private:
    TreeNode* rootNode;  // Root node of the tree

public:
    explicit TreeModel(QObject* parent = nullptr);

    ~TreeModel() override;

    QModelIndex indexFromNode(TreeNode* node) const;

    void safelyResetModel();

    void removeNode(TreeNode* parent, TreeNode* child);

    // Returns data
    QVariant data(const QModelIndex& index, qint32 role) const override;

    bool setData(const QModelIndex& index, const QVariant& value, qint32 role) override;

    // Returns element flags
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Returns the column or row header
    QVariant headerData(qint32 section, Qt::Orientation orientation, qint32 role) const override;

    // Creates a QModelIndex for the specified element by row, column, and parent
    QModelIndex index(qint32 row, qint32 column, const QModelIndex& parent = QModelIndex()) const override;

    // Returns the parent QModelIndex for the specified element
    QModelIndex parent(const QModelIndex& index) const override;

    // Returns the number of child elements of the specified parent
    qint32 rowCount(const QModelIndex& parent = QModelIndex()) const override;

    // Returns the number of columns
    qint32 columnCount(const QModelIndex& parent = QModelIndex()) const override;

    TreeNode* getRootNode() const;

signals:

    void treeModelChanged(TreeNode* node);


};

#endif // OURPAINT_TREEMODEL_H
