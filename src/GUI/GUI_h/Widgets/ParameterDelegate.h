#ifndef OURPAINT_PARAMETERDELEGATE_H
#define OURPAINT_PARAMETERDELEGATE_H

#include <QApplication>
#include <QHBoxLayout>
#include <QItemDelegate>
#include <QLineEdit>
#include <QPainter>
#include <QString>
#include <QStyledItemDelegate>
#include <QTimer>

#include "ProtectedPrefixLineEdit.h"
#include "TreeNode.h"

// A class for changing data in the left menu

class ParameterDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    bool
    editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                const QModelIndex &index) override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void showContextMenu(QPoint globalPos, const QModelIndex& index,TreeNode* node);

signals:
    void deleteClicked(TreeNode *node);
    void doubleClickOnProject(TreeNode *node);
    void doubleClickOnID(const QModelIndex &index);
    void renameRequested(const QModelIndex& index);
};

#endif //OURPAINT_PARAMETERDELEGATE_H
