#include "LeftMenuBar.h"
#include "ID.h"
#include "Objects.h"

LeftMenuBar::LeftMenuBar(QWidget* parent) {
    // Creating a model
    treeModel = new TreeModel(parent);
    rootNode = treeModel->getRootNode();

    // Creating nodes
    nothing = new TreeNode("", rootNode); // to indent from the top
    nothing->setEnable(false);
    figuresNode = new TreeNode("Figures", rootNode);
    requirementsNode = new TreeNode("Requirements", rootNode);

    rootNode->addChild(nothing);
    rootNode->addChild(figuresNode);
    rootNode->addChild(requirementsNode);

    // Font
    QFont fontCategory;
    fontCategory.setPointSize(12);
    figuresNode->setFont(fontCategory);
    requirementsNode->setFont(fontCategory);

    elem = QIcon("../Static/icons/Database.ico");
    paraam = QIcon("../Static/icons/Icon.ico");

    connect(treeModel, &TreeModel::treeModelChanged,
            this, &LeftMenuBar::paramChanged);

}

void LeftMenuBar::refreshAllLinkedParams() {
    if (!figuresNode) return;
    refreshLinkedParams(figuresNode);
}

void LeftMenuBar::refreshLinkedParams(TreeNode* node) {
    if (!node) return;

    if (node->isLinkedNumber()) {
        node->updateTextFromLinkedValue();
    }

    for (qsizetype i = 0; i < node->childCount(); ++i) {
        refreshLinkedParams(node->child(i));
    }
}

void LeftMenuBar::paramChanged(TreeNode* node) {
    if (!node) {
        return;
    }

    qlonglong id = 0;

    if (node->parent() == figuresNode || (node->parent())->parent() == figuresNode) {

        std::vector<qreal> parameters;
        std::string type;
        for (qint32 i = 0; i < node->childCount(); ++i) {
            TreeNode* child = node->child(i);
            QString text = child->data(0).toString();

            if (text.startsWith("ID: ")) {
                id = text.section(": ", 1).toULongLong();
            } else if (text.startsWith("Type: ")) {
                type = text.section(": ", 1).toStdString();
            } else {
                qreal paramValue = text.section(": ", 1).toDouble();
                parameters.push_back(paramValue);
            }
        }

        emit figureParamsChanged(id, type, parameters);
    } else if (node->parent() == requirementsNode) {
        qreal parameter = 0.0;

        for (qint32 i = 0; i < node->childCount(); ++i) {
            TreeNode* child = node->child(i);
            QString text = child->data(0).toString();

            if (text.startsWith("ID: ")) {
                id = text.section(": ", 1).toULongLong();
            } else {
                parameter = text.section(": ", 1).toDouble();
            }
        }

        emit reqParamChanged(id, parameter);
    }
}

void LeftMenuBar::doubleClickID(const QModelIndex& index) {
    QString text = index.data(Qt::DisplayRole).toString();

    qlonglong id = 0;
    std::string type;

    if (!text.startsWith("ID: ")) {
        return;
    }

    id = text.section(": ", 1).toULongLong();

    QModelIndex parentIndex = index.parent();
    qint16 row = index.row();

    if (row > 0) {
        QModelIndex typeIndex = index.model()->index(row - 1, 0, parentIndex);
        QString typeText = typeIndex.data(Qt::DisplayRole).toString();
        if (typeText.startsWith("Type: ")) {
            type = typeText.section(": ", 1).toStdString();
        }
    }

    emit doubleClickLeftMenu(id, type);
}


TreeModel* LeftMenuBar::getTreeModel() {
    return treeModel;
}

TreeNode* LeftMenuBar::createParamNode(const QString& name, const QVariant& value, TreeNode* parent,
                                       bool editable = false, bool isNumber = false, bool doubleClickable = false) {

    TreeNode* node = new TreeNode(QString("%1: %2").arg(name).arg(value.toString()), parent);
    node->setIcon(paraam);
    if (editable){
        node->setEditable(true);
    }
    if (isNumber) {
        node->setNumber(true);
    }
    if (doubleClickable) {
        node->setDoubleClick(true);
    }

    parent->addChild(node);
    return node;
}

TreeNode* LeftMenuBar::createParamNode(const QString& name, const qreal* ptr, TreeNode* parent) {

    TreeNode* node = new TreeNode(QString("%1: %2").arg(name).arg(*ptr, 0, 'g', 10), parent);
    node->setIcon(paraam);
    node->setEditable(true);
    node->setNumber(true);
    node->setDoubleClick(false);
    node->setLinkedValue(ptr);
    parent->addChild(node);
    return node;
}


TreeNode* LeftMenuBar::createPointNode(const QString& name, qlonglong id,
                                       const std::pair<const qreal*, const qreal*>& coords,
                                       TreeNode* parent) {
    TreeNode* pointNode = new TreeNode(name, parent);
    pointNode->setEditable(true);
    pointNode->setSelected(true);
    pointNode->setLiteral(true);
    pointNode->setDropEnabled(true);
    pointNode->setIcon(elem);
    parent->addChild(pointNode);

    createParamNode("Type", "Point", pointNode);
    createParamNode("ID", id, pointNode, false, false, true);
    createParamNode("x", coords.first, pointNode);
    createParamNode("y", coords.second, pointNode);
    return pointNode;
}

void LeftMenuBar::addPointInLeftMenu(const QString& namePoint, const qlonglong pID,
                                     const std::pair<const qreal*, const qreal*>& params) {

    if (!figuresNode || pID <= 0) return;
    font.setPointSize(9);
    createPointNode(namePoint, pID, params, figuresNode);
}

void LeftMenuBar::addSectionInLeftMenu(const QString& nameSection, const QString& namePoint1, const QString& namePoint2,
                                       const qlonglong secID, const qlonglong pID1, const qlonglong pID2,
                                       const std::pair<const qreal*, const qreal*>& firstParams,
                                       const std::pair<const qreal*, const qreal*>& secondParams) {
    if (!figuresNode || secID <= 0 || pID1 <= 0 || pID2 <= 0) {
        return;
    }
    constexpr quint16 SIZE = 9;
    font.setPointSize(SIZE);

    // Creating the main node of the segment
    TreeNode* sectionNode = new TreeNode(nameSection, figuresNode);
    sectionNode->setEditable(true);
    sectionNode->setSelected(true);
    sectionNode->setLiteral(true);
    sectionNode->setIcon(elem);
    figuresNode->addChild(sectionNode);

    // Adding segment parameters
    createParamNode("Type", "Section", sectionNode);
    createParamNode("ID", secID, sectionNode, false, false, true);

    // Adding two segment points
    createPointNode(namePoint1, pID1, firstParams, sectionNode);
    createPointNode(namePoint2, pID2, secondParams, sectionNode);
}

void LeftMenuBar::addArcInLeftMenu(const QString& nameArc,
                                   const QString& namePoint1, const QString& namePoint2, const QString& namePoint3,
                                   const qlonglong arcID,
                                   const qlonglong pID1, const qlonglong pID2, const qlonglong pID3,
                                   const std::pair<const qreal*, const qreal*>& firstParams,
                                   const std::pair<const qreal*, const qreal*>& secondParams,
                                   const std::pair<const qreal*, const qreal*>& thirdParams) {
    if (!figuresNode || arcID <= 0 || pID1 <= 0 || pID2 <= 0 || pID3 <= 0) {
        return;
    }
    constexpr quint16 SIZE = 9;
    font.setPointSize(SIZE);

    // The main node of the arc
    TreeNode* arcNode = new TreeNode(nameArc, figuresNode);
    arcNode->setEditable(true);
    arcNode->setSelected(true);
    arcNode->setLiteral(true);
    arcNode->setIcon(elem);
    figuresNode->addChild(arcNode);

    // Arc Characteristics
    createParamNode("Type", "Arc", arcNode);
    createParamNode("ID", arcID, arcNode, false, false, true);

    // Three arc points
    createPointNode(namePoint1, pID1, firstParams, arcNode);
    createPointNode(namePoint2, pID2, secondParams, arcNode);
    createPointNode(namePoint3, pID3, thirdParams, arcNode);
}


void LeftMenuBar::addCircleInLeftMenu(const QString& nameCircle, const QString& namePoint,
                                      const qlonglong cID, const qlonglong pID,
                                      const std::pair<const qreal*, const qreal*>& params, qreal R) {
    if (!figuresNode || cID <= 0 || pID <= 0){
        return;
    } 
    
    constexpr quint16 SIZE=9;
    font.setPointSize(SIZE);

    TreeNode* circleNode = new TreeNode(nameCircle, figuresNode);
    circleNode->setEditable(true);
    circleNode->setSelected(true);
    circleNode->setLiteral(true);
    circleNode->setIcon(elem);
    figuresNode->addChild(circleNode);

    createParamNode("Type", "Circle", circleNode);
    createParamNode("ID", cID, circleNode, false, false, true);
    createParamNode("R", R, circleNode, false, true, true);

    createPointNode(namePoint, pID, params, circleNode);
}

// Adding requirements
void LeftMenuBar::addRequirementElem(const QString& name, const QString& type, const qint32 ReqID,
                                     const qlonglong ElemID1, const qlonglong ElemID2,
                                     const qreal param) {
    if (!requirementsNode) {
        return;
    }

    constexpr quint16 SIZE = 9;
    font.setPointSize(SIZE);

    // Creating the main node of the element
    TreeNode* elemNode = new TreeNode(name, requirementsNode);
    elemNode->setEditable(true);
    elemNode->setSelected(true);
    elemNode->setLiteral(true);
    elemNode->setIcon(elem);
    requirementsNode->addChild(elemNode);


    // Adding Type as a child node
    TreeNode* ReqTypeNode = new TreeNode(QString("Type: %1").arg(type), elemNode);
    ReqTypeNode->setIcon(paraam);
    elemNode->addChild(ReqTypeNode);


    // Adding the ID as a child node
    TreeNode* ReqIDNode = new TreeNode(QString("ID: %1").arg(ReqID), elemNode);
    ReqIDNode->setIcon(paraam);
    elemNode->addChild(ReqIDNode);

    // Adding the ID as a child node
    TreeNode* ElemID1Node = new TreeNode(QString("ID₁: %1").arg(ElemID1), elemNode);
    ElemID1Node->setIcon(paraam);
    elemNode->addChild(ElemID1Node);

    // Adding the ID as a child node
    TreeNode* ElemID2Node = new TreeNode(QString("ID₂: %1").arg(ElemID2), elemNode);
    ElemID2Node->setIcon(paraam);
    elemNode->addChild(ElemID2Node);

    // Adding Param as a child node
    TreeNode* paramNode = new TreeNode(QString("Param: %1").arg(param), elemNode);
    paramNode->setEditable(true);
    paramNode->setNumber(true);
    paramNode->setIcon(paraam);
    elemNode->addChild(paramNode);
}


// Adding requirements
void LeftMenuBar::addRequirementElem(const QString& type, const QString& name, const qint32 ReqID,
                                     const qlonglong ElemID1, const qlonglong ElemID2) {
    if (!requirementsNode) { 
        return; 
    }

    constexpr quint16 SIZE = 9;
    font.setPointSize(SIZE);

    // Creating the main node of the element
    TreeNode* elemNode = new TreeNode(name, requirementsNode);
    elemNode->setIcon(elem);
    elemNode->setEditable(true);
    elemNode->setSelected(true);
    elemNode->setLiteral(true);
    requirementsNode->addChild(elemNode);

    // Adding Type as a child node
    TreeNode* ReqTypeNode = new TreeNode(QString("Type: %1").arg(type), elemNode);
    ReqTypeNode->setIcon(paraam);
    elemNode->addChild(ReqTypeNode);

    // Adding the ID as a child node
    TreeNode* ReqIDNode = new TreeNode(QString("ID: %1").arg(ReqID), elemNode);
    ReqIDNode->setIcon(paraam);
    elemNode->addChild(ReqIDNode);

    // Adding the ID as a child node
    TreeNode* ElemID1Node = new TreeNode(QString("ID₁: %1").arg(ElemID1), elemNode);
    ElemID1Node->setIcon(paraam);
    elemNode->addChild(ElemID1Node);

    // Adding the ID as a child node
    TreeNode* ElemID2Node = new TreeNode(QString("ID₂: %1").arg(ElemID2), elemNode);
    ElemID2Node->setIcon(paraam);
    elemNode->addChild(ElemID2Node);
}

void LeftMenuBar::updateLeftMenu() {
    refreshAllLinkedParams();
    treeModel->layoutChanged();
}

// Clearing all the elements
void LeftMenuBar::LeftMenuBar::clearAllRequirements() {
    if (!requirementsNode || !treeModel) { 
        return;
    }

    // Deleting all children of the node
    requirementsNode->deleteAll();
    treeModel->safelyResetModel();
    QModelIndex figuresIndex = treeModel->index(requirementsNode->row(), 0, QModelIndex());
    treeModel->dataChanged(figuresIndex, figuresIndex);
}

// Clearing all the elements
void LeftMenuBar::clearAllFigures() {
    if (!figuresNode || !treeModel) { return; }

    // Deleting all children of the node
    figuresNode->deleteAll();
    treeModel->safelyResetModel();
    QModelIndex figuresIndex = treeModel->index(figuresNode->row(), 0, QModelIndex());
    treeModel->dataChanged(figuresIndex, figuresIndex);
}


// Clearing one element by ID
void LeftMenuBar::removeFigureById(qlonglong id) {
    if (!figuresNode || !treeModel) { return; }

    for (qsizetype i = 0; i < figuresNode->childCount(); ++i) {
        TreeNode* elemNode = figuresNode->child(i);
        for (qsizetype j = 0; j < elemNode->childCount(); ++j) {
            TreeNode* child = elemNode->child(j);
            if (child->data(0).toString() == QString("ID: %1").arg(id)) {
                treeModel->removeNode(figuresNode, elemNode);
                return;
            }
        }
    }
}

QModelIndex LeftMenuBar::selectFigureById(qlonglong id) {
    if (!figuresNode || !treeModel) {
        throw std::runtime_error("Don't init node or model");
    }
    QModelIndex index;

    for (qsizetype i = 0; i < figuresNode->childCount(); ++i) {
        TreeNode* elemNode = figuresNode->child(i);

        for (qsizetype j = 0; j < elemNode->childCount(); ++j) {
            TreeNode* child = elemNode->child(j);

            if (child->data(0).toString() == QString("ID: %1").arg(id)) {
                 index = treeModel->indexFromNode(elemNode);

                if (index.isValid()) {
                    treeModel->setData(index, "Q", Qt::EditRole);
                    return index;

                }
            }
        }
    }
    return index;
}

void LeftMenuBar::onPointAdded(ID id, const double* x, const double* y) {
    addPointInLeftMenu(
            "Point",
            id.get(),
            {x, y});
    updateLeftMenu();
}

void LeftMenuBar::onSectionAdded(ID id, const double* x1, const double* y1, const double* x2, const double* y2) {
    addSectionInLeftMenu(
            "Section",
            "Point",
            "Point",
            id.get(),
            id.get() - 1,
            id.get() - 2,
            {x1, y1},
            {x2, y2});
    updateLeftMenu();
}

void LeftMenuBar::onCircleAdded(ID id, const double* x, const double* y, const double* r) {
    addCircleInLeftMenu(
            "Circle",
            "Point",
            id.get(),
            id.get() - 1,
            {x, y},
            *r);
    updateLeftMenu();
}

void LeftMenuBar::onArcAdded(ID id,
                             const double* beg_x,
                             const double* beg_y,
                             const double* end_x,
                             const double* end_y,
                             const double* center_x,
                             const double* center_y) {
    addArcInLeftMenu(
            "Arc",
            "Point",
            "Point",
            "Point",
            id.get(),
            id.get() - 1,
            id.get() - 2,
            id.get() - 3,
            {beg_x, beg_y},
            {end_x, end_y},
            {center_x, center_y});
    updateLeftMenu();
}

void LeftMenuBar::onReqAdded(const Requirement& req) {
    if (req.param.has_value()) {
        addRequirementElem(
                reqTypes[static_cast<uint8_t>(req.type)],
                reqTypes[static_cast<uint8_t>(req.type)],
                static_cast<int>(req.id.get()),
                static_cast<qlonglong>(req.obj1.get()),
                static_cast<qlonglong>(req.obj2.get()),
                static_cast<qreal>(req.param.value())
        );
    }
    else {
        addRequirementElem(
                reqTypes[static_cast<uint8_t>(req.type)],
                reqTypes[static_cast<uint8_t>(req.type)],
                static_cast<int>(req.id.get()),
                static_cast<qlonglong>(req.obj1.get()),
                static_cast<qlonglong>(req.obj2.get())
        );
    }
    updateLeftMenu();
}



