#include "LeftMenuController.h"

#include "Document.h"
#include "DocumentManager.h"
#include "ExceptionGuard.h"
#include "ID.h"
#include "MainWindow.h"
#include "QTPainter.h"
#include "Scene.h"

LeftMenuController::LeftMenuController(MainWindow& mainWind, DocumentManager& documentManager, QTPainter& painter)
    : _mainWind(mainWind),
      _documentManager(documentManager),
      _painter(painter) {}

void LeftMenuController::onFigureParamsChanged(long long int id, const std::string& type,
                                             const std::vector<double>& parameters) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    if (id == 0) {
        _mainWind.showError("ID = 0");
        return;
    }
    if (type == "Point" && parameters.size() == 2) {
        try {
            scene.setPoint(ID(id), parameters[0], parameters[1]);
        } catch (const std::exception& a) {
            _mainWind.showError(a.what());
        }
    } else if (type == "Circle" && parameters.size() == 3) {
        try {
            scene.setCircle(ID(id), parameters[0], parameters[1], parameters[2]);
        } catch (const std::exception& a) {
            _mainWind.showError(a.what());
        }
    } else if (type == "Section" && parameters.size() == 4) {
        try {
            scene.setSection(ID(id), parameters[0], parameters[1], parameters[2],
                             parameters[3]);
        } catch (const std::exception& a) {
            _mainWind.showError(a.what());
        }
    } else if (type == "Arc" && parameters.size() == 6) {
        try {
            // TODO The arch does not store the radius!
            // scene.setArc(ID(id), parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5]);
        } catch (const std::exception& a) {
            _mainWind.showError(a.what());
        }
    } else {
        _mainWind.showError("Don't move element!");
    }
    SLOT_GUARD_MAINWIND_END
}

void LeftMenuController::onReqParamChanged(long long int, const double&) {}

void LeftMenuController::onDoubleClickLeftMenu(long long int id, const std::string& type) {
    SLOT_GUARD_MAINWIND_BEGIN
    _painter.selectedElemByID(ID(id), type);
    SLOT_GUARD_MAINWIND_END
}

void LeftMenuController::onRenameTab(const QString& oldName,const QString& newName) {
    SLOT_GUARD_MAINWIND_BEGIN
    _mainWind.slotRenameTab(oldName,newName);
    SLOT_GUARD_MAINWIND_END
}

void LeftMenuController::onDeleteTab(const QString& fileName) {
    SLOT_GUARD_MAINWIND_BEGIN
    _mainWind.slotDeleteTab(fileName);
    SLOT_GUARD_MAINWIND_END
}

void LeftMenuController::onOpenTab(const QString& fileName) {
    SLOT_GUARD_MAINWIND_BEGIN
    _mainWind.slotOpenTab(fileName);
    SLOT_GUARD_MAINWIND_END
}
