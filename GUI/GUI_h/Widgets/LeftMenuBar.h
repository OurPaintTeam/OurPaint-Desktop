#ifndef OURPAINT_LEFTMENUBAR_H
#define OURPAINT_LEFTMENUBAR_H

#include <QTreeWidget>
#include <QIcon>
#include <QString>
#include <vector>
#include <string>
#include <QTimer>
#include <QWidget>
#include <QFile>
#include <QDataStream>
#include <iostream>
#include <QLabel>


#include "TreeModelLazy.h"

// Класс для управления деревом

class LeftMenuBar {
// Q_OBJECT
private:
    TreeModelLazy *treeModel = nullptr;
    TreeNode *rootNode = nullptr;
    TreeNode *figuresNode = nullptr;
    TreeNode *requirementsNode = nullptr;
    TreeNode *nothing = nullptr;
    QIcon elem;
    QIcon paraam;

    QFont font;

public:
    explicit LeftMenuBar(QObject *parent);

    TreeModelLazy *getTreeModel();

    // Добавление фигуры
    void addElemLeftMenu(const QString &name, unsigned long long ID, const std::vector<double> &params);

    // Добавление требований
    void addRequirementElem(const QString &name, const int ReqID, const unsigned long long ElemID1,
                            const unsigned long long ElemID2, const double param);

    // Добавление требований
    void addRequirementElem(const QString &name, const int ReqID, const unsigned long long ElemID1,
                            const unsigned long long ElemID2);

    // Очистка всех элементов
    void clearAllFigures();

    // Очистка всех элементов
    void clearAllRequirements();

    void  updateLeftMenu();

    // Очистка одного элемента по айди
    void removeFigureById(unsigned long long id);

    // Очистка одного элемента по айди
    void removeRequirementById(unsigned long long id);

    // Изменение параметров по айди
    void updateParametersById(unsigned long long id, const std::vector<double> &newParams);

    // Сохранение в бин файл
    void saveToBinaryFile(const QString &filePath);

    // Считывание бин файла
    void loadFromBinaryFile(const QString &filePath);


signals:

    void parameterChanged(unsigned long long id, const std::vector<double> &parameters);

    void DoubleClickLeftMenu(std::vector<double> &parameters, unsigned long long id);

    void showMenu();

};

#endif //OURPAINT_LEFTMENUBAR_H
