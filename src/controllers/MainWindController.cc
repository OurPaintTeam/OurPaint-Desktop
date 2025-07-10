#include "MainWindController.h"
#include "QTPainter.h"
#include "Objects.h"
#include "Scene.h"
#include "Mainwindow.h"
#include "UndoRedo.h"
#include "Transaction.h"
#include "CommandDeletePoint.h"
#include "CommandDeleteSection.h"
#include "CommandDeleteCircle.h"
#include "CommandDeleteArc.h"
#include "LeftMenuBar.h"
#include "ConsoleManager.h"
#include "Server.h"
#include "Client.h"

MainWindController::MainWindController(QTPainter* _painter,
                                       Scene& scene,
                                       MainWindow& mainWind,
                                       LeftMenuBar* lmb,
                                       UndoRedo::UndoRedoManager* urm,
                                       CommandManager& cm,
                                       Server& s,
                                       Client& c,
                                       QString& username)
    : _painter(_painter),
      _scene(scene),
      _mainWind(mainWind),
      _lmb(lmb),
      _urm(urm),
      _cm(cm),
      _s(s),
      _c(c),
      _username(username)
      {
    vec_requirements = {
            "PointSectionDist",
            "PointOnSection",
            "PointPointDist",
            "PointOnPoint",
            "SectionCircleDist",
            "SectionOnCircle",
            "SectionInCircle",
            "SectionSectionParallel",
            "SectionSectionPerpendicular",
            "SectionSectionAngle"
    };
}

void MainWindController::onDelete() {
    try {
        QVector<ID> vecPoint = _painter->getVecSelectedIDPoints();
        QVector<ID> vecSection = _painter->getVecSelectedIDSections();
        QVector<ID> vecCircle = _painter->getVecSelectedIDCircles();
        QVector<ID> vecArcs = _painter->getVecSelectedIDArcs();

        deleteOwnPoints(vecPoint, vecSection, vecCircle, vecArcs);
        deleteObjects(vecPoint, vecSection, vecCircle, vecArcs);

        _painter->selectedClear();
        _scene.paint();
        updateState();
    } catch (std::exception& e) {
        _mainWind.showError(e.what());
    }
}

void MainWindController::onCopy() {
    objectsBuffer.clear();
    fillSelectedIDBuffer();
    _painter->selectedClear();
    _scene.paint();
}

void MainWindController::onPaste() {
    try {
        for (auto& obj: objectsBuffer) {
            ID id = _scene.addObject(obj);
            if (obj.et == ObjType::ET_POINT) {
                std::vector<const double*> param = _scene.getPointParams(id);
                vecCalls.push_back([=, this]() {
                    _lmb->addPointInLeftMenu("Point", id.get(), {param[0], param[1]});
                });
            } else if (obj.et == ObjType::ET_SECTION) {
                std::vector<const double*> param = _scene.getSectionParams(id);
                vecCalls.push_back([=, this]() {
                    _lmb->addSectionInLeftMenu("Section", "Point", "Point",
                                                   id.get(), id.get() - 1, id.get() - 2,
                                                   {param[0], param[1]}, {param[2], param[3]});
                });
            } else if (obj.et == ObjType::ET_CIRCLE) {
                std::vector<const double*> param = _scene.getCircleParams(id);
                vecCalls.push_back([=, this]() {
                    _lmb->addCircleInLeftMenu("Circle", "Center",
                                                  id.get(), id.get() - 1,
                                                  {param[0], param[1]}, *param[2]);
                });
            } else if (obj.et == ObjType::ET_ARC) {
                std::vector<const double*> param = _scene.getArcParams(id);
                vecCalls.push_back([=, this]() {
                    _lmb->addArcInLeftMenu("Arc", "Point", "Point", "Center",
                                               id.get(), id.get() - 1, id.get() - 2, id.get() - 3,
                                               {param[0], param[1]}, {param[2], param[3]}, {param[4], param[5]});
                });
            }
        }
    } catch (std::runtime_error& error) {
        qWarning() << error.what();
        _mainWind.showError("Error pasted");
    }
    updateState();
}

void MainWindController::onCut() {
    objectsBuffer.clear();
    fillSelectedIDBuffer();

    for (auto& obj: objectsBuffer) {

        vecCalls.push_back([=, this]() {
            _lmb->removeFigureById(obj.id.get());
        });
        _scene.deleteObject(obj.id);
    }

    _painter->selectedClear();
    updateState();
}

void MainWindController::onResize() {
    _painter->update();
}

void MainWindController::onOneRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        InputWindow window("Enter parameters: ", &_mainWind);
        if (window.exec() == QDialog::Accepted) {
            bool ok = false;
            double parameters = window.getText().toDouble(&ok);
            if (!ok) { return; }
            //addRequirement(Requirement::ET_POINTSECTIONDIST, pairSelectedID->first, pairSelectedID->second, parameters);
            std::vector<double> vec = {1, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()), parameters};
            UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
            _urm->push(std::move(*txn));
            updateState();
        }
    }
}

void MainWindController::onTwoRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        //RequirementData reqData;
        //addRequirement(Requirement::ET_POINTONSECTION, pairSelectedID->first, pairSelectedID->second);
        std::vector<double> vec = {2, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
        _urm->push(std::move(*txn));
        updateState();
    }
}

void MainWindController::onThreeRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        InputWindow window("Enter parameters: ", &_mainWind);
        if (window.exec() == QDialog::Accepted) {
            //RequirementData reqData;
            bool ok = false;
            double parameters = window.getText().toDouble(&ok);
            if (!ok) { return; }
            //addRequirement(Requirement::ET_POINTPOINTDIST, pairSelectedID->first, pairSelectedID->second, parameters);
            std::vector<double> vec = {3, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()), parameters};
            UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
            _urm->push(std::move(*txn));
            updateState();
        }
    } else {
        QVector<ID> vec_id = _painter->getVecSelectedIDSections();
        if (vec_id.size() == 1) {
            InputWindow window("Enter parameters: ", &_mainWind);
            if (window.exec() == QDialog::Accepted) {
                Requirement reqData;
                bool ok = false;
                double parameters = window.getText().toDouble(&ok);
                if (!ok) {
                    return;
                }
                //addRequirement(Requirement::ET_POINTPOINTDIST, ID(vec_id[0].get() - 1), ID(vec_id[0].get() - 2), parameters);
                std::vector<double> vec = {3, static_cast<double>(ID(vec_id[0].get() - 1).get()), static_cast<double>(ID(vec_id[0].get() - 2).get()), parameters};
                UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
                _urm->push(std::move(*txn));
                updateState();
            }
        }
    }
}

void MainWindController::onFourRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_POINTONPOINT, pairSelectedID->first, pairSelectedID->second);
        std::vector<double> vec = {4, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
        _urm->push(std::move(*txn));
        updateState();
    }
}

void MainWindController::onFiveRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        InputWindow window("Enter parameters: ", &_mainWind);
        if (window.exec() == QDialog::Accepted) {
            Requirement reqData;
            bool ok = false;
            double parameters = window.getText().toDouble(&ok);
            if (!ok) { return; }
            //addRequirement(Requirement::ET_SECTIONCIRCLEDIST, pairSelectedID->first, pairSelectedID->second, parameters);
            std::vector<double> vec = {5, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()), parameters};
            UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
            _urm->push(std::move(*txn));
            updateState();
        }
    }
}

void MainWindController::onSixRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_SECTIONONCIRCLE, pairSelectedID->first, pairSelectedID->second);
        std::vector<double> vec = {6, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
        _urm->push(std::move(*txn));
        updateState();
    }
}

void MainWindController::onSevenRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_SECTIONINCIRCLE, pairSelectedID->first, pairSelectedID->second);
        std::vector<double> vec = {7, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
        _urm->push(std::move(*txn));
        updateState();
    }
}

void MainWindController::onEightRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_SECTIONSECTIONPARALLEL, pairSelectedID->first, pairSelectedID->second);
        std::vector<double> vec = {8, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
        _urm->push(std::move(*txn));
        updateState();
    }
}

void MainWindController::onNineRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_SECTIONSECTIONPERPENDICULAR, pairSelectedID->first, pairSelectedID->second);
        std::vector<double> vec = {9, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
        _urm->push(std::move(*txn));
        updateState();
    }
}

void MainWindController::onTenRequirements() {
    auto pairSelectedID = _painter->getPairSelectedID();
    if (pairSelectedID) {
        InputWindow window("Enter parameters: ", &_mainWind);
        if (window.exec() == QDialog::Accepted) {
            bool ok = false;
            double parameters = window.getText().toDouble(&ok);
            if (!ok) { return; }
            //addRequirement(Requirement::ET_SECTIONSECTIONANGLE, pairSelectedID->first, pairSelectedID->second, parameters);
            std::vector<double> vec = {10, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()), parameters};
            UndoRedo::Transaction* txn = _cm.invoke("REQ", { vec });
            _urm->push(std::move(*txn));
            updateState();
        }
    }
}

void MainWindController::onEnterPressed(const QString& command) {
    if (ModeManager::getConnection()) {
        if (ModeManager::getFlagServer()) {
            if (command == "Exit") {
                QCoreApplication::quit();
                return;
            }
            else {
                UndoRedo::Transaction* txn = _cm.invoke(command.toStdString());
                _urm->push(std::move(*txn));

                // TODO update left menu

                updateState();
                //server.sendToClients(QString::fromStdString(scene.to_string()));
            }
        } else {
            //client.sendCommandToServer(command);
        }
    } else {
        if (command == "Exit") {
            QCoreApplication::quit();
            return;
        }
        else {
            UndoRedo::Transaction* txn = _cm.invoke(command.toStdString());
            _urm->push(std::move(*txn));

            // TODO update left menu

            updateState();
        }
    }
}

void MainWindController::onProjectSaved(const QString& fileName, QString format) {
    if (format != (".ourp")) {
        _scene.paint();
        _painter->saveToImage(fileName, format);
    } else {
        std::string File = fileName.toStdString();
        try {
            //scene.saveToFile(File.c_str());
        }
        catch (std::runtime_error& error) {
            qWarning("Don't save to file");
            _mainWind.showError("Don't save to file");
            return;
        }
        _scene.paint();
    }
    _mainWind.showSuccess("The project is saved!");
}

void MainWindController::onLoadFile(const QString& fileName) {
    try {
        _scene.clearImage();
        std::string File = fileName.toStdString();
        //scene.loadFromFile(File.c_str());

        _scene.paint();
        _scene.paint();
        _mainWind.showSuccess("The project is loaded!");
    } catch (std::exception& e) {
        qWarning(e.what());
        _mainWind.showError(e.what());
    }
}

void MainWindController::onEmitScript(const QString& fileName) {
    std::string File = fileName.toStdString();
    std::ifstream Script(File);

    // TODO logs

    // TODO UNDO/REDO

    std::string command;

    while (std::getline(Script, command)) {
        QString qCommand = QString::fromStdString(command);

        // TODO logs

        if (ModeManager::getConnection()) {
            if (ModeManager::getFlagServer()) {
                //handler(qCommand);
                //server.sendToClients(QString::fromStdString(scene.to_string()));
            } else {
                //client.sendCommandToServer(qCommand);
            }
        } else {
            //handler(qCommand);
        }
    }

    updateState();
    _scene.paint();
}

void MainWindController::onUNDO() {
    bool b = _urm->undo();
    if (!b) {
        _mainWind.showError("Undo failed");
    }
    _lmb->updateLeftMenu();
}

void MainWindController::onREDO() {
    bool b = _urm->redo();
    if (!b) {
        _mainWind.showError("Redo failed");
    }
    _lmb->updateLeftMenu();
}

void MainWindController::onSigExitSession() {
    if (ModeManager::getConnection()) {
        if (ModeManager::getFlagServer()) {
            _s.stopServer();
            ModeManager::setConnection(false);
            ModeManager::setFlagServer(false);
            _mainWind.updateExitServerStyle(false);
            _mainWind.showSuccess("Successfully stopped!");
        } else {
            _c.disconnectFromServer();
            ModeManager::setConnection(false);
            _mainWind.updateExitServerStyle(false);
            _mainWind.showSuccess("Successfully disconnected!");
        }
    } else {
        _mainWind.showError("Firstly connect to server!");
    }
}

void MainWindController::onSigOpenServer(const QString& text) {
    if (ModeManager::getConnection() || ModeManager::getFlagServer()) {
        _mainWind.showError("Firstly disconnect!");
        _mainWind.updateExitServerStyle(false);
        return;
    }
    bool ok = false;
    text.toUShort(&ok);
    if (!ok) {
        _mainWind.showError("Error! This is not valid port!");
        return;
    }
    _s.startServer(text.toUShort(&ok));
    ModeManager::setConnection(true);
    ModeManager::setFlagServer(true);
    _mainWind.updateExitServerStyle(true);
    _mainWind.showSuccess("Successfully connected to server!");
}

void MainWindController::onSigJoinServer(const QString& text) {
    if (ModeManager::getConnection() || ModeManager::getFlagServer()) {
        _mainWind.showError("Firstly disconnect!");
        return;
    }
    bool ok = false;
    QStringList texts = text.split(':');
    texts[1].toUShort(&ok);
    if (!ok) {
        _mainWind.showError("Error! This is not valid port!");
        return;
    }
    _c.connectToServer(texts[0], texts[1].toUShort(&ok));
    ModeManager::setConnection(true);
    _mainWind.showSuccess("Successfully connected to server!");
}

void MainWindController::onEnterMessage(const QString& text) {
    if (ModeManager::getConnection()) {
        if (ModeManager::getFlagServer()) {
            _mainWind.setMessage(_username.toStdString(), text.toStdString());
            _s.sendChatToClients(text, _username);
        } else {
            if (!text.isEmpty()) {
                _c.sendChatMessage(text);
            }
        }
    } else {
        _mainWind.showError("Firstly connect to server!");
        return;
    }
}

void MainWindController::onNameUsers(const QString& text) {
    _username = text;
    if (!ModeManager::getConnection()) {
        _s.setName(_username);
        _c.setName(_username);
    }
}

void MainWindController::deleteOwnPoints(QVector<ID>& vecPoints, const QVector<ID>& vecSections, const QVector<ID>& vecCircles,
                                  const QVector<ID>&) {

    for (const auto& sectionID: vecSections) {
        ObjectData obj = _scene.getObjectData(sectionID);
        const std::vector<ID>& points = obj.subObjects;

        for (const auto& p: points) {
            int index = vecPoints.indexOf(p);
            if (index != -1) {
                vecPoints.removeAt(index);
            }
        }
    }

    for (auto& circleID: vecCircles) {
        ObjectData obj = _scene.getObjectData(circleID);
        std::vector<ID> points = obj.subObjects;

        for (auto& p: points) {
            int index = vecPoints.indexOf(p);
            if (index != -1) {
                vecPoints.removeAt(index);
            }
        }
    }
}

void MainWindController::deleteObjects(QVector<ID>& vecPoints, QVector<ID>& vecSections, QVector<ID>& vecCircles,
                                QVector<ID>&) {
    UndoRedo::Transaction txn("Delete objects");

    for (qsizetype i = 0; i < vecPoints.size(); ++i) {
        UndoRedo::CommandDeletePoint* cmd = new UndoRedo::CommandDeletePoint(_scene, vecPoints[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            _lmb->removeFigureById(vecPoints[i].get());
        });
    }
    for (qsizetype i = 0; i < vecSections.size(); ++i) {
        UndoRedo::CommandDeleteSection* cmd = new UndoRedo::CommandDeleteSection(_scene, vecSections[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            _lmb->removeFigureById(vecSections[i].get());
        });
    }
    for (qsizetype i = 0; i < vecCircles.size(); ++i) {
        UndoRedo::CommandDeleteCircle* cmd = new UndoRedo::CommandDeleteCircle(_scene, vecCircles[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            _lmb->removeFigureById(vecCircles[i].get());
        });
    }
    _urm->push(std::move(txn));
}

void MainWindController::fillSelectedIDBuffer() {
    QVector<ID> bufferSelectedIDPoints = _painter->getVecSelectedIDPoints();
    QVector<ID> bufferSelectedIDSections = _painter->getVecSelectedIDSections();
    QVector<ID> bufferSelectedIDCircles = _painter->getVecSelectedIDCircles();
    QVector<ID> bufferSelectedIDArcs = _painter->getVecSelectedIDArcs();

    deleteOwnPoints(bufferSelectedIDPoints, bufferSelectedIDSections, bufferSelectedIDCircles, bufferSelectedIDArcs);

    for (auto& id: bufferSelectedIDPoints) {
        objectsBuffer.push_back(_scene.getObjectData(id));
    }

    for (auto& id: bufferSelectedIDSections) {
        objectsBuffer.push_back(_scene.getObjectData(id));
    }

    for (auto& id: bufferSelectedIDCircles) {
        objectsBuffer.push_back(_scene.getObjectData(id));
    }

    for (auto& id: bufferSelectedIDArcs) {
        objectsBuffer.push_back(_scene.getObjectData(id));
    }
}

void MainWindController::updateState()     {
    _scene.paint();

    for (auto& call: vecCalls) {
        call();
    }

    vecCalls.clear();

    _lmb->updateLeftMenu();
}

