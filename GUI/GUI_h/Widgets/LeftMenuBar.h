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

public:
    void refreshAllLinkedParams();

    void refreshLinkedParams(TreeNode* node);

    explicit LeftMenuBar(QObject *parent);

    TreeModel *getTreeModel();

    void addPointInLeftMenu(const QString &namePoint,const long long pID,
                              const std::pair<const double*,const double *> &params);

    void addCircleInLeftMenu(const QString &nameCircle, const QString &namePoint,
                                          const long long cID, const long long pID,
                                          const std::pair<const double*,const double *> &params,double R);


    void addSectionInLeftMenu(const QString &nameSection,const QString &namePoint1,const QString &namePoint2,
                              const long long secID, const long long pID1, const long long pID2,
                              const std::pair<const double*,const double *> &firstParams,const std::pair<const double*,const double *> &secondParams);


    void addArcInLeftMenu(const QString &nameArc,const QString &namePoint1,const QString &namePoint2,const QString &namePoint3,
                          const long long arcID, const long long pID1, const long long pID2, const long long pID3,
                          const std::pair<const double*,const double *> &firstParams,const std::pair<const double*,const double *> &secondParams
            ,const std::pair<const double*,const double *> &thirdParams);

    // Adding requirements
    void addRequirementElem(const QString &name,const QString &type, const int ReqID, const unsigned long long ElemID1,
                            const unsigned long long ElemID2, const double param);

    // Adding requirements
    void addRequirementElem(const QString &name,const QString &type, const int ReqID, const unsigned long long ElemID1,
                            const unsigned long long ElemID2);
    TreeNode *
    createParamNode(const QString &name, const QVariant &value, TreeNode *parent, bool editable, bool isNumber,
                    bool doubleClickable);

    TreeNode *
    createParamNode(const QString &name, const double* ptr, TreeNode *parent);

    TreeNode *
    createPointNode(const QString &name, long long int id, const std::pair<const double*,const double *> &coords, TreeNode *parent);


    // Clearing all the elements
    void clearAllFigures();

    // // Clearing all the requirements
    void clearAllRequirements();

    void  updateLeftMenu();

    // Clearing one element by ID
    void removeFigureById(long long id);


    QModelIndex selectFigureById(long long int id);

public slots:
    void doubleClickID(const QModelIndex& index);
private slots:
    void paramChanged(TreeNode* node);

signals:
    void figureParamsChanged(const long long id,const std::string &type, const std::vector<double> &parameters);
    void reqParamChanged(const long long id, const double &parameter);

    void doubleClickLeftMenu(const long long id,const std::string &type);



};

#endif //OURPAINT_LEFTMENUBAR_H
