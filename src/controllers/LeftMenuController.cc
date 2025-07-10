#include "LeftMenuController.h"
#include "Scene.h"
#include "ID.h"
#include "QTPainter.h"
#include "MainWindow.h"

LeftMenuController::LeftMenuController(MainWindow& mainWind, Scene& scene, QTPainter& painter)
    : _mainWind(mainWind),
      _scene(scene),
      _painter(painter) {}

void LeftMenuController::onFigureParamsChanged(long long int id, const std::string& type,
                                             const std::vector<double>& parameters) {
    if (id == 0) {
        _mainWind.showError("ID = 0");
        return;
    }
    if (type == "Point" && parameters.size() == 2) {
        try {
            _scene.setPoint(ID(id), parameters[0], parameters[1]);
            _scene.paint();
        } catch (const std::exception& a) {
            _mainWind.showError(a.what());
        }
    } else if (type == "Circle" && parameters.size() == 3) {
        try {
            _scene.setCircle(ID(id), parameters[0], parameters[1], parameters[2]);
            _scene.paint();
        } catch (const std::exception& a) {
            _mainWind.showError(a.what());
        }
    } else if (type == "Section" && parameters.size() == 4) {
        try {
            _scene.setSection(ID(id), parameters[0], parameters[1], parameters[2],
                             parameters[3]);
            _scene.paint();
        } catch (const std::exception& a) {
            _mainWind.showError(a.what());
        }
    } else if (type == "Arc" && parameters.size() == 6) {
        try {
            // TODO The arch does not store the radius!
            // scene.setArc(ID(id), parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5]);
            _scene.paint();
        } catch (const std::exception& a) {
            _mainWind.showError(a.what());
        }
    } else {
        _mainWind.showError("Don't move element!");
    }
}

void LeftMenuController::onReqParamChanged(long long int, const double&) {}

void LeftMenuController::onDoubleClickLeftMenu(long long int id, const std::string& type) {
    _painter.selectedElemByID(ID(id), type);
    _scene.paint();
}

