#ifndef OURPAINT_HEADERS_CONTROLLERS_LEFT_MENU_CONTROLLER_H_
#define OURPAINT_HEADERS_CONTROLLERS_LEFT_MENU_CONTROLLER_H_

#include <QObject>

class MainWindow;
class Scene;
class QTPainter;

class LeftMenuController : public QObject {
Q_OBJECT
private:
    MainWindow& _mainWind;
    Scene& _scene;
    QTPainter& _painter;

public:
    LeftMenuController(MainWindow& mainWind, Scene& scene, QTPainter& painter);

public slots:
    void onFigureParamsChanged(long long id, const std::string& type, const std::vector<double>& parameters);
    void onReqParamChanged(long long, const double&);
    void onDoubleClickLeftMenu(long long id, const std::string& type);

};

#endif // ! OURPAINT_HEADERS_CONTROLLERS_LEFT_MENU_CONTROLLER_H_