#ifndef OURPAINT_LEFTMENUBAR_H
#define OURPAINT_LEFTMENUBAR_H

#include <QTreeWidget>
#include <QIcon>
#include <QString>
#include <vector>
#include <string>
#include <QTimer>
#include <QWidget>

class LeftMenuBar: public QObject {
Q_OBJECT
private:
    QTreeWidget *menu;
    bool addElem = false;
    QTreeWidgetItem *itemFigures;
    QTreeWidgetItem *itemReq;


public:
    QTreeWidgetItem* getItemFigures() const {
        return itemFigures;
    }

    explicit LeftMenuBar(QTreeWidget *leftMenu); // Передаём указатель на QTreeWidget

    void printObject(unsigned long long id, const std::string &text, const std::vector<double> &object);

    void
    printReq(unsigned long long int id, const std::string &text, unsigned long long int id1, unsigned long long int id2,
             double parametr);


    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

    void processChildItems(const QList<QTreeWidgetItem *> &childItems);

    QTreeWidgetItem *findItemById(QTreeWidgetItem *item, unsigned long long int id);

    void FocusOnItemById(unsigned long long int id);

    std::vector<std::vector<QString>> getListFigures();

    std::vector<std::vector<QString>> getListReq();

    QList<QTreeWidgetItem *> getAllChildItems(QTreeWidgetItem *item);

    bool isFiguresExpanded();

    void removeIcons(QTreeWidgetItem *parentItem);

    void loadIconsGradually(QTreeWidgetItem *parentItem);

public slots:
    // Изменение обьектов левого меню
    void LeftMenuChanged(QTreeWidgetItem *item);
    void onItemExpanded(QTreeWidgetItem *item);    // Слот для раскрытия
    void onItemCollapsed(QTreeWidgetItem *item);   // Слот для сворачивания

signals:
    void parameterChanged(unsigned long long id, const std::vector<double> &parameters);
    void DoubleClickLeftMenu(std::vector<double>& parameters,unsigned long long id);
    void showMenu();

};

#endif //OURPAINT_LEFTMENUBAR_H
