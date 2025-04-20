#include "LeftMenuBar.h"

LeftMenuBar::LeftMenuBar(QObject  *parent) {
    // Создаём модель
    treeModel = new TreeModelLazy(parent);
    rootNode = treeModel->getRootNode();

    // Создаём узлы
    nothing = new TreeNode("", rootNode); // для отступа сверху
    figuresNode = new TreeNode("Figures", rootNode);
    requirementsNode = new TreeNode("Requirements", rootNode);

    rootNode->addChild(nothing);
    rootNode->addChild(figuresNode);
    rootNode->addChild(requirementsNode);

    // Шрифт
    QFont fontCategory;
    fontCategory.setPointSize(12);
    figuresNode->setFont(fontCategory);
    requirementsNode->setFont(fontCategory);
}

TreeModelLazy* LeftMenuBar::getTreeModel(){
    return treeModel;
}

// Добавление фигуры
void LeftMenuBar::addElemLeftMenu(const QString &name, unsigned long long ID, const std::vector<double> &params) {
    if (!figuresNode) { return; }

    // Создаем основной узел элемента
    TreeNode* elemNode = new TreeNode(name, figuresNode);
    figuresNode->addChild(elemNode);

    // Добавляем ID как дочерний узел
    TreeNode* idNode = new TreeNode(QString("ID = %1").arg(ID), elemNode);
    elemNode->addChild(idNode);


    if (name == "Point") {
        QStringList paramNames = { "x", "y" };
        for (int i = 0; i < paramNames.size(); ++i) {
            TreeNode* paramNode = new TreeNode(QString("%1 = %2").arg(paramNames[i]).arg(params[i]), elemNode);
            elemNode->addChild(paramNode);
        }
    } else if (name == "Section") {
        QStringList paramNames = { "x0", "y0", "x1", "y1" };
        for (int i = 0; i < paramNames.size(); ++i) {
            TreeNode* paramNode = new TreeNode(QString("%1 = %2").arg(paramNames[i]).arg(params[i]), elemNode);
            elemNode->addChild(paramNode);
        }
    }else if (name == "Circle") {
        QStringList paramNames = { "x", "y", "R" };
        for (int i = 0; i < paramNames.size(); ++i) {
            TreeNode* paramNode = new TreeNode(QString("%1 = %2").arg(paramNames[i]).arg(params[i]), elemNode);
            elemNode->addChild(paramNode);
        }
    }

}

// Добавление требований
void LeftMenuBar::addRequirementElem(const QString &name, const unsigned long long ReqID,const unsigned long long ElemID1,const unsigned long long ElemID2, const double param) {
    if (!requirementsNode) { return; }

    // Создаем основной узел элемента
    TreeNode *elemNode = new TreeNode(name, requirementsNode);
    requirementsNode->addChild(elemNode);

    // Добавляем ID как дочерний узел
    TreeNode *ReqIDNode = new TreeNode(QString("ID = %1").arg(ReqID), elemNode);
    elemNode->addChild(ReqIDNode);

    // Добавляем ID как дочерний узел
    TreeNode *ElemID1Node = new TreeNode(QString("ID1 = %1").arg(ElemID1), elemNode);
    elemNode->addChild(ElemID1Node);

    // Добавляем ID как дочерний узел
    TreeNode *ElemID2Node = new TreeNode(QString("ID2 = %1").arg(ElemID2), elemNode);
    elemNode->addChild(ElemID2Node);

    // Добавляем ID как дочерний узел
    TreeNode *paramNode = new TreeNode(QString("Param = %1").arg(param), elemNode);
    elemNode->addChild(paramNode);
}

// Очистка всех элементов
void LeftMenuBar::clearAllFigures() {
    if (!figuresNode || !treeModel) { return; }

    // Удаляем всех детей узла
    figuresNode->deleteAll();

    // Сообщаем вьюшке, что структура дерева изменилась
    QModelIndex figuresIndex = treeModel->index(figuresNode->row(), 0, QModelIndex());
    treeModel->dataChanged(figuresIndex, figuresIndex);
    treeModel->layoutChanged();  // Обновляем структуру модели
}

// Очистка всех элементов
void LeftMenuBar::LeftMenuBar::clearAllRequirements() {
    if (!requirementsNode || !treeModel) { return; }

    // Удаляем всех детей узла
    requirementsNode->deleteAll();

    QModelIndex reqIndex = treeModel->index(requirementsNode->row(), 0, QModelIndex());
    treeModel->dataChanged(reqIndex, reqIndex);
    treeModel->layoutChanged();
}

// Очистка одного элемента по айди
void LeftMenuBar::removeFigureById(unsigned long long id) {
    if (!figuresNode) { return; }

    for (int i = 0; i < figuresNode->childCount(); ++i) {
        TreeNode* elemNode = figuresNode->child(i);

        for (int j = 0; j < elemNode->childCount(); ++j) {
            TreeNode* child = elemNode->child(j);
            if (child->data(0).toString() == QString("id = %1").arg(id)) {
                figuresNode->removeChild(elemNode);

                // Обновление модели
                treeModel->layoutChanged();
                { return; }
            }
        }
    }
}

// Очистка одного элемента по айди
void LeftMenuBar::removeRequirementById(unsigned long long id) {
    if (!requirementsNode) { return; }

    for (int i = 0; i < requirementsNode->childCount(); ++i) {
        TreeNode* elemNode = requirementsNode->child(i);

        for (int j = 0; j < elemNode->childCount(); ++j) {
            TreeNode* child = elemNode->child(j);
            if (child->data(0).toString() == QString("id = %1").arg(id)) {
                requirementsNode->removeChild(elemNode);

                // Обновление модели
                treeModel->layoutChanged();
                { return; }
            }
        }
    }
}

// Изменение параметров по айди
void LeftMenuBar::updateParametersById(unsigned long long id, const std::vector<double>& newParams) {
    auto updateInNode = [&](TreeNode* categoryNode) {
        if (!categoryNode) { return; }

        for (int i = 0; i < categoryNode->childCount(); ++i) {
            TreeNode* elemNode = categoryNode->child(i);

            for (int j = 0; j < elemNode->childCount(); ++j) {
                TreeNode* child = elemNode->child(j);

                if (child->data(0).toString() == QString("id = %1").arg(id)) {
                    // Найден нужный элемент
                    QString name = elemNode->data(0).toString();

                    QStringList paramNames;
                    if (name == "Point") {
                        paramNames = { "x", "y" };
                    } else if (name == "Section") {
                        paramNames = { "x0", "y0", "x1", "y1" };
                    } else if (name == "Circle") {
                        paramNames = { "x", "y", "r" };
                    } else {
                        { return; }
                    }

                    int paramStartIndex = 1; // после "id = ..." идут параметры
                    for (int k = 0; k < paramNames.size() && (paramStartIndex + k) < elemNode->childCount(); ++k) {
                        TreeNode* paramNode = elemNode->child(paramStartIndex + k);
                        paramNode->setData(0, QString("%1 = %2").arg(paramNames[k]).arg(newParams[k]));
                    }

                    treeModel->layoutChanged(); // обновим отображение
                    { return; }
                }
            }
        }
    };

    updateInNode(figuresNode);
    updateInNode(requirementsNode);
}

// Сохранение в бин файл
void LeftMenuBar::saveToBinaryFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Не удалось открыть файл для записи");
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_9);


    file.close();
}

// Считывание бин файла
void LeftMenuBar::loadFromBinaryFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Не удалось открыть файл для чтения");
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_9);

    // Очищаем текущие объекты
    clearAllFigures();
    clearAllRequirements();


    file.close();
}
