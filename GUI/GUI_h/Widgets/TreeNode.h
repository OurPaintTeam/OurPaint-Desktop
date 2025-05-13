#ifndef OURPAINT_TREENODE_H
#define OURPAINT_TREENODE_H

#include <QVariant>  // Для хранения данных любого типа
#include <QVector>   // Контейнер для хранения дочерних узлов
#include <QFont>
#include <QIcon>

// Класс TreeNode представляет собой узел дерева, используемого в TreeModel
// Каждый узел хранит данные, список дочерних элементов, ссылку на родителя и шрифт

class TreeNode {
private:
    QVector<TreeNode*> vec_child;  // Список дочерних узлов
    QVariant dataChild;              // Данные, ассоциированные с этим узлом
    TreeNode* parent;           // Указатель на родительский узел
    QFont childFont;
    QIcon icon;
public:
    explicit TreeNode(const QVariant& data, TreeNode* parent = nullptr);

    ~TreeNode();

    // Добавляет дочерний узел к текущему узлу
    void addChild(TreeNode* child);

    // Возвращает указатель на дочерний элемент по индексу
    TreeNode* child(int row) const;

    // Возвращает количество дочерних элементов
    int childCount() const;

    // Возвращает данные узла для заданного столбца
    QVariant data(int column) const;

    void setIcon(const QIcon& newIcon);

    QIcon getIcon() const ;

    // Назначить данные
    void setData(int column, const QVariant& value);

    // Возвращает индекс текущего узла в списке детей родительского узла
    int row() const;

    // Возвращает указатель на родительский узел
    TreeNode* parentItem();

    void setFont(const QFont& font);

    QFont getFont() const;

    void deleteAll();

    // Удаление одного
    bool removeChild(TreeNode* child);

    QString getName() const;

    void setName(const QString& name);

};

#endif // OURPAINT_TREENODE_H
