#ifndef OURPAINT_PARAMETERDELEGATE_H
#define OURPAINT_PARAMETERDELEGATE_H

#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QString>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTimer>
#include <QItemDelegate>
#include <QPainter>
#include <QApplication>
#include "ProtectedPrefixLineEdit.h"
#include "TreeNode.h"

// A class for changing data in the left menu

class ParameterDelegate : public QStyledItemDelegate {
Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    bool
    editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                const QModelIndex& index) override;

    void paint(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override;

signals:
    void deleteClicked(TreeNode* node);
    void doubleClickOnID(const QModelIndex& index);
};

#endif //OURPAINT_PARAMETERDELEGATE_H
