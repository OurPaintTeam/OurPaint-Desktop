#include "ParameterDelegate.h"

QWidget* ParameterDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const {
    const QString text = index.data(Qt::DisplayRole).toString();

    // If parameters
    if (text.contains(": ")) {
        const QString namePart = text.section(": ", 0, 0);
        const QString numberPart = text.section(": ", 1, 1);
        const QString prefix = namePart + ": ";

        auto* editor = new ProtectedPrefixLineEdit(prefix, parent);
        editor->setStyleSheet("background-color: #494850;");
        editor->insert(numberPart);
        editor->setDoubleNumber(true);
        return editor;
    } else {
        // If name
        QLineEdit* editor = new QLineEdit(parent);
        editor->setStyleSheet("background-color: #494850;");
        return editor;
    }
}


void ParameterDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {

    auto* lineEdit = qobject_cast<QLineEdit*>(editor);

    if (!lineEdit)
        return;

    QString text = index.data(Qt::DisplayRole).toString();

    if (text.contains(": ")) {
        const quint16 colonIndex = text.indexOf(": ");
        const quint16 prefixLength = colonIndex + 2;
        const qint16 numberLength = text.length() - prefixLength;

        QTimer::singleShot(0, lineEdit, [lineEdit, prefixLength, numberLength]() {
            lineEdit->setSelection(prefixLength, numberLength);
        });
    } else {
        // Simple Name
        lineEdit->setText(text);
    }
}


void ParameterDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                     const QModelIndex& index) const {

    auto* lineEdit = qobject_cast<QLineEdit*>(editor);

    if (!lineEdit) {
        return;
    }

    const QString fullText = lineEdit->text();
    const QString originalText = index.data(Qt::DisplayRole).toString();

    if (originalText.contains(": ")) {
        const qint16 colonIndex = originalText.indexOf(": ");
        const qint16 prefixLength = colonIndex + 2;
        const QString numberOnly = fullText.mid(prefixLength);
        model->setData(index, numberOnly, Qt::EditRole);
    } else {
        model->setData(index, fullText, Qt::EditRole);
    }
}

bool ParameterDelegate::editorEvent(QEvent* event,
                                    QAbstractItemModel* model,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) {

    if (event->type() == QEvent::MouseButtonDblClick) {

        TreeNode* node = static_cast<TreeNode*>(index.internalPointer());
        if (!node) {
            return QStyledItemDelegate::editorEvent(event, model, option, index);
        }

        if (node->parent()) {
             if ((node->parent()->getName() == "Projects")) {
                 emit doubleClickOnProject(node);
                 return true;
             }
        }

        const QString text = index.data(Qt::DisplayRole).toString();
        if (text.startsWith("ID: ")) {
            emit doubleClickOnID(index);
            return true;
        }
    }

    if (event->type() == QEvent::MouseButtonRelease) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);

        QRect rect = option.rect;
        QRect crossRect(rect.right() - 20, rect.top(), 20, rect.height());

        if (crossRect.contains(mouseEvent->pos())) {
            TreeNode* node =
                static_cast<TreeNode*>(index.internalPointer());
            emit deleteClicked(node);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void ParameterDelegate::paint(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    TreeNode* node =
        static_cast<TreeNode*>(index.internalPointer());

    if (!node || !node->parent()) {
        return;
    }

    if (node->parent()->getName() != "Projects") {
        return;
    }

    QRect rect = option.rect;
    QRect crossRect(rect.right() - 20, rect.top(), 20, rect.height());

    painter->save();
    painter->setPen(QColor(220, 220, 220));
    painter->drawText(crossRect, Qt::AlignCenter, "×");
    painter->restore();
}