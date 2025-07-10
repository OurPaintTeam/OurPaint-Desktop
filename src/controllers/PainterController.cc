#include "PainterController.h"
#include "Transaction.h"
#include "UndoRedo.h"
#include "ConsoleManager.h"
#include <QPointF>
#include "Scaling.h"
#include "Mainwindow.h"

PainterController::PainterController(Scene& scene, CommandManager& commandManager, UndoRedoManager& undoRedo, MainWindow& mainWind)
        : _scene(scene),
          _commandManager(commandManager),
          _undoRedo(undoRedo),
          _mainWind(mainWind),
          _isStartMoving(true),
          _pre_move_object_states() {}

void PainterController::onSigPoint(const QPointF& point) {
    UndoRedo::Transaction* txn = _commandManager.invoke("POINT", { point.x(), point.y() });
    _undoRedo.push(std::move(*txn));
}

void PainterController::onSigSection(const QPointF& startPoint, const QPointF& endPoint) {
    UndoRedo::Transaction* txn = _commandManager.invoke("LINE", { startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y() });
    _undoRedo.push(std::move(*txn));
}

void PainterController::onSigCircle(const QPointF& center, const double radius) {
    UndoRedo::Transaction* txn = _commandManager.invoke("CIRCLE", { center.x(), center.y(), radius });
    _undoRedo.push(std::move(*txn));
}

void PainterController::onSigArc(const QPointF& startPoint, const QPointF& endPoint, const QPointF& centerPoint) {
    UndoRedo::Transaction* txn = _commandManager.invoke("ARC", { startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y(), centerPoint.x(), centerPoint.y() });
    _undoRedo.push(std::move(*txn));
}

void PainterController::onMovingPoint(const QVector<ID>& vec_id) {
    if (_isStartMoving) {
        // I'm afraid. It's really dangerous.
        Component& c = _scene.findComponentByID(vec_id[0]);
        for (auto& id : c._objectIDs) {
            _pre_move_object_states.push_back(_scene.getObjectData(id));
        }
        _isStartMoving = false;
    }

    const QPointF cursorNow = Scaling::logicCursor();
    const QPointF delta = Scaling::getCursorLogicDelta();

    try {
        if (vec_id.size() == 1) {
            _scene.setPoint(vec_id[0], cursorNow.x(), cursorNow.y());
            //updateState();
            return;
        }

        for (qsizetype i = 0; i < vec_id.size(); ++i) {
            _scene.movePoint(vec_id[i], delta.x(), delta.y());
        }
        //updateState();
        // leftMenu->updateLeftMenu();
    } catch (const std::exception& a) {
        _mainWind.showError(a.what());
    }

    _scene.paint();
}

void PainterController::onMovingSection(const QVector<ID>& vec_id, const QPointF& p1, const QPointF& p2) {
    if (_isStartMoving) {
        Component& c = _scene.findComponentByID(vec_id[0]);
        for (auto& id : c._objectIDs) {
            _pre_move_object_states.push_back(_scene.getObjectData(id));
        }
        _isStartMoving = false;
    }
    const  QPointF cursorNow = Scaling::logicCursor();
    const QPointF delta = Scaling::getCursorLogicDelta();

    try {
        if (vec_id.size() == 1) {
            _scene.setSection(vec_id[0], cursorNow.x() + p1.x(), cursorNow.y() + p1.y(),
                             cursorNow.x() + p2.x(), cursorNow.y() + p2.y());
            //updateState();
            return;
        }
        for (qsizetype i = 0; i < vec_id.size(); ++i) {
            _scene.moveSection(vec_id[i], delta.x(), delta.y());
        }
        //updateState();
    } catch (const std::exception& a) {
        _mainWind.showError(a.what());
    }
    _scene.paint();
}

void PainterController::onMovingCircle(const QVector<ID>& vec_id, const QPointF& offset) {
    if (_isStartMoving) {
        Component& c = _scene.findComponentByID(vec_id[0]);
        for (auto& id : c._objectIDs) {
            _pre_move_object_states.push_back(_scene.getObjectData(id));
        }
        _isStartMoving = false;
    }

    const  QPointF cursorNow = Scaling::logicCursor();
    const QPointF delta = Scaling::getCursorLogicDelta();

    try {
        if (vec_id.size() == 1) {
            ObjectData obj = _scene.getObjectData(vec_id[0]);
            QPointF newCenter = cursorNow + offset;

            double radius = obj.params[2];
            _scene.setCircle(vec_id[0], newCenter.x(), newCenter.y(), radius);
            //updateState();
            return;
        }
        for (qsizetype i = 0; i < vec_id.size(); ++i) {
            _scene.moveCircle(vec_id[i], delta.x(), delta.y());
        }
        //updateState();
    } catch (const std::exception& a) {
        _mainWind.showError(a.what());
    }
    _scene.paint();
}

void PainterController::onMovingArc(const QVector<ID>& vec_id) {
    if (_isStartMoving) {
        Component& c = _scene.findComponentByID(vec_id[0]);
        for (auto& id : c._objectIDs) {
            _pre_move_object_states.push_back(_scene.getObjectData(id));
        }
        _isStartMoving = false;
    }

    //const QPointF cursorNow = Scaling::logicCursor();
    const QPointF delta = Scaling::getCursorLogicDelta();

    try {
        for (qsizetype i = 0; i < vec_id.size(); ++i) {
            _scene.moveArc(ID(vec_id[i]), delta.x(), delta.y());
        }
        //updateState();
    } catch (const std::exception& a) {
        _mainWind.showError(a.what());
    }
    _scene.paint();
}

void PainterController::onEndMoving() {
    UndoRedo::CommandMove* cmd = new UndoRedo::CommandMove(_scene, _pre_move_object_states);
    UndoRedo::Transaction txn(cmd->description());
    txn.addCommand(cmd);
    _undoRedo.push(std::move(txn));
    _isStartMoving = true;
    _scene.paint();
}
