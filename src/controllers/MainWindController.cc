#include "MainWindController.h"
#include "CommandDeleteCircle.h"
#include "CommandDeletePoint.h"
#include "CommandDeleteSection.h"
#include "ConsoleManager.h"
#include "Document.h"
#include "DocumentManager.h"
#include "ExceptionGuard.h"
#include "InputWindow.h"
#include "ID.h"
#include "LeftMenuBar.h"
#include "Mainwindow.h"
#include "Objects.h"
#include "QTPainter.h"
#include "Scene.h"
#include "Transaction.h"
#include "UndoRedo.h"
#include "SaveLoadJson.h"


MainWindController::MainWindController(QTPainter& painter,
                                       DocumentManager& documentManager,
                                       MainWindow& mainWind,
                                       LeftMenuBar& lmb,
                                       SceneQtAdapter& sceneQtAdapter)
    : _painter(painter),
      _mainWind(mainWind),
      _lmb(lmb),
      _documentManager(documentManager),
      _sceneQtAdapter(sceneQtAdapter)
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

    pathTxtFileCommands = mainWind.getProjectPath() + "/CommandsFile.txt";
}

void MainWindController::onDelete() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    try {
        QVector<ID> vecPoint = _painter.getVecSelectedIDPoints();
        QVector<ID> vecSection = _painter.getVecSelectedIDLines();
        QVector<ID> vecCircle = _painter.getVecSelectedIDCircles();
        QVector<ID> vecArcs = _painter.getVecSelectedIDArcs();

        deleteOwnPoints(vecPoint, vecSection, vecCircle, vecArcs);
        deleteObjects(vecPoint, vecSection, vecCircle, vecArcs);

        _painter.selectedClear();
        scene.paint();
        updateState();
        scene.paint();
    } catch (std::exception& e) {
        _mainWind.showError(e.what());
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onCopy() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    objectsBuffer.clear();
    fillSelectedIDBuffer();
    _painter.selectedClear();
    scene.paint();
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onPaste() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    try {
        for (auto& obj: objectsBuffer) {
            ID id = scene.addObject(obj);
            if (obj.et == ObjType::ET_POINT) {
                std::vector<const double*> param = scene.getPointParams(id);
                vecCalls.push_back([=, this]() {
                    _lmb.addPointInLeftMenu("Point", id.get(), {param[0], param[1]});
                });
            } else if (obj.et == ObjType::ET_SECTION) {
                std::vector<const double*> param = scene.getSectionParams(id);
                vecCalls.push_back([=, this]() {
                    _lmb.addSectionInLeftMenu("Section", "Point", "Point",
                                                   id.get(), id.get() - 1, id.get() - 2,
                                                   {param[0], param[1]}, {param[2], param[3]});
                });
            } else if (obj.et == ObjType::ET_CIRCLE) {
                std::vector<const double*> param = scene.getCircleParams(id);
                vecCalls.push_back([=, this]() {
                    _lmb.addCircleInLeftMenu("Circle", "Center",
                                                  id.get(), id.get() - 1,
                                                  {param[0], param[1]}, *param[2]);
                });
            } else if (obj.et == ObjType::ET_ARC) {
                std::vector<const double*> param = scene.getArcParams(id);
                vecCalls.push_back([=, this]() {
                    _lmb.addArcInLeftMenu("Arc", "Point", "Point", "Center",
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
    scene.paint();
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onCut() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    objectsBuffer.clear();
    fillSelectedIDBuffer();

    for (auto& obj: objectsBuffer) {

        vecCalls.push_back([=, this]() {
            _lmb.removeFigureById(obj.id.get());
        });
        scene.deleteObject(obj.id);
    }

    _painter.selectedClear();
    updateState();
    scene.paint();
    SLOT_GUARD_MAINWIND_END
}


void MainWindController::onOneRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        InputWindow window("Enter parameters: ", &_mainWind);
        if (window.exec() == QDialog::Accepted) {
            bool ok = false;
            double parameters = window.getText().toDouble(&ok);
            if (!ok) { return; }
            //addRequirement(Requirement::ET_POINTSECTIONDIST, pairSelectedID.first, pairSelectedID.second, parameters);
            std::vector<double> vec = {1, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()), parameters};
            UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
            urm.push(std::move(*txn));
            updateState();
            scene.paint();
        }
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onTwoRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        std::vector<double> vec = {2, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        Transaction* txn = cm.invoke("REQ", { vec });
        urm.push(std::move(*txn));
        updateState();
        scene.paint();
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onThreeRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        InputWindow window("Enter parameters: ", &_mainWind);
        if (window.exec() == QDialog::Accepted) {
            //RequirementData reqData;
            bool ok = false;
            double parameters = window.getText().toDouble(&ok);
            if (!ok) { return; }
            //addRequirement(Requirement::ET_POINTPOINTDIST, pairSelectedID.first, pairSelectedID.second, parameters);
            std::vector<double> vec = {3, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()), parameters};
            UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
            urm.push(std::move(*txn));
            updateState();
            scene.paint();
        }
    } else {
        QVector<ID> vec_id = _painter.getVecSelectedIDLines();
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
                UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
                urm.push(std::move(*txn));
                updateState();
                scene.paint();
            }
        }
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onFourRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_POINTONPOINT, pairSelectedID.first, pairSelectedID.second);
        std::vector<double> vec = {4, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
        urm.push(std::move(*txn));
        updateState();
        scene.paint();
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onFiveRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        InputWindow window("Enter parameters: ", &_mainWind);
        if (window.exec() == QDialog::Accepted) {
            Requirement reqData;
            bool ok = false;
            double parameters = window.getText().toDouble(&ok);
            if (!ok) { return; }
            //addRequirement(Requirement::ET_SECTIONCIRCLEDIST, pairSelectedID.first, pairSelectedID.second, parameters);
            std::vector<double> vec = {5, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()), parameters};
            UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
            urm.push(std::move(*txn));
            updateState();
            scene.paint();
        }
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onSixRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_SECTIONONCIRCLE, pairSelectedID.first, pairSelectedID.second);
        std::vector<double> vec = {6, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
        urm.push(std::move(*txn));
        updateState();
        scene.paint();
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onSevenRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_SECTIONINCIRCLE, pairSelectedID.first, pairSelectedID.second);
        std::vector<double> vec = {7, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
        urm.push(std::move(*txn));
        updateState();
        scene.paint();
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onEightRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_SECTIONSECTIONPARALLEL, pairSelectedID->first, pairSelectedID->second);
        std::vector<double> vec = {8, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
        urm.push(std::move(*txn));
        updateState();
        scene.paint();
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onNineRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        //addRequirement(Requirement::ET_SECTIONSECTIONPERPENDICULAR, pairSelectedID.first, pairSelectedID.second);
        std::vector<double> vec = {9, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()) };
        UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
        urm.push(std::move(*txn));
        updateState();
        scene.paint();
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onTenRequirements() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    auto pairSelectedID = _painter.getPairSelectedID();
    if (pairSelectedID) {
        if (InputWindow window("Enter parameters: ", &_mainWind); window.exec() == QDialog::Accepted) {
            bool ok = false;
            const double parameters = window.getText().toDouble(&ok);
            if (!ok) { return; }
            //addRequirement(Requirement::ET_SECTIONSECTIONANGLE, pairSelectedID.first, pairSelectedID.second, parameters);
            std::vector<double> vec = {10, static_cast<double>(pairSelectedID->first.get()), static_cast<double>(pairSelectedID->second.get()), parameters};
            UndoRedo::Transaction* txn = cm.invoke("REQ", { vec });
            urm.push(std::move(*txn));
            updateState();
            scene.paint();
        }
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onEnterCommand(const QString& command) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();
    if (command == "Exit") {
        _mainWind.closeProgram();
        QCoreApplication::quit();
    }
    else {
        Transaction* txn = cm.invoke(command.toStdString());
        urm.push(std::move(*txn));

        updateState();
        scene.paint();
        _painter.draw();
        ModeManager::setSave(false);
    }

    SLOT_GUARD_MAINWIND_END
}


void MainWindController::onSaveProject(const QString& workDir) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    try {
        QDir dir(workDir);
        if (!dir.exists()) {
            QString msg = tr("Каталог проекта не существует: %1").arg(workDir);
            qDebug() << "SAVE ERROR:" << msg;
            throw std::runtime_error(msg.toStdString());
        }

        QDirIterator it(
            workDir,
            QStringList() << "*.ourp",
            QDir::Files,
            QDirIterator::Subdirectories
        );


        while (it.hasNext()) {
            const QString projectFilePath = it.next();
            QFile file(projectFilePath);

            if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                const QString msg = tr("Не удалось открыть файл для записи: %1").arg(projectFilePath);
                qDebug() << "SAVE ERROR:" << msg;
                throw std::runtime_error(msg.toStdString());
            }

            SaveLoadJson saver(scene);
            QByteArray data = QString::fromStdString(saver.to_json().dump(4)).toUtf8();

            const qint64 bytesWritten = file.write(data);
            file.close();

            if (bytesWritten != data.size()) {
                const QString msg = tr("Ошибка при записи файла: %1").arg(projectFilePath);
                qDebug() << "SAVE ERROR:" << msg;
                throw std::runtime_error(msg.toStdString());
            }

            ModeManager::setSave(true);

            qDebug() << "SAVE OK:" << projectFilePath
                     << "(" << bytesWritten << "байт)";
        }

        qDebug() << "SAVE WARNING: файлов для сохранения не найдено в" << workDir;
        _mainWind.showWarning(tr("Файлы проекта не найдены для сохранения."));

    } catch (const std::exception& e) {
        qDebug() << "SAVE EXCEPTION:" << e.what();
        _mainWind.showError(tr("Ошибка при сохранении проекта: %1").arg(e.what()));
    }

    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onOpenProject(const QString& workDir) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();

    scene.clearImage();

    try {
        QFile file(workDir);
        if (!file.open(QIODevice::ReadOnly)) {
            const QString msg = tr("Невозможно открыть файл: %1").arg(workDir);
            qDebug() << "LOAD ERROR:" << msg;
            _mainWind.showError(msg);
            throw std::runtime_error(msg.toStdString());
        }

        const QByteArray data = file.readAll();
        file.close();

        const nlohmann::json j = nlohmann::json::parse(data.constData(), nullptr, false);
        if (j.is_discarded()) {
            const QString msg = tr("Файл повреждён или не является JSON: %1").arg(workDir);
            qDebug() << "LOAD ERROR:" << msg;
            _mainWind.showError(msg);
            throw std::runtime_error(msg.toStdString());
        }

        SaveLoadJson loader(scene);
        loader.from_json(j);
        loader.loadToScene();

        _mainWind.inProjectWindow();
        scene.paint();

        const QString justName = QFileInfo(workDir).fileName();
        _lmb.addFileToProject(justName);
        _lmb.updateLeftMenu();

        qDebug() << "LOAD OK:" << workDir << "(" << data.size() << "байт)";
        _mainWind.showSuccess(tr("Проект успешно загружен!"));

        ModeManager::setProject(true);

    } catch (const std::exception& e) {
        ModeManager::setProject(false);
        _mainWind.inStartWindow();
        qDebug() << "LOAD EXCEPTION:" << e.what();
        _mainWind.showError(tr("Ошибка при загрузке проекта: %1").arg(e.what()));
    }

    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onCreateTab(const QString& tabName) {
    SLOT_GUARD_MAINWIND_BEGIN
    _documentManager.createNewDocument(tabName.toStdString());
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    scene.setObserver(&_sceneQtAdapter);
    ObjectContainer& container = scene.getObjectContainer();
    _painter.initObjectContainer(container);
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onChangeTab(const QString& tabName) {
    SLOT_GUARD_MAINWIND_BEGIN
    if (_documentManager.setActiveDocument(tabName.toStdString()) == false) {
        throw std::runtime_error("can't change tab");
    }
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    ObjectContainer& container = scene.getObjectContainer();
    _painter.initObjectContainer(container);
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onEmitScript(const QString& fileName) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Scene& scene = document->scene();

    std::string File = fileName.toStdString();
    std::ifstream Script(File);

    // TODO logs

    // TODO UNDO/REDO

    std::string command;
    while (std::getline(Script, command)) {
        Transaction* txn = cm.invoke(command);
        urm.push(std::move(*txn));
    }

    updateState();
    scene.paint();
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onUNDO() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();

    bool b = urm.undo();
    if (!b) {
        _mainWind.showError("Undo failed");
    }
    _lmb.updateLeftMenu();
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onREDO() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();

    bool b = urm.redo();
    if (!b) {
        _mainWind.showError("Redo failed");
    }
    _lmb.updateLeftMenu();
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::onEnterMessage(const QString& text) {
    SLOT_GUARD_MAINWIND_BEGIN
    if (ModeManager::getConnection()) {
        if (ModeManager::getFlagServer()) {
            _mainWind.setMessage("DEFAULT", text);
        } else {
            if (!text.isEmpty()) {

            }
        }
    } else {
        _mainWind.showError("Firstly connect to server!");
        return;
    }
    SLOT_GUARD_MAINWIND_END
}


void MainWindController::deleteOwnPoints(QVector<ID>& vecPoints, const QVector<ID>& vecSections, const QVector<ID>& vecCircles,
                                  const QVector<ID>&) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();

    for (const auto& sectionID: vecSections) {
        ObjectData obj = scene.getObjectData(sectionID);
        const std::vector<ID>& points = obj.subObjects;

        for (const auto& p: points) {
            int index = vecPoints.indexOf(p);
            if (index != -1) {
                vecPoints.removeAt(index);
            }
        }
    }

    for (auto& circleID: vecCircles) {
        ObjectData obj = scene.getObjectData(circleID);
        std::vector<ID> points = obj.subObjects;

        for (auto& p: points) {
            int index = vecPoints.indexOf(p);
            if (index != -1) {
                vecPoints.removeAt(index);
            }
        }
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::deleteObjects(QVector<ID>& vecPoints, QVector<ID>& vecSections, QVector<ID>& vecCircles,
                                QVector<ID>&) {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    Scene& scene = document->scene();

    UndoRedo::Transaction txn("Delete objects");

    for (qsizetype i = 0; i < vecPoints.size(); ++i) {
        UndoRedo::CommandDeletePoint* cmd = new UndoRedo::CommandDeletePoint(scene, vecPoints[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            _lmb.removeFigureById(vecPoints[i].get());
        });
    }
    for (qsizetype i = 0; i < vecSections.size(); ++i) {
        UndoRedo::CommandDeleteSection* cmd = new UndoRedo::CommandDeleteSection(scene, vecSections[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            _lmb.removeFigureById(vecSections[i].get());
        });
    }
    for (qsizetype i = 0; i < vecCircles.size(); ++i) {
        UndoRedo::CommandDeleteCircle* cmd = new UndoRedo::CommandDeleteCircle(scene, vecCircles[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            _lmb.removeFigureById(vecCircles[i].get());
        });
    }
    urm.push(std::move(txn));
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::fillSelectedIDBuffer() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();

    QVector<ID> bufferSelectedIDPoints = _painter.getVecSelectedIDPoints();
    QVector<ID> bufferSelectedIDSections = _painter.getVecSelectedIDLines();
    QVector<ID> bufferSelectedIDCircles = _painter.getVecSelectedIDCircles();
    QVector<ID> bufferSelectedIDArcs = _painter.getVecSelectedIDArcs();

    deleteOwnPoints(bufferSelectedIDPoints, bufferSelectedIDSections, bufferSelectedIDCircles, bufferSelectedIDArcs);

    for (auto& id: bufferSelectedIDPoints) {
        objectsBuffer.push_back(scene.getObjectData(id));
    }

    for (auto& id: bufferSelectedIDSections) {
        objectsBuffer.push_back(scene.getObjectData(id));
    }

    for (auto& id: bufferSelectedIDCircles) {
        objectsBuffer.push_back(scene.getObjectData(id));
    }

    for (auto& id: bufferSelectedIDArcs) {
        objectsBuffer.push_back(scene.getObjectData(id));
    }
    SLOT_GUARD_MAINWIND_END
}

void MainWindController::updateState() {
    SLOT_GUARD_MAINWIND_BEGIN
    Document* document = _documentManager.getActiveDocument();
    Scene& scene = document->scene();
    scene.paint();

    for (auto& call: vecCalls) {
        call();
    }

    vecCalls.clear();

    _lmb.updateLeftMenu();
    SLOT_GUARD_MAINWIND_END
}

