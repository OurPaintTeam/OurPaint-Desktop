#include "GUI/widgets/LeftMenuBar.h"

LeftMenuBar::LeftMenuBar(QTreeWidget *leftMenu) : menu(leftMenu) {
    connect(menu, &QTreeWidget::itemChanged, this, &LeftMenuBar::LeftMenuChanged);
    connect(menu, &QTreeWidget::itemDoubleClicked, this, &LeftMenuBar::onItemDoubleClicked);

    itemFigures = menu->topLevelItem(1);
    itemReq = menu->topLevelItem(2);

}

void LeftMenuBar::onItemExpanded(QTreeWidgetItem *item) {
    if (item == itemFigures) {
        qDebug() << "Figure item expanded!";
        // Включаем иконки для всех параметров, если панель видна
        loadIconsGradually(item);
    }
}

// Слот, который вызывается при сворачивании элемента
void LeftMenuBar::onItemCollapsed(QTreeWidgetItem *item) {
    if (item == itemFigures) {
        qDebug() << "Figure item collapsed!";
        // Удаляем иконки для всех параметров
        removeIcons(item);
    }
}

void LeftMenuBar::printObject(unsigned long long id, const std::string &text, const std::vector<double> &object) {
    if (text == "Clear") {
        itemFigures->takeChildren();
        return;
    }

    QString figureName = QString::fromStdString(text);
    int count = 1;
    addElem = true;

    while (true) {
        bool exists = false;
        for (int i = 0; i < itemFigures->childCount(); ++i) {
            if (itemFigures->child(i)->text(0) == figureName) {
                exists = true;
                break;
            }
        }
        if (!exists) break;
        figureName = QString("%1%2").arg(QString::fromStdString(text)).arg(count++);
    }

    QTreeWidgetItem *itemFigure = new QTreeWidgetItem(itemFigures);
    itemFigure->setText(0, figureName);
    itemFigure->setFlags(itemFigure->flags() | Qt::ItemIsEditable);

    bool isPanelVisible = menu->isVisible();
    if (isPanelVisible) {
        itemFigure->setIcon(0, QIcon("../Static/icons/Icon.ico"));
    }

    itemFigures->addChild(itemFigure);

    std::vector<QString> paramNames;

    if (object.size() == 2) {
        paramNames = {"ID", "X", "Y"};
    } else if (object.size() == 3) {
        paramNames = {"ID", "X", "Y", "R"};
    } else if (object.size() == 4) {
        paramNames = {"ID", "X1", "Y1", "X2", "Y2"};
    }

    // Для постепенной подгрузки иконок по 30 штук:
    int loadedIcons = 0;

    for (size_t i = 0; i < paramNames.size() && i < object.size() + 1; ++i) {
        QTreeWidgetItem *paramItem = new QTreeWidgetItem(itemFigure);

        if (paramNames[i] != "ID") {
            paramItem->setFlags(paramItem->flags() | Qt::ItemIsEditable);
        } else {
            paramItem->setFlags(paramItem->flags() & ~Qt::ItemIsEditable);
            paramItem->setFlags(paramItem->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }

        // Подгрузка иконок только если панель видна
        if (isPanelVisible && loadedIcons < 30) {
            paramItem->setIcon(0, QIcon("../Static/icons/Database.ico"));
            loadedIcons++;
        }

        if (paramNames[i] == "ID") {
            paramItem->setText(0, QString("%1: %2").arg(paramNames[i]).arg(id));
        } else {
            paramItem->setText(0, QString("%1: %2").arg(paramNames[i]).arg(
                    QString::number(object[i - 1], 'f', 6)));
        }

        itemFigure->addChild(paramItem);
    }

    addElem = false;

}
void LeftMenuBar::loadIconsGradually(QTreeWidgetItem *parentItem) {
    int iconCount = 0;

    // Проходим по всем дочерним элементам и добавляем иконки
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem *item = parentItem->child(i);

        // Устанавливаем иконки
        item->setIcon(0, QIcon("../Static/icons/Database.ico"));
        iconCount++;

        // Печатаем, сколько памяти было "использовано" (на самом деле добавляем иконки)
        qDebug() << "Added icon to item:" << item->text(0);
        qDebug() << "Icons added so far:" << iconCount;

        // Если хотите, чтобы иконки добавлялись постепенно, можете добавить таймер
        if (iconCount >= 30) {
            break; // Останавливаем, если загрузили 30 иконок
        }
    }

    qDebug() << "Total icons added: " << iconCount;
}

void LeftMenuBar::removeIcons(QTreeWidgetItem *parentItem) {
    int removedIcons = 0;

    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem *item = parentItem->child(i);

        // Убираем иконку
        item->setIcon(0, QIcon());
        removedIcons++;

        // Печатаем, сколько иконок было удалено
        qDebug() << "Removed icon from item:" << item->text(0);
    }

    qDebug() << "Total icons removed: " << removedIcons;
}

bool LeftMenuBar::isFiguresExpanded() {
    QModelIndex index = menu->indexFromItem(itemFigures);
    return menu->isExpanded(index);
}


void LeftMenuBar::onItemDoubleClicked(QTreeWidgetItem *item, int column) {
    QString text = item->text(column);
    std::vector<double> parameters;

    if (text.startsWith("ID:")) {
        // Извлекаем ID
        QString idStr = text.section(':', 1).trimmed();
        bool ok = false;
        unsigned long long id = idStr.toULongLong(&ok);
        if (ok) {

            QTreeWidgetItem *parent = item->parent(); // Получаем родителя — фигуру
            if (parent) {
                for (int i = 0; i < parent->childCount(); ++i) {
                    QTreeWidgetItem *child = parent->child(i);
                    QString paramText = child->text(0);

                    // Пропускаем ID
                    if (paramText.startsWith("ID:")) continue;

                    QStringList paramParts = paramText.split(": ");
                    if (paramParts.size() == 2) {
                        bool paramOk = false;
                        double paramValue = paramParts[1].toDouble(&paramOk);
                        if (paramOk) {
                            parameters.push_back(paramValue);
                        }
                    }
                }
            }

            if(!parameters.empty() && id!=0)
                emit DoubleClickLeftMenu(parameters,id);
        }
    }
}




// Добавление требований в левое меню
void LeftMenuBar::printReq(unsigned long long NumberReq, const std::string &name, unsigned long long id1,
                           unsigned long long id2, double parametr) {

// name
    // Requirements ID: NumberReq
    // Element ID: id1
    // Element ID: id2
    // Parament: parament


    if (name == "Clear") {
        itemReq->takeChildren();  // Очищаем все дочерние элементы
        return;
    }

    // Создаем новый элемент с названием
    QString itemType = QString::fromStdString(name);
    QTreeWidgetItem *newItem = new QTreeWidgetItem(itemReq);
    newItem->setText(0, itemType);

    // Устанавливаем иконку для нового элемента
    static QIcon textIcon("../Static/icons/Icon.ico");
    newItem->setIcon(0, textIcon);

    // Создаем подэлементы для "Requirements ID", "Element ID" и "Parameter"
    QTreeWidgetItem *reqItem = new QTreeWidgetItem(newItem);
    reqItem->setText(0, QString("Requirements ID: %1").arg(NumberReq));  // Заполняем Requirements ID
    reqItem->setIcon(0, textIcon);  // Устанавливаем иконку для Requirements ID элемента

    QTreeWidgetItem *elemItem1 = new QTreeWidgetItem(newItem);
    elemItem1->setText(0, QString("Element ID: %1").arg(id1));  // Заполняем первый Element ID
    elemItem1->setIcon(0, textIcon);  // Устанавливаем иконку для первого Element ID элемента

    QTreeWidgetItem *elemItem2 = new QTreeWidgetItem(newItem);
    elemItem2->setText(0, QString("Element ID: %1").arg(id2));  // Заполняем второй Element ID
    elemItem2->setIcon(0, textIcon);  // Устанавливаем иконку для второго Element ID элемента

    QTreeWidgetItem *paramItem = new QTreeWidgetItem(newItem);
    paramItem->setText(0, QString("Parameter: %1").arg(parametr));  // Заполняем параметр
    static QIcon paramIcon("../Static/icons/Database.ico");  // Иконка для параметра
    paramItem->setIcon(0, paramIcon);  // Устанавливаем иконку для параметра

    // Добавляем все дочерние элементы в новый элемент
    newItem->addChild(reqItem);
    newItem->addChild(elemItem1);
    newItem->addChild(elemItem2);
    newItem->addChild(paramItem);

    // Добавляем новый элемент в основной контейнер
    itemReq->addChild(newItem);

    // ui->leftMenu->expandAll(); // Разворачивание
}



// Двойное нажатие на левое меню
/*
void LeftMenuBar::onItemDoubleClicked(QTreeWidgetItem *item, int column) {
qDebug()<<"double";
    QString itemText = item->text(0);
    bool ok;
    unsigned long long id = itemText.split(":").first().toULongLong(&ok); // Извлекаем ID
    if (ok) {
        QList<QTreeWidgetItem*> childItems = getAllChildItems(item);
        processChildItems(childItems);
    }
}*/

// Передача координат
void LeftMenuBar::processChildItems(const QList<QTreeWidgetItem*> &childItems) {

    for (const auto &child : childItems) {
        qDebug() << "Child Item:" << child->text(0);
    }
}


// Подсветка по ID
void LeftMenuBar::FocusOnItemById(unsigned long long id) {
    if(id==0) return;
    QTreeWidgetItem *target = findItemById(itemFigures, id);
    if (target) {
        QModelIndex index = menu->indexFromItem(target);
        menu->expand(index);                     // раскрываем
        menu->scrollTo(index);                   // прокручиваем к нему
        menu->setCurrentItem(target);            // выделяем его
    }
    emit showMenu();
}


QTreeWidgetItem* LeftMenuBar::findItemById(QTreeWidgetItem *parent, unsigned long long id) {
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *figure = parent->child(i);
        for (int j = 0; j < figure->childCount(); ++j) {
            QString paramText = figure->child(j)->text(0);
            if (paramText.startsWith("ID:")) {
                QString idStr = paramText.section(':', 1).trimmed();
                bool ok;
                unsigned long long currentId = idStr.toULongLong(&ok);
                if (ok && currentId == id) {
                    return figure;
                }
            }
        }
    }
    return nullptr;
}

// Элементы левого меню
QList<QTreeWidgetItem*> LeftMenuBar::getAllChildItems(QTreeWidgetItem *item) {
    QList<QTreeWidgetItem*> childItems;
    for (int i = 0; i < item->childCount(); ++i) {
        childItems.append(item->child(i)); // Добавляем дочерний элемент
        // Рекурсивно добавляем дочерние элементы
        QList<QTreeWidgetItem*> grandChildItems = getAllChildItems(item->child(i));
        childItems.append(grandChildItems);
    }
    return childItems;
}


std::vector<std::vector<QString>> LeftMenuBar::getListFigures(){
    std::vector<std::vector<QString>> figures;
    QTreeWidgetItem *itemFigures = menu->topLevelItem(1);
    for (int i = 0; i < itemFigures->childCount(); ++i) {
        QTreeWidgetItem *figureItem = itemFigures->child(i);
        QString figureName = figureItem->text(0);

        std::vector<QString> figureData;
        figureData.push_back(figureName);

        unsigned long long id = 0;
        QString idStr;

        for (int j = 0; j < figureItem->childCount(); ++j) {
            QTreeWidgetItem *paramItem = figureItem->child(j);
            QString peremen = paramItem->text(0);
            QStringList XYR = peremen.split(": ");

            if (XYR.size() != 2) continue;

            QString name = XYR[0];
            QString value = XYR[1];

            if (name == "ID") {
                idStr = value;
                id = value.toULongLong();
                figureData.push_back(value);
            }else{
                break;
            }
        }

        figures.push_back(figureData);
    }
    return figures;
}

std::vector<std::vector<QString>> LeftMenuBar::getListReq(){
    std::vector<std::vector<QString>> requirements;

    QTreeWidgetItem *itemReq = menu->topLevelItem(2);
    for (int i = 0; i < itemReq->childCount(); ++i) {
        QTreeWidgetItem *reqItem = itemReq->child(i);
        QString reqText = reqItem->text(0);

        std::vector<QString> reqData;
        reqData.push_back(reqText);

        unsigned long long id = 0;
        QString idStr;

        for (int j = 0; j < reqItem->childCount(); ++j) {
            QTreeWidgetItem *paramItem = reqItem->child(j);
            QString peremen = paramItem->text(0);
            QStringList XYR = peremen.split(": ");

            if (XYR.size() != 2) continue;

            QString name = XYR[0];
            QString value = XYR[1];

            if (name == "ID") {
                idStr = value;
                id = value.toULongLong();
                reqData.push_back(value);
            }else{
                break;
            }

        }

        requirements.push_back(reqData);
    }
    return requirements;
}

// Изменение обьектов левого меню (слот)
void LeftMenuBar::LeftMenuChanged(QTreeWidgetItem *item) {
    //Игнорируем изменения, если в процессе добавления
    if (!item || addElem) return;

    QTreeWidgetItem *parentItem = item->parent();

    // Если нет родителя — это верхний уровень (например, Figure), игнорируем
    if (!parentItem) return;

    // Проверяем, что родитель находится внутри блока Figures (itemFigures)
    if (parentItem->parent() != itemFigures) return;

    //  Всё ок — работаем с изменённым параметром
    std::vector<double> parameters;
    unsigned long long id = 0;

    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem *paramItem = parentItem->child(i);
        QString text = paramItem->text(0);

        QStringList parts = text.split(": ");
        if (parts.size() != 2) continue;

        QString name = parts[0];
        QString value = parts[1];

        if (name == "ID") {
            id = value.toULongLong();
        } else {
            bool ok = false;
            double num = value.toDouble(&ok);
            if (ok) parameters.push_back(num);
        }
    }

    emit parameterChanged(id, parameters);
}
