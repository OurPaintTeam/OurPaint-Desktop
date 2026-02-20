#ifndef OURPAINT_HEADERS_CONTROLLERS_LEFT_MENU_CONTROLLER_H_
#define OURPAINT_HEADERS_CONTROLLERS_LEFT_MENU_CONTROLLER_H_

#include <QObject>

class MainWindow;
class QTPainter;
class DocumentManager;

class LeftMenuController : public QObject {
Q_OBJECT
private:
    MainWindow& _mainWind;
    DocumentManager& _documentManager;
    QTPainter& _painter;

public:
    LeftMenuController(MainWindow& mainWind, DocumentManager& documentManager, QTPainter& painter);

public slots:
    void onFigureParamsChanged(long long id, const std::string& type, const std::vector<double>& parameters);
    void onReqParamChanged(long long, const double&);
    void onDoubleClickLeftMenu(long long id, const std::string& type);
    void onRenameTab(const QString& oldName,const QString& newName);
    void onDeleteTab(const QString& fileName);
    void onOpenTab(const QString& fileName);

};

#endif // ! OURPAINT_HEADERS_CONTROLLERS_LEFT_MENU_CONTROLLER_H_