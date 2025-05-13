#include "Application.h"

Application::Application(int& argc, char** argv)
        : app(argc, argv),
          mainWind(),
          scene(nullptr),
          painter(nullptr),
          username("User"),
          server(username),
          client(username),
          undoRedo(100) {


    initLogger();
    initialize();
    setupAddingCommandsConnections();
    setupQTPainterConnections();
    setupServerConnections();
    setupLeftMenuConnections();
    setupRequirementsConnections();
}

int Application::exec() {
    return app.exec();
}

void Application::initLogger() {
    try {
        QString logDirPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../loggs");
        QDir logDir(logDirPath);

        if (!logDir.exists()) {
            if (!logDir.mkpath(".")) {
                throw std::runtime_error("Failed to create log directory: " + logDirPath.toStdString());
            }
        }

        QString logPath = logDirPath + "/guiLog.txt";
        logFile.setFileName(logPath);

        if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
            QString errorMsg = QString("Failed to open log file '%1': %2")
                    .arg(logPath, logFile.errorString());
            throw std::runtime_error(errorMsg.toStdString());
        }

        qInstallMessageHandler(guiLogger);

    } catch (const std::runtime_error &error) {
        mainWind.showWarning("Can't open or create log file!");
    }
}

void Application::initialize() {
    QApplication::setStyle("Fusion");
    app.setWindowIcon(QIcon(R"(..\Static\logo\logo2.ico)"));

    mainWind.show();
    mainWind.resize();

    if(mainWind.getQTPainter() == nullptr){
        mainWind.showWarning("Can't opened QTPainter");
    }
    if(mainWind.getLeftMenuBar() == nullptr){
        mainWind.showWarning("Can't opened LeftMenu");
    }

    painter = mainWind.getQTPainter();
    scene.setPainter(painter);
    leftMenu = mainWind.getLeftMenuBar();

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

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [this]() {
        leftMenu->saveToTextFile(pathTxtFileCommands);
        qDebug() << "Save to txt.";
    });

    autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, &Application::autoSave);
    autoSaveTimer->start(180000);
}

void Application::autoSave() {
    leftMenu->saveToTextFile(pathTxtFileCommands);
    qDebug() << "Save to txt.";
}

    void Application::setupQTPainterConnections() {
    if (painter) {
        // Двойное нажатие на обьект и открытие его в левом меню
        QObject::connect(painter, &QTPainter::DoubleClickOnObject, []() {
        });

        QObject::connect(painter, &QTPainter::EndMoving, [](){

        });

        // Перемещение точки
        QObject::connect(painter, &QTPainter::MovingPoint, [this](std::vector<ID> vec_id) {
            QPointF cursorNow(Scaling::logicCursorX(), Scaling::logicCursorY());

            try {
                for (std::size_t i = 0; i < vec_id.size(); ++i) {
                    scene.setPoint(vec_id[i], cursorNow.x(), cursorNow.y());
                    vecCalls.push_back([]() {
                        // leftMenu->updateParametersById(vec_id[i].get(),{Cx,Cy});
                    });
                }
            } catch (const std::exception& a) {
                mainWind.showError(a.what());
            }
            // updateState();
            painter->draw();
        });

        // Перемещение отрезка
        QObject::connect(painter, &QTPainter::MovingSection, [this](std::vector<ID> vec_id,QPointF p1,QPointF p2) {
            QPointF cursorNow(Scaling::logicCursorX(), Scaling::logicCursorY());
            QPointF delta(Scaling::logic(Scaling::getDeltaX()), Scaling::logic(Scaling::getDeltaY()));

            try {
                if(vec_id.size()==1){
                    scene.setSection(vec_id[0], cursorNow.x() + p1.x(),  cursorNow.y() + p1.y(),
                                     cursorNow.x() + p2.x(), cursorNow.y() + p2.y());
                    return;
                }
                for (std::size_t i = 0; i < vec_id.size(); ++i) {
                    scene.moveSection(vec_id[i], delta.x(),delta.y());
                    vecCalls.push_back([]() {
                        //  leftMenu->updateParametersById(vec_id[i].get(),{});
                    });
                }
            } catch (const std::exception& a) {
                mainWind.showError(a.what());
            }
            // updateState();
            painter->draw();

        });

        // Перемещение круга
        QObject::connect(painter, &QTPainter::MovingCircle, [this](std::vector<ID> vec_id,QPointF offset) {

            QPointF cursorNow(Scaling::logicCursorX(), Scaling::logicCursorY());
            QPointF delta(Scaling::logic(Scaling::getDeltaX()), Scaling::logic(Scaling::getDeltaY()));

            try {
                if(vec_id.size()==1){
                    ObjectData obj=scene.getObjectData(vec_id[0]);
                    QPointF newCenter = cursorNow + offset;

                    double radius = obj.params[2];
                    scene.setCircle(vec_id[0], newCenter.x(), newCenter.y(), radius);
                    return;
                }
                for (std::size_t i = 0; i < vec_id.size(); ++i) {
                    scene.moveCircle(vec_id[i], delta.x(),delta.y());
                    vecCalls.push_back([]() {
                        //  leftMenu->updateParametersById(vec_id[i].get(),{});
                    });
                }
            } catch (const std::exception& a) {
                mainWind.showError(a.what());
            }
            // updateState();
            painter->draw();
        });

        // Перемещение арки
        QObject::connect(painter, &QTPainter::MovingArc, [this](std::vector<ID> vec_id) {

            double dx = Scaling::logic(Scaling::getCursorDeltaX());
            double dy = Scaling::logic(Scaling::getCursorDeltaY());

            try {
                for (std::size_t i = 0; i < vec_id.size(); ++i) {
                    scene.moveArc(ID(vec_id[i]), dx, dy);
                    vecCalls.push_back([]() {
                        //   leftMenu->updateParametersById(vec_id[i].get(),{});
                    });
                }
            } catch (const std::exception& a) {
                mainWind.showError(a.what());
            }
            // updateState();
            painter->draw();
        });

        // Отрисовка точки
        QObject::connect(painter, &QTPainter::SigPoint,
                         [this](double x, double y) {
            if (ModeManager::getConnection()) {
                if (ModeManager::getFlagServer()) {
                    ObjectData point;
                    point.et = ET_POINT;
                    point.params.push_back(x);
                    point.params.push_back(y);

                    UndoRedo::CommandAddPoint* cmd = new UndoRedo::CommandAddPoint(scene, point);
                    UndoRedo::Transaction txn(cmd->description());
                    txn.addCommand(cmd);
                    undoRedo.push(std::move(txn));

                    ModeManager::setSave(false);
                    painter->draw();
                    leftMenu->addElemLeftMenu("Point", cmd->getPointID().get(), {x, y});
                    leftMenu->updateLeftMenu();
                    server.sendToClients(QString::fromStdString(scene.to_string()));
                } else {
                    client.sendCommandToServer("point " + QString::number(y) + " " +
                                               QString::number(x));
                }
            } else {
                ObjectData point;
                point.et = ET_POINT;
                point.params.push_back(x);
                point.params.push_back(y);

                UndoRedo::CommandAddPoint* cmd = new UndoRedo::CommandAddPoint(scene, point);
                UndoRedo::Transaction txn(cmd->description());
                txn.addCommand(cmd);
                undoRedo.push(std::move(txn));

                ModeManager::setSave(false);
                painter->draw();
                leftMenu->addElemLeftMenu("Point", cmd->getPointID().get(), {x, y});
                leftMenu->updateLeftMenu();
            }
        });

        // Отрисовка линии
        QObject::connect(painter, &QTPainter::SigSection,
                         [this](double startX, double startY, double endX, double endY) {
                             ObjectData section;
                             if (ModeManager::getConnection()) {
                                 if (ModeManager::getFlagServer()) {
                                     section.et = ET_SECTION;
                                     section.params.push_back(startX);
                                     section.params.push_back(startY);
                                     section.params.push_back(endX);
                                     section.params.push_back(endY);

                                     UndoRedo::CommandAddSection* cmd = new UndoRedo::CommandAddSection(scene, section);
                                     UndoRedo::Transaction txn(cmd->description());
                                     txn.addCommand(cmd);
                                     undoRedo.push(std::move(txn));
                                     ID id = cmd->getSectionID();

                                     ModeManager::setSave(false);
                                     leftMenu->addElemLeftMenu("Section", id.get(), {startX, startY, endX, endY});
                                     leftMenu->addElemLeftMenu("Point", id.get() - 1, {startX, startY});
                                     leftMenu->addElemLeftMenu("Point", id.get() - 2, {endX, endY});
                                     painter->draw();
                                     leftMenu->updateLeftMenu();
                                     server.sendToClients(QString::fromStdString(scene.to_string()));
                                 } else {
                                     client.sendCommandToServer("section " + QString::number(startX) + " " +
                                                                QString::number(startY) + " " +
                                                                QString::number(endX) + " " +
                                                                QString::number(endY));
                                 }
                             } else {
                                 section.et = ET_SECTION;
                                 section.params.push_back(startX);
                                 section.params.push_back(startY);
                                 section.params.push_back(endX);
                                 section.params.push_back(endY);

                                 UndoRedo::CommandAddSection* cmd = new UndoRedo::CommandAddSection(scene, section);
                                 UndoRedo::Transaction txn(cmd->description());
                                 txn.addCommand(cmd);
                                 undoRedo.push(std::move(txn));
                                 ID id = cmd->getSectionID();

                                 ModeManager::setSave(false);
                                 painter->draw();
                                 leftMenu->addElemLeftMenu("Point", id.get() - 1, {startX, startY});
                                 leftMenu->addElemLeftMenu("Point", id.get() - 2, {endX, endY});
                                 leftMenu->addElemLeftMenu("Section", id.get(), {startX, startY, endX, endY});
                                 leftMenu->updateLeftMenu();
                             }
                         });

        // Отрисовка круга
        QObject::connect(painter, &QTPainter::SigCircle,
                         [this](double x, double y, double radius) {
            ObjectData circle;
            if (ModeManager::getConnection()) {
                if (ModeManager::getFlagServer()) {
                    circle.et = ET_CIRCLE;
                    circle.params.push_back(x);
                    circle.params.push_back(y);
                    circle.params.push_back(radius);
                    ModeManager::setSave(false);

                    UndoRedo::CommandAddCircle* cmd = new UndoRedo::CommandAddCircle(scene, circle);
                    UndoRedo::Transaction txn(cmd->description());
                    txn.addCommand(cmd);
                    undoRedo.push(std::move(txn));
                    ID id = cmd->getCircleID();

                    leftMenu->addElemLeftMenu("Circle", id.get(), {x, y, radius});
                    leftMenu->addElemLeftMenu("Point", id.get() - 1, {x, y});
                    painter->draw();
                    leftMenu->updateLeftMenu();
                    server.sendToClients(QString::fromStdString(scene.to_string()));
                } else {
                    client.sendCommandToServer("circle " + QString::number(x) + " " +
                                               QString::number(y) + " " +
                                               QString::number(radius));
                }
            } else {
                circle.et = ET_CIRCLE;
                circle.params.push_back(x);
                circle.params.push_back(y);
                circle.params.push_back(radius);
                ModeManager::setSave(false);

                UndoRedo::CommandAddCircle* cmd = new UndoRedo::CommandAddCircle(scene, circle);
                UndoRedo::Transaction txn(cmd->description());
                txn.addCommand(cmd);
                undoRedo.push(std::move(txn));
                ID id = cmd->getCircleID();

                leftMenu->addElemLeftMenu("Circle", id.get(), {x, y, radius});
                leftMenu->addElemLeftMenu("Point", id.get() - 1, {x, y});
                painter->draw();
                leftMenu->updateLeftMenu();
            }
        });

        // Отрисовка арки
        QObject::connect(painter, &QTPainter::SigArc,
                         [this](double x0, double y0, double x1, double y1, double xc, double yc) {
                             ObjectData arc;
                             if (ModeManager::getConnection()) {
                                 if (ModeManager::getFlagServer()) {
                                     arc.et = ET_ARC;
                                     arc.params.push_back(x0);
                                     arc.params.push_back(y0);
                                     arc.params.push_back(x1);
                                     arc.params.push_back(y1);
                                     arc.params.push_back(xc);
                                     arc.params.push_back(yc);
                                     arc.params.push_back(1);

                                     UndoRedo::CommandAddArc* cmd = new UndoRedo::CommandAddArc(scene, arc);
                                     UndoRedo::Transaction txn(cmd->description());
                                     txn.addCommand(cmd);
                                     undoRedo.push(std::move(txn));
                                     ID id = cmd->getArcID();

                                     ModeManager::setSave(false);
                                     leftMenu->addElemLeftMenu("Arc", id.get(), {x0, y0, x1, y1, xc, yc});
                                     leftMenu->addElemLeftMenu("Point", id.get() - 1, {x0, y0});
                                     leftMenu->addElemLeftMenu("Point", id.get() - 2, {x1, y1});
                                     leftMenu->addElemLeftMenu("Point", id.get() - 2, {xc, yc});
                                     leftMenu->updateLeftMenu();
                                     painter->draw();
                                     server.sendToClients(QString::fromStdString(scene.to_string()));
                                 } else {
                                     client.sendCommandToServer("arc " + QString::number(x0) + " " +
                                                                QString::number(y0) + " " +
                                                                QString::number(x1) + " " +
                                                                QString::number(y1) + " " +
                                                                QString::number(xc) + " " +
                                                                QString::number(yc));
                                 }
                             } else {
                                 arc.et = ET_ARC;
                                 arc.params.push_back(x0);
                                 arc.params.push_back(y0);
                                 arc.params.push_back(x1);
                                 arc.params.push_back(y1);
                                 arc.params.push_back(xc);
                                 arc.params.push_back(yc);
                                 arc.params.push_back(1);

                                 UndoRedo::CommandAddArc* cmd = new UndoRedo::CommandAddArc(scene, arc);
                                 UndoRedo::Transaction txn(cmd->description());
                                 txn.addCommand(cmd);
                                 undoRedo.push(std::move(txn));
                                 ID id = cmd->getArcID();

                                 ModeManager::setSave(false);
                                 painter->draw();
                                 leftMenu->addElemLeftMenu("Arc", id.get(), {x0, y0, x1, y1, xc, yc});
                                 leftMenu->addElemLeftMenu("Point", id.get() - 1, {x0, y0});
                                 leftMenu->addElemLeftMenu("Point", id.get() - 2, {x1, y1});
                                 leftMenu->addElemLeftMenu("Point", id.get() - 2, {xc, yc});
                                 leftMenu->updateLeftMenu();
                             }
                         });


    }

    // Удаление элемента
    QObject::connect(&mainWind, &MainWindow::DELETE, [this]() {
        try {
            std::vector<ID> vecPoint = painter->getVecSelectedIDPoints();
            std::vector<ID> vecSection = painter->getVecSelectedIDSections();
            std::vector<ID> vecCircle = painter->getVecSelectedIDCircles();


            for (auto& sectionID : vecSection) {
                ObjectData obj = scene.getObjectData(sectionID);
                std::vector<ID> points = obj.subObjects;
                for (auto& p : points) {
                    auto it = std::find(vecPoint.begin(), vecPoint.end(), p);
                    if (it != vecPoint.end()) {
                        vecPoint.erase(it);
                    }
                }
            }

            for (auto& circleID : vecCircle) {
                ObjectData obj = scene.getObjectData(circleID);
                std::vector<ID> points = obj.subObjects;
                for (auto& p : points) {
                    auto it = std::find(vecPoint.begin(), vecPoint.end(), p);
                    if (it != vecPoint.end()) {
                        vecPoint.erase(it);
                    }
                }
            }

            UndoRedo::Transaction txn("Delete objects");

            for (std::size_t i = 0; i < vecPoint.size(); ++i) {
                UndoRedo::CommandDeletePoint *cmd = new UndoRedo::CommandDeletePoint(scene, vecPoint[i]);
                txn.addCommand(cmd);

                vecCalls.push_back([=, this]() {
                    leftMenu->removeFigureById(vecPoint[i].get());
                });
            }
            for (std::size_t i = 0; i < vecSection.size(); ++i) {
                UndoRedo::CommandDeleteSection *cmd = new UndoRedo::CommandDeleteSection(scene, vecSection[i]);
                txn.addCommand(cmd);

                vecCalls.push_back([=, this]() {
                    leftMenu->removeFigureById(vecSection[i].get());
                });
            }
            for (std::size_t i = 0; i < vecCircle.size(); ++i) {
                UndoRedo::CommandDeleteCircle *cmd = new UndoRedo::CommandDeleteCircle(scene, vecCircle[i]);
                txn.addCommand(cmd);

                vecCalls.push_back([=, this]() {
                    leftMenu->removeFigureById(vecCircle[i].get());
                });
            }

            undoRedo.push(std::move(txn));

            painter->selectedClear();
            painter->draw();
            updateState();
        } catch (std::exception& e) {
            mainWind.showError(e.what());
        }
    });

    // Изменение размера
    QObject::connect(&mainWind, &MainWindow::resize, [this]() { painter->update(); });
}

void Application::setupServerConnections() {

    QObject::connect(&client, &Client::serverShutdown, [this]() {
        mainWind.showSuccess("Server shutdown!(");
        ModeManager::setConnection(false);
        ModeManager::setFlagServer(false);

    });

    QObject::connect(&mainWind, &MainWindow::NameUsers, [this](const QString& text) {
        username = text;
        if (!ModeManager::getConnection()) {
            server.setName(username);
            client.setName(username);
        }
    });

    QObject::connect(&client, &Client::disconnectedFromServer, [this]() {
        ModeManager::setConnection(false);
        mainWind.showSuccess("You disconnected from server");
    });

    // Server button
    QObject::connect(&mainWind, &MainWindow::SigExitSession, [this]() {
        if (ModeManager::getConnection()) {
            if (ModeManager::getFlagServer()) {
                server.stopServer();
                ModeManager::setConnection(false);
                ModeManager::setFlagServer(false);
                mainWind.updateExitServerStyle(false);
                mainWind.showSuccess("Successfully stopped!");
            } else {
                client.disconnectFromServer();
                ModeManager::setConnection(false);
                mainWind.updateExitServerStyle(false);
                mainWind.showSuccess("Successfully disconnected!");
            }
        } else {
            mainWind.showError("Firstly connect to server!");
        }
    });

    QObject::connect(&mainWind, &MainWindow::SigOpenServer, [&](const QString& text) {
        if (ModeManager::getConnection() || ModeManager::getFlagServer()) {
            mainWind.showError("Firstly disconnect!");
            mainWind.updateExitServerStyle(false);
            return;
        }
        bool ok = false;
        text.toUShort(&ok);
        if (!ok) {
            mainWind.showError("Error! This is not valid port!");
            return;
        }
        server.startServer(text.toUShort(&ok));
        ModeManager::setConnection(true);
        ModeManager::setFlagServer(true);
        mainWind.updateExitServerStyle(true);
        mainWind.showSuccess("Successfully connected to server!");
    });

    QObject::connect(&mainWind, &MainWindow::SigJoinServer, [&](const QString& text) {
        if (ModeManager::getConnection() || ModeManager::getFlagServer()) {
            mainWind.showError("Firstly disconnect!");
            return;
        }
        bool ok = false;
        QStringList texts = text.split(':');
        texts[1].toUShort(&ok);
        if (!ok) {
            mainWind.showError("Error! This is not valid port!");
            return;
        }
        client.connectToServer(texts[0], texts[1].toUShort(&ok));
        ModeManager::setConnection(true);
        mainWind.showSuccess("Successfully connected to server!");
    });

    // Servers
    QObject::connect(&server, &Server::newCommandReceived, [&](const QString& cmd) {
        handler(cmd);
        updateState();
        server.sendToClients(QString::fromStdString(scene.to_string()));
    });

    QObject::connect(&client, &Client::newStateReceived, [&](/* const QString& state */) {
        // TODO scene.loadFromString(state.toStdString());
        updateState();
    });

    // Chat
    QObject::connect(&client, &Client::newChatMessageReceived, [&](const QString& msg, const QString& name) {
        mainWind.setMessage(name.toStdString(), msg.toStdString());
        updateState();
    });

    QObject::connect(&server, &Server::newMessageReceived, [&](const QString& msg, const QString& name) {
        mainWind.setMessage(name.toStdString(), msg.toStdString());
        updateState();
    });

    QObject::connect(&server, &Server::newConnection, [this]() {
        server.sendToClients(QString::fromStdString(scene.to_string()));
    });

    QObject::connect(&mainWind, &MainWindow::EnterMessage, [this](const QString& text) {
        if (ModeManager::getConnection()) {
            if (ModeManager::getFlagServer()) {
                mainWind.setMessage(username.toStdString(), text.toStdString());
                server.sendChatToClients(text, username);
            } else {
                if (!text.isEmpty()) {
                    client.sendChatMessage(text);
                }
            }
        } else {
            mainWind.showError("Firstly connect to server!");
            return;
        }
    });
}

void Application::setupRequirementsConnections() {
    try {
        // Требования
        QObject::connect(&mainWind, &MainWindow::oneRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                InputWindow window("Enter parameters: ");
                if (window.exec() == QDialog::Accepted) {
                    bool ok = false;
                    double parameters = window.getText().toDouble(&ok);
                    if (!ok) return;
                    addRequirement(ET_POINTSECTIONDIST, pairSelectedID->first, pairSelectedID->second, parameters);
                    updateState();
                }
            }
        });

        QObject::connect(&mainWind, &MainWindow::twoRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                RequirementData reqData;
                addRequirement(ET_POINTONSECTION, pairSelectedID->first, pairSelectedID->second);
                updateState();
            }
        });

        QObject::connect(&mainWind, &MainWindow::threeRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                InputWindow window("Enter parameters: ");
                if (window.exec() == QDialog::Accepted) {
                    RequirementData reqData;
                    bool ok = false;
                    double parameters = window.getText().toDouble(&ok);
                    if (!ok) return;
                    addRequirement(ET_POINTPOINTDIST, pairSelectedID->first, pairSelectedID->second, parameters);
                    updateState();
                }
            } else {
                std::vector<ID> vec_id = painter->getVecSelectedIDSections();
                if (vec_id.size() == 1) {
                    InputWindow window("Enter parameters: ");
                    if (window.exec() == QDialog::Accepted) {
                        RequirementData reqData;
                        bool ok = false;
                        double parameters = window.getText().toDouble(&ok);
                        if (!ok) return;
                        addRequirement(ET_POINTPOINTDIST, ID(vec_id[0].get() - 1), ID(vec_id[0].get() - 2), parameters);
                        updateState();
                    }
                }
            }
        });

        QObject::connect(&mainWind, &MainWindow::fourRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                addRequirement(ET_POINTONPOINT, pairSelectedID->first, pairSelectedID->second);
                updateState();
            }
        });

        QObject::connect(&mainWind, &MainWindow::fiveRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                InputWindow window("Enter parameters: ");
                if (window.exec() == QDialog::Accepted) {
                    RequirementData reqData;
                    bool ok = false;
                    double parameters = window.getText().toDouble(&ok);
                    if (!ok) return;
                    addRequirement(ET_SECTIONCIRCLEDIST, pairSelectedID->first, pairSelectedID->second, parameters);
                    updateState();
                }
            }
        });

        QObject::connect(&mainWind, &MainWindow::sixRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                    addRequirement(ET_SECTIONONCIRCLE, pairSelectedID->first, pairSelectedID->second);
                    updateState();
            }
        });

        QObject::connect(&mainWind, &MainWindow::sevenRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                addRequirement(ET_SECTIONINCIRCLE, pairSelectedID->first, pairSelectedID->second);
                updateState();
            }
        });

        QObject::connect(&mainWind, &MainWindow::eightRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                addRequirement(ET_SECTIONSECTIONPARALLEL, pairSelectedID->first, pairSelectedID->second);
                updateState();
            }

        });

        QObject::connect(&mainWind, &MainWindow::nineRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                addRequirement(ET_SECTIONSECTIONPERPENDICULAR, pairSelectedID->first, pairSelectedID->second);
                updateState();
            }
        });

        QObject::connect(&mainWind, &MainWindow::tenRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                InputWindow window("Enter parameters: ");
                if (window.exec() == QDialog::Accepted) {
                    bool ok = false;
                    double parameters = window.getText().toDouble(&ok);
                    if (!ok) return;
                    addRequirement(ET_SECTIONSECTIONANGLE, pairSelectedID->first, pairSelectedID->second, parameters);
                    updateState();
                }
            }
        });
    } catch (std::exception& e) {
        mainWind.showError(e.what());
    }
}

void Application::setupLeftMenuConnections() {
    // Изменение параметра в левом меню
    /*
    QObject::connect(leftMenu.get(), &LeftMenuBar::parameterChanged,
                     [this](unsigned long long Id, const std::vector<double> &parameters) {

                     });

    // Двойное нажатие левого меню
    QObject::connect(leftMenu.get(), &LeftMenuBar::DoubleClickLeftMenu,
                     [this](std::vector<double> &parameters, unsigned long long int ID) {

                     });
*/

}

void Application::setupAddingCommandsConnections() {
    // Console
    QObject::connect(&mainWind, &MainWindow::EnterPressed, [&](const QString& command) {
        QStringList commandParts = command.split(' ');
        if (ModeManager::getConnection()) {
            if (ModeManager::getFlagServer()) {
                handler(command);
                updateState();
                server.sendToClients(QString::fromStdString(scene.to_string()));
            } else {
                client.sendCommandToServer(command);
            }
        } else {
            handler(command);
            updateState();
        }
    });

    // Save
    QObject::connect(&mainWind, &MainWindow::projectSaved, [this](const QString& fileName, QString format) {
        if (format != (".ourp")) {
            scene.paint();
            painter->saveToImage(fileName, format);
        } else {
            std::string File = fileName.toStdString();
            scene.saveToFile(File.c_str());
            painter->draw();
        }
        mainWind.showSuccess("The project is saved!");
    });

    //Load
    QObject::connect(&mainWind, &MainWindow::LoadFile, [&](const QString& fileName) {
        try {
            painter->clear();
            std::string File = fileName.toStdString();
            scene.loadFromFile(File.c_str());
            painter->draw();
            scene.paint();
            mainWind.showSuccess("The project is loaded!");
        } catch (std::exception& e) {
            mainWind.showError(e.what());
        }
    });

    // Script
    QObject::connect(&mainWind, &MainWindow::EmitScript, [&](const QString& fileName) {
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
                    handler(qCommand);
                    server.sendToClients(QString::fromStdString(scene.to_string()));
                } else {
                    client.sendCommandToServer(qCommand);
                }
            } else {
                handler(qCommand);
            }
        }

        updateState();
        scene.paint();
    });

    // UNDO
    QObject::connect(&mainWind, &MainWindow::UNDO, [this]() {
        bool b = undoRedo.undo();
        if (!b) {
            mainWind.showError("undo failed");
        }
    });

    // REDO
    QObject::connect(&mainWind, &MainWindow::REDO, [this]() {
        bool b = undoRedo.redo();
        if (!b) {
            mainWind.showError("redo failed");
        }
    });
}

void Application::updateState() {

    painter->draw();

    auto calls = vecCalls;
    vecCalls.clear();

    QFuture<void> future = QtConcurrent::run([calls = std::move(calls)]() mutable {
        for (auto& call : calls) {
            call();
        }
    });


    leftMenu->updateLeftMenu();
}

void Application::handler(const QString &command) {

    QStringList commandParts = command.split(' ');

    if (commandParts[0] == "point" && commandParts.size() >= 3) {
        bool xOk, yOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);

        if (xOk && yOk) {
            ObjectData point;
            point.et = ET_POINT;
            point.params.push_back(x);
            point.params.push_back(y);

            UndoRedo::CommandAddPoint* cmd = new UndoRedo::CommandAddPoint(scene, point);
            UndoRedo::Transaction txn(cmd->description());
            txn.addCommand(cmd);
            undoRedo.push(std::move(txn));

            ID id = cmd->getPointID();
            ModeManager::setSave(false);
            vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Point", id.get(), {x, y});
            });
        }

    }
    else if (commandParts[0] == "section" && commandParts.size() >= 5) {
        bool xOk, yOk, zOk, rOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);
        double z = commandParts[3].toDouble(&zOk);
        double r = commandParts[4].toDouble(&rOk);
        if (xOk && yOk && zOk && rOk) {
            ObjectData section;
            section.et = ET_SECTION;
            section.params.push_back(x);
            section.params.push_back(y);
            section.params.push_back(z);
            section.params.push_back(r);

            UndoRedo::CommandAddSection* cmd = new UndoRedo::CommandAddSection(scene, section);
            UndoRedo::Transaction txn(cmd->description());
            txn.addCommand(cmd);
            undoRedo.push(std::move(txn));
            ID id = cmd->getSectionID();

            ModeManager::setSave(false);
            vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Point", id.get() - 1, {x, y});
                leftMenu->addElemLeftMenu("Point", id.get() - 2, {z, r});
                leftMenu->addElemLeftMenu("Section", id.get(), {x, y, z, r});
            });
        }
    }
    else if (commandParts[0] == "circle" && commandParts.size() >= 4) {
        bool xOk, yOk, rOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);
        double r = commandParts[3].toDouble(&rOk);

        if (xOk && yOk && rOk) {

            ObjectData circle;
            circle.et = ET_CIRCLE;
            circle.params.push_back(x);
            circle.params.push_back(y);
            circle.params.push_back(r);

            UndoRedo::CommandAddCircle* cmd = new UndoRedo::CommandAddCircle(scene, circle);
            UndoRedo::Transaction txn(cmd->description());
            txn.addCommand(cmd);
            undoRedo.push(std::move(txn));
            ID id = cmd->getCircleID();

            ModeManager::setSave(false);
            vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Circle", id.get(), {x, y, r});
                leftMenu->addElemLeftMenu("Point", id.get() - 1, {x, y});
            });
        }

    }
    else if (commandParts[0] == "arc" && commandParts.size() >= 8) {
            bool x0Ok, y0Ok, x1Ok, y1Ok, cxOk, cyOk, rOk;
            double x1 = commandParts[1].toDouble(&x0Ok);
            double y1 = commandParts[2].toDouble(&y0Ok);
            double x2 = commandParts[3].toDouble(&x1Ok);
            double y2 = commandParts[4].toDouble(&y1Ok);
            double cx = commandParts[5].toDouble(&cxOk);
            double cy = commandParts[6].toDouble(&cyOk);
            double r = commandParts[7].toDouble(&rOk);
            if (x0Ok && y0Ok && x1Ok && y1Ok && cxOk && cyOk && rOk) {
                ObjectData arc;
                arc.et = ET_ARC;
                arc.params.push_back(x1);
                arc.params.push_back(y1);
                arc.params.push_back(x2);
                arc.params.push_back(y2);
                arc.params.push_back(cx);
                arc.params.push_back(cy);
                arc.params.push_back(r);

                UndoRedo::CommandAddArc* cmd = new UndoRedo::CommandAddArc(scene, arc);
                UndoRedo::Transaction txn(cmd->description());
                txn.addCommand(cmd);
                undoRedo.push(std::move(txn));
                ID id = cmd->getArcID();

                ModeManager::setSave(false);
                vecCalls.push_back([=, this]() {
                    leftMenu->addElemLeftMenu("Point", id.get() - 3, {x1, y1});
                    leftMenu->addElemLeftMenu("Point", id.get() - 2, {x2, y2});
                    leftMenu->addElemLeftMenu("Point", id.get() - 1, {cx, cy});
                    leftMenu->addElemLeftMenu("Arc", id.get(), {x1, y1, x2, y2, cx, cy});
                });
            }

    }
    else if (commandParts[0] == "exit") {
        mainWind.close();
    }
    else if (commandParts[0] == "clear") {
        ModeManager::setSave(true);
        painter->clear();
        scene.clear();
        vecCalls.push_back([=, this]() {
            leftMenu->clearAllFigures();
            leftMenu->clearAllRequirements();
            leftMenu->updateLeftMenu();
        });
        updateState();
    }
    else if (commandParts[0] == "addReq" && commandParts.size() > 3) {
        int req = commandParts[1].toInt();
        ID obj1(commandParts[2].toInt());
        ID obj2(commandParts[3].toInt());
        double parameters = 0;

        if (commandParts.size() >= 5) {
            parameters = commandParts[4].toDouble();
        }
        try {
            switch (req) {
                case 1:
                    addRequirement(ET_POINTSECTIONDIST, obj1, obj2, parameters);
                    break;
                case 2:
                    addRequirement(ET_POINTONSECTION, obj1, obj2);
                    break;
                case 3:
                    addRequirement(ET_POINTPOINTDIST, obj1, obj2, parameters);
                    break;
                case 4:
                    addRequirement(ET_POINTONPOINT, obj1, obj2);
                    break;
                case 5:
                    addRequirement(ET_SECTIONCIRCLEDIST, obj1, obj2, parameters);
                    break;
                case 6:
                    addRequirement(ET_SECTIONONCIRCLE, obj1, obj2, parameters);
                    break;
                case 7:
                    addRequirement(ET_SECTIONINCIRCLE, obj1, obj2);
                    break;
                case 8:
                    addRequirement(ET_SECTIONSECTIONPARALLEL, obj1, obj2);
                    break;
                case 9:
                    addRequirement(ET_SECTIONSECTIONPERPENDICULAR, obj1, obj2);
                    break;
                case 10:
                    addRequirement(ET_SECTIONSECTIONANGLE, obj1, obj2, parameters);
                    break;
                default:
                    mainWind.showError("Not right number of req");
                    break;
            }
        } catch (std::exception& e) {
            mainWind.showError(e.what());
        }
    }
    else if (commandParts[0] == "delReq" && commandParts.size() > 1) {
        ID reqID(commandParts[1].toInt());
        UndoRedo::CommandDeleteRequirement* cmd = new UndoRedo::CommandDeleteRequirement(scene, reqID);
        UndoRedo::Transaction txn(cmd->description());
        txn.addCommand(cmd);
        undoRedo.push(std::move(txn));
    }
    else if (commandParts[0] == "delObj" && commandParts.size() > 1) {
        ID objID(commandParts[1].toInt());
        UndoRedo::CommandDeleteObject* cmd = new UndoRedo::CommandDeleteObject(scene, objID);
        UndoRedo::Transaction txn(cmd->description());
        txn.addCommand(cmd);
        undoRedo.push(std::move(txn));
    }
}

void Application::addRequirement(Requirement RQ, ID id1, ID id2, double parameters) {
    RequirementData reqData;
    Requirement type = RQ;
    QString name = vec_requirements[RQ];

    reqData.req = type;
    reqData.objects.push_back(id1);
    reqData.objects.push_back(id2);
    reqData.params.push_back(parameters);
    try {
        UndoRedo::CommandAddRequirement* cmd = new UndoRedo::CommandAddRequirement(scene, reqData);
        UndoRedo::Transaction txn(cmd->description());
        txn.addCommand(cmd);
        undoRedo.push(std::move(txn));

        ModeManager::setSave(false);
        vecCalls.push_back([=, this]() {
            leftMenu->addRequirementElem(name, RQ, id1.get(),
                                         id2.get(), parameters);
        });
    } catch (std::exception &e) {
        mainWind.showError(e.what());
    }
}

void Application::addRequirement(Requirement RQ, ID id1, ID id2) {
    RequirementData reqData;
    Requirement type = RQ;
    QString name = vec_requirements[RQ];

    reqData.req = type;
    reqData.objects.push_back(id1);
    reqData.objects.push_back(id2);
    try {
        UndoRedo::CommandAddRequirement* cmd = new UndoRedo::CommandAddRequirement(scene, reqData);
        UndoRedo::Transaction txn(cmd->description());
        txn.addCommand(cmd);
        undoRedo.push(std::move(txn));

        ModeManager::setSave(false);
        vecCalls.push_back([=, this]() {
            leftMenu->addRequirementElem(name, RQ, id1.get(),
                                         id2.get());
        });
    } catch (const std::exception &e) {
        mainWind.showError(e.what());
    }
}
