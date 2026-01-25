#include "PainterController.h"

#include <QPointF>

#include "Component.h"
#include "ConsoleManager.h"
#include "Document.h"
#include "DocumentManager.h"
#include "ExceptionGuard.h"
#include "Mainwindow.h"
#include "Scaling.h"
#include "Transaction.h"
#include "UndoRedo.h"

PainterController::PainterController(DocumentManager& documentManager, MainWindow& mainWind)
        :
          _documentManager(documentManager),
          _mainWind(mainWind),
          _isStartMoving(true),
          _pre_move_object_states() {}

void PainterController::onSigPoint(const QPointF& point) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::Transaction* txn = document->commandManager().invoke("POINT", { point.x(), point.y() });
    document->undoRedoManager().push(std::move(*txn));

    ModeManager::setSave(false);
    SLOT_GUARD_MAINWIND_END
}

void PainterController::onSigSection(const QPointF& startPoint, const QPointF& endPoint) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::Transaction* txn = document->commandManager().invoke("LINE", { startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y() });
    document->undoRedoManager().push(std::move(*txn));

    ModeManager::setSave(false);
    SLOT_GUARD_MAINWIND_END
}

void PainterController::onSigCircle(const QPointF& center, const double radius) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::Transaction* txn = document->commandManager().invoke("CIRCLE", { center.x(), center.y(), radius });
    document->undoRedoManager().push(std::move(*txn));

    ModeManager::setSave(false);
    SLOT_GUARD_MAINWIND_END
}

void PainterController::onSigArc(const QPointF& startPoint, const QPointF& endPoint, const QPointF& centerPoint) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::Transaction* txn = document->commandManager().invoke("ARC", { startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y(), centerPoint.x(), centerPoint.y() });
    document->undoRedoManager().push(std::move(*txn));

    ModeManager::setSave(false);
    SLOT_GUARD_MAINWIND_END
}

void PainterController::onMovingPoint(const QVector<ID>& vec_id) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    if (_isStartMoving) {
        // I'm afraid. It's really dangerous.
        Component& c = scene.findComponentByID(vec_id[0]);
        for (auto& id : c._objectIDs) {
            _pre_move_object_states.push_back(scene.getObjectData(id));
        }
        _isStartMoving = false;
    }

    const QPointF cursorNow = Scaling::logicCursor();
    const QPointF delta = Scaling::getCursorLogicDelta();

    try {
        if (vec_id.size() == 1) {
            scene.setPoint(vec_id[0], cursorNow.x(), cursorNow.y());
            //updateState();
            return;
        }

        for (qsizetype i = 0; i < vec_id.size(); ++i) {
            scene.movePoint(vec_id[i], delta.x(), delta.y());
        }
    } catch (const std::exception& a) {
        _mainWind.showError(a.what());
    }

    scene.paint();
    ModeManager::setSave(false);
    SLOT_GUARD_MAINWIND_END
}

void PainterController::onMovingSection(const QVector<ID>& vec_id, const QPointF& p1, const QPointF& p2) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    if (_isStartMoving) {
        Component& c = scene.findComponentByID(vec_id[0]);
        for (auto& id : c._objectIDs) {
            _pre_move_object_states.push_back(scene.getObjectData(id));
        }
        _isStartMoving = false;
    }
    const  QPointF cursorNow = Scaling::logicCursor();
    const QPointF delta = Scaling::getCursorLogicDelta();

    try {
        if (vec_id.size() == 1) {
            scene.setSection(vec_id[0], cursorNow.x() + p1.x(), cursorNow.y() + p1.y(),
                             cursorNow.x() + p2.x(), cursorNow.y() + p2.y());
            //updateState();
            return;
        }
        for (qsizetype i = 0; i < vec_id.size(); ++i) {
            scene.moveSection(vec_id[i], delta.x(), delta.y());
        }
    } catch (const std::exception& a) {
        _mainWind.showError(a.what());
    }
    scene.paint();
    ModeManager::setSave(false);
    SLOT_GUARD_MAINWIND_END
}

void PainterController::onMovingCircle(const QVector<ID>& vec_id, const QPointF& offset) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    if (_isStartMoving) {
        Component& c = scene.findComponentByID(vec_id[0]);
        for (auto& id : c._objectIDs) {
            _pre_move_object_states.push_back(scene.getObjectData(id));
        }
        _isStartMoving = false;
    }

    const  QPointF cursorNow = Scaling::logicCursor();
    const QPointF delta = Scaling::getCursorLogicDelta();

    try {
        if (vec_id.size() == 1) {
            ObjectData obj = scene.getObjectData(vec_id[0]);
            QPointF newCenter = cursorNow + offset;

            double radius = obj.params[2];
            scene.setCircle(vec_id[0], newCenter.x(), newCenter.y(), radius);
            //updateState();
            return;
        }
        for (qsizetype i = 0; i < vec_id.size(); ++i) {
            scene.moveCircle(vec_id[i], delta.x(), delta.y());
        }
        //updateState();
    } catch (const std::exception& a) {
        _mainWind.showError(a.what());
    }
    scene.paint();
    ModeManager::setSave(false);
    SLOT_GUARD_MAINWIND_END
}

void PainterController::onMovingArc(const QVector<ID>& vec_id) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    if (_isStartMoving) {
        Component& c = scene.findComponentByID(vec_id[0]);
        for (auto& id : c._objectIDs) {
            _pre_move_object_states.push_back(scene.getObjectData(id));
        }
        _isStartMoving = false;
    }

    //const QPointF cursorNow = Scaling::logicCursor();
    const QPointF delta = Scaling::getCursorLogicDelta();

    try {
        for (qsizetype i = 0; i < vec_id.size(); ++i) {
            scene.moveArc(ID(vec_id[i]), delta.x(), delta.y());
        }

    } catch (const std::exception& a) {
        _mainWind.showError(a.what());
    }
    scene.paint();
    ModeManager::setSave(false);
    SLOT_GUARD_MAINWIND_END
}

void PainterController::onEndMoving() {
    SLOT_GUARD_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    UndoRedo::UndoRedoManager& undoRedo = document->undoRedoManager();
    UndoRedo::CommandMove* cmd = new UndoRedo::CommandMove(scene, _pre_move_object_states);
    UndoRedo::Transaction txn(cmd->description());
    txn.addCommand(cmd);
    undoRedo.push(std::move(txn));
    _isStartMoving = true;
    scene.paint();
    ModeManager::setSave(false);
    SLOT_GUARD_END
}
