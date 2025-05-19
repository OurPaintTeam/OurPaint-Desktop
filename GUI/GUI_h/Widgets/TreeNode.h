#ifndef OURPAINT_TREENODE_H
#define OURPAINT_TREENODE_H

#include <QVariant>
#include <QVector>
#include <QFont>
#include <QStringView>
#include <QIcon>

// TreeNode is a single item in a tree structure used by TreeModel

class TreeNode {
private:
    QVector<TreeNode*> vec_child;     // List of child nodes
    QVariant dataChild;               // Data associated with this node
    TreeNode* _parent;                // Pointer to parent node
    QFont childFont;                  // Font used to display this node
    QIcon icon;                       // Icon associated with the node

    const double* _linkedValue = nullptr; // Pointer to external double value
    bool _isLinkedNumber = false;         // True if this node shows a linked numeric value

    bool editable;                   // Whether the node's data is editable
    bool doubleClick;                // Whether double-click is allowed
    bool number;                     // Whether node represents a number
    bool literal;                    // Whether node represents a literal
    bool enabled;                    // Whether the node is enabled (visible/selectable)
    bool selected;                   // Whether the node is currently selected
    bool dropEnabled;                // Whether node supports drag-and-drop

public:
    explicit TreeNode(const QVariant& data, TreeNode* parent = nullptr);

    ~TreeNode();

    // Add or access child nodes
    void addChild(TreeNode* child);

    TreeNode* child(qint32 row) const;

    qint32 childCount() const;

    qint32 row() const;

    void removeChildAt(qint32 index);          // Remove child by index
    bool removeChild(TreeNode* child);      // Remove child by pointer

    // Remove all children
    void deleteAll();
    void deleteRecursively();

    // Data access
    QVariant data(qint32 column) const;

    void setData(qint32 column, const QVariant& value);

    QString getNameOnly() const;            // Return name (before colon)
    QString getName() const;                // Return full name
    void setName(const QString& name);      // Set display name

    // Font and icon customization
    QFont getFont() const;

    void setFont(const QFont& font);

    QIcon getIcon() const;

    void setIcon(const QIcon& newIcon);

    void setLinkedValue(const double* ptr); // Set external pointer
    const double* linkedValue() const;      // Get pointer
    bool isLinkedNumber() const;            // Check if linked
    void updateTextFromLinkedValue();       // Update display based on linked value

    TreeNode* parent();                     // Get parent node
    qint32 depth() const;                      // Depth of this node in the tree

    // Node state flags
    void setEditable(bool e);

    bool isEditable() const;

    void setDoubleClick(bool e);

    bool isDoubleClick() const;

    void setLiteral(bool e);

    bool isLiteral() const;

    void setNumber(bool e);

    bool isNumber() const;

    void setEnable(bool e);

    bool isEnable() const;

    void setSelected(bool e);

    bool isSelected() const;

    void setDropEnabled(bool e);

    bool isDropEnabled() const;
};

#endif // OURPAINT_TREENODE_H
