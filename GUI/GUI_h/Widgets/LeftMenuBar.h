#ifndef OURPAINT_LEFTMENUBAR_H
#define OURPAINT_LEFTMENUBAR_H

#include <QTreeWidget>
#include <QIcon>
#include <QString>
#include <vector>
#include <string>
#include <QTimer>
#include <QWidget>
#include <QObject>
#include <QMetaObject>
#include <QFile>
#include <QDataStream>
#include <QLabel>

#include "TreeModel.h"
#include "SceneQtAdapter.h"

// A class for managing a tree

class LeftMenuBar: public QWidget  {
    Q_OBJECT
private:
    TreeModel *treeModel = nullptr;
    TreeNode *rootNode = nullptr;
    TreeNode *figuresNode = nullptr;
    TreeNode *requirementsNode = nullptr;
    TreeNode *nothing = nullptr;
    QIcon elem;
    QIcon paraam;

    QFont font;

    // TODO вынести
    QString reqTypes[10] = {
            "POINT_SECTION_DIST",
            "POINT_ON_SECTION",
            "POINT_POINT_DIST",
            "POINT_ON_POINT",
            "SECTION_CIRCLE_DIST",
            "SECTION_ON_CIRCLE",
            "SECTION_IN_CIRCLE",
            "SECTION_SECTION_PARALLEL",
            "SECTION_SECTION_PERPENDICULAR",
            "SECTION_SECTION_ANGLE"
    };

public:
    void refreshAllLinkedParams();

    void refreshLinkedParams(TreeNode* node);

    explicit LeftMenuBar(QWidget* parent = nullptr);


public slots:
    void onPointAdded(ID id, const double* x, const double* y);
    void onSectionAdded(ID id, const double* x1, const double* y1, const double* x2, const double* y2);
    void onCircleAdded(ID id, const double* x, const double* y, const double* r);
    void onArcAdded(ID id,
                    const double* beg_x,
                    const double* beg_y,
                    const double* end_x,
                    const double* end_y,
                    const double* center_x,
                    const double* center_y);
    void onReqAdded(const Requirement& req);

public:
    TreeModel *getTreeModel();

    void addPointInLeftMenu(const QString &namePoint, const qlonglong pID,
                              const std::pair<const qreal*,const qreal *> &params);

    void addCircleInLeftMenu(const QString &nameCircle, const QString &namePoint,
                                          const qlonglong cID, const qlonglong pID,
                                          const std::pair<const qreal*,const qreal *> &params,qreal R);


    void addSectionInLeftMenu(const QString &nameSection,const QString &namePoint1,const QString &namePoint2,
                              const qlonglong secID, const qlonglong pID1, const qlonglong pID2,
                              const std::pair<const qreal*,const qreal *> &firstParams,const std::pair<const qreal*,const qreal *> &secondParams);


    void addArcInLeftMenu(const QString &nameArc,const QString &namePoint1,const QString &namePoint2,const QString &namePoint3,
                          const qlonglong arcID, const qlonglong pID1, const qlonglong pID2, const qlonglong pID3,
                          const std::pair<const qreal*,const qreal *> &firstParams,const std::pair<const qreal*,const qreal *> &secondParams
            ,const std::pair<const qreal*,const qreal *> &thirdParams);

    // Adding requirements
    void addRequirementElem(const QString &name,const QString &type, const int ReqID, const qlonglong ElemID1,
                            const qlonglong ElemID2, const qreal param);

    // Adding requirements
    void addRequirementElem(const QString &name,const QString &type, const int ReqID, const qlonglong ElemID1, const qlonglong ElemID2);

    TreeNode *
    createParamNode(const QString &name, const QVariant &value, TreeNode *parent, bool editable, bool isNumber,
                    bool doubleClickable);

    TreeNode *
    createParamNode(const QString &name, const qreal* ptr, TreeNode *parent);

    TreeNode *
    createPointNode(const QString &name,const qlonglong id, const std::pair<const qreal*,const qreal *> &coords, TreeNode *parent);


    // Clearing all the elements
    void clearAllFigures();

    // // Clearing all the requirements
    void clearAllRequirements();

    void  updateLeftMenu();

    // Clearing one element by ID
    void removeFigureById(const qlonglong id);

    QModelIndex selectFigureById(const qlonglong id);

public slots:
    void doubleClickID(const QModelIndex& index);
private slots:
    void paramChanged(TreeNode* node);

signals:
    void figureParamsChanged(const qlonglong id,const std::string &type, const std::vector<qreal> &parameters);
    void reqParamChanged(const qlonglong id, const qreal &parameter);
    void doubleClickLeftMenu(const qlonglong id,const std::string &type);


};

#endif //OURPAINT_LEFTMENUBAR_H
