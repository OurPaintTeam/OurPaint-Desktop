#include "Application.h"

Application::Application(int& argc, char** argv)
        : app(argc, argv),
          mainWind(),
          scene(nullptr),
          painter(nullptr),
          username(mainWind.getUserName()),
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

    } catch (const std::runtime_error& error) {
        mainWind.showWarning("Can't open or create log file!");
    }
}


void Application::initialize() {
    QApplication::setStyle("Fusion");
    app.setWindowIcon(QIcon(R"(..\Static\logo\logo2.ico)"));

    mainWind.show();
    mainWind.resize();

    if (mainWind.getQTPainter() == nullptr) {
        mainWind.showWarning("Can't opened QTPainter");
    }
    if (mainWind.getLeftMenuBar() == nullptr) {
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

}


void Application::setupQTPainterConnections() {
    if (painter) {

        // Double-clicking on an object and opening it in the left menu
        QObject::connect(painter, &QTPainter::DoubleClickOnObject, [this](ID id) {
            // QModelIndex index=leftMenu->selectFigureById(id.get());
            // mainWind.selectLeftMenuElem(index);
        });


        QObject::connect(painter, &QTPainter::EndMoving, []() {});


        QObject::connect(painter, &QTPainter::MovingPoint, [this](const QVector<ID>& vec_id) {
            const  QPointF cursorNow = Scaling::logicCursor();
            const QPointF delta = Scaling::getCursorLogicDelta();

            try {
                if (vec_id.size() == 1) {
                    scene.setPoint(vec_id[0], cursorNow.x(), cursorNow.y());
                    leftMenu->refreshAllLinkedParams();
                    return;
                }

                for (qsizetype i = 0; i < vec_id.size(); ++i) {
                    scene.movePoint(vec_id[i], delta.x(), delta.y());
                }
                leftMenu->updateLeftMenu();
            } catch (const std::exception& a) {
                mainWind.showError(a.what());
            }

            scene.paint();
        });


        QObject::connect(painter, &QTPainter::MovingSection,
                         [this](const QVector<ID>& vec_id, const QPointF& p1, const QPointF& p2) {
                             const  QPointF cursorNow = Scaling::logicCursor();
                             const QPointF delta = Scaling::getCursorLogicDelta();

                             try {
                                 if (vec_id.size() == 1) {
                                     scene.setSection(vec_id[0], cursorNow.x() + p1.x(), cursorNow.y() + p1.y(),
                                                      cursorNow.x() + p2.x(), cursorNow.y() + p2.y());
                                     leftMenu->refreshAllLinkedParams();
                                     return;
                                 }
                                 for (qsizetype i = 0; i < vec_id.size(); ++i) {
                                     scene.moveSection(vec_id[i], delta.x(), delta.y());
                                 }
                                 leftMenu->refreshAllLinkedParams();
                             } catch (const std::exception& a) {
                                 mainWind.showError(a.what());
                             }
                             scene.paint();

                         });


        QObject::connect(painter, &QTPainter::MovingCircle, [this](const QVector<ID>& vec_id, const QPointF& offset) {

            const  QPointF cursorNow = Scaling::logicCursor();
            const QPointF delta = Scaling::getCursorLogicDelta();

            try {
                if (vec_id.size() == 1) {
                    ObjectData obj = scene.getObjectData(vec_id[0]);
                    QPointF newCenter = cursorNow + offset;

                    double radius = obj.params[2];
                    scene.setCircle(vec_id[0], newCenter.x(), newCenter.y(), radius);
                    leftMenu->refreshAllLinkedParams();
                    return;
                }
                for (qsizetype i = 0; i < vec_id.size(); ++i) {
                    scene.moveCircle(vec_id[i], delta.x(), delta.y());
                }
                leftMenu->refreshAllLinkedParams();
            } catch (const std::exception& a) {
                mainWind.showError(a.what());
            }
            scene.paint();
        });


        QObject::connect(painter, &QTPainter::MovingArc, [this](const QVector<ID>& vec_id) {
            const  QPointF cursorNow = Scaling::logicCursor();
            const QPointF delta = Scaling::getCursorLogicDelta();

            try {
                for (qsizetype i = 0; i < vec_id.size(); ++i) {
                    scene.moveArc(ID(vec_id[i]), delta.x(), delta.y());
                }
                leftMenu->refreshAllLinkedParams();
            } catch (const std::exception& a) {
                mainWind.showError(a.what());
            }
            scene.paint();
        });

        // Drawing a point
        QObject::connect(painter, &QTPainter::SigPoint,
                         [this](const QPointF& point) {
                             if (ModeManager::getConnection()) {
                                 if (ModeManager::getFlagServer()) {
                                     addPoints(point.x(), point.y());
                                     server.sendToClients(QString::fromStdString(scene.to_string()));
                                 } else {
                                     client.sendCommandToServer("point " + QString::number(point.y()) + " " +
                                                                QString::number(point.x()));
                                 }
                             } else {
                                 addPoints(point.x(), point.y());
                             }
                             updateState();
                         });

        // Drawing a line
        QObject::connect(painter, &QTPainter::SigSection,
                         [this](const QPointF& startPoint, const QPointF& endPoint) {
                             if (ModeManager::getConnection()) {
                                 if (ModeManager::getFlagServer()) {
                                     addSections(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y());
                                     server.sendToClients(QString::fromStdString(scene.to_string()));
                                 } else {
                                     client.sendCommandToServer("section " + QString::number(startPoint.x()) + " " +
                                                                QString::number(startPoint.y()) + " " +
                                                                QString::number(endPoint.x()) + " " +
                                                                QString::number(endPoint.y()));
                                 }
                             } else {
                                 addSections(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y());
                             }
                             updateState();
                         });


        // Drawing a circle
        QObject::connect(painter, &QTPainter::SigCircle,
                         [this](const QPointF& center, const double radius) {
                             if (ModeManager::getConnection()) {
                                 if (ModeManager::getFlagServer()) {
                                     addCircles(center.x(), center.y(), radius);
                                     server.sendToClients(QString::fromStdString(scene.to_string()));
                                 } else {
                                     client.sendCommandToServer("circle " + QString::number(center.x()) + " " +
                                                                QString::number(center.y()) + " " +
                                                                QString::number(radius));
                                 }
                             } else {
                                 addCircles(center.x(), center.y(), radius);
                             }
                             updateState();
                         });


        // Drawing an arcs
        QObject::connect(painter, &QTPainter::SigArc,
                         [this](const QPointF& startPoint, const QPointF& endPoint, const QPointF& centerPoint) {
                             if (ModeManager::getConnection()) {
                                 if (ModeManager::getFlagServer()) {
                                     addArcs(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y(),
                                             centerPoint.x(), centerPoint.y());
                                     server.sendToClients(QString::fromStdString(scene.to_string()));
                                 } else {
                                     client.sendCommandToServer("arc " + QString::number(startPoint.x()) + " " +
                                                                QString::number(startPoint.y()) + " " +
                                                                QString::number(endPoint.x()) + " " +
                                                                QString::number(endPoint.y()) + " " +
                                                                QString::number(centerPoint.x()) + " " +
                                                                QString::number(centerPoint.y()));
                                 }
                             } else {
                                 addArcs(startPoint.x(), startPoint.y(), endPoint.x(), endPoint.y(), centerPoint.x(),
                                         centerPoint.y());
                             }
                             updateState();
                         });


    }

    // Deleting an element
    QObject::connect(&mainWind, &MainWindow::DELETE, [this]() {
        try {
            QVector<ID> vecPoint = painter->getVecSelectedIDPoints();
            QVector<ID> vecSection = painter->getVecSelectedIDSections();
            QVector<ID> vecCircle = painter->getVecSelectedIDCircles();
            QVector<ID> vecArcs = painter->getVecSelectedIDArcs();

            deleteOwnPoints(vecPoint, vecSection, vecCircle, vecArcs);
            deleteObjects(vecPoint, vecSection, vecCircle, vecArcs);

            painter->selectedClear();
            scene.paint();
            updateState();
        } catch (std::exception& e) {
            mainWind.showError(e.what());
        }
    });

    // ctrl+c
    QObject::connect(&mainWind, &MainWindow::COPY, [this]() {
        objectsBuffer.clear();
        fillSelectedIDBuffer();
        painter->selectedClear();
        scene.paint();
    });

    // ctrl+v
    QObject::connect(&mainWind, &MainWindow::PASTE, [this]() {
        try {
            for (auto& obj: objectsBuffer) {
                ID id = scene.addObject(obj);
                if (obj.et == ET_POINT) {
                    std::vector<const double*> param = scene.getPointParams(id);
                    vecCalls.push_back([=, this]() {
                        leftMenu->addPointInLeftMenu("Point", id.get(), {param[0], param[1]});
                    });
                } else if (obj.et == ET_SECTION) {
                    std::vector<const double*> param = scene.getSectionParams(id);
                    vecCalls.push_back([=, this]() {
                        leftMenu->addSectionInLeftMenu("Section", "Point", "Point",
                                                       id.get(), id.get() - 1, id.get() - 2,
                                                       {param[0], param[1]}, {param[2], param[3]});
                    });
                } else if (obj.et == ET_CIRCLE) {
                    std::vector<const double*> param = scene.getCircleParams(id);
                    vecCalls.push_back([=, this]() {
                        leftMenu->addCircleInLeftMenu("Circle", "Center",
                                                      id.get(), id.get() - 1,
                                                      {param[0], param[1]}, *param[2]);
                    });
                } else if (obj.et == ET_ARC) {
                    std::vector<const double*> param = scene.getArcParams(id);
                    vecCalls.push_back([=, this]() {
                        leftMenu->addArcInLeftMenu("Arc", "Point", "Point", "Center",
                                                   id.get(), id.get() - 1, id.get() - 2, id.get() - 3,
                                                   {param[0], param[1]}, {param[2], param[3]}, {param[4], param[5]});
                    });
                }
            }
        } catch (std::runtime_error& error) {
            qWarning() << error.what();
            mainWind.showError("Error pasted");
        }
        updateState();
    });

    // ctrl+x
    QObject::connect(&mainWind, &MainWindow::CUT, [this]() {
        objectsBuffer.clear();
        fillSelectedIDBuffer();

        for (auto& obj: objectsBuffer) {

            vecCalls.push_back([=, this]() {
                leftMenu->removeFigureById(obj.id.get());
            });
            scene.deleteObject(obj.id);
        }

        painter->selectedClear();
        updateState();
    });

    // Changing the size
    QObject::connect(&mainWind, &MainWindow::resize, [this]() {
        painter->update();
    });
}


void Application::deleteOwnPoints(QVector<ID>& vecPoints, const QVector<ID>& vecSections, const QVector<ID>& vecCircles,
                                  const QVector<ID>& vecArcs) {

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
}


void Application::deleteObjects(QVector<ID>& vecPoints, QVector<ID>& vecSections, QVector<ID>& vecCircles,
                                QVector<ID>& vecArcs) {
    UndoRedo::Transaction txn("Delete objects");

    for (qsizetype i = 0; i < vecPoints.size(); ++i) {
        UndoRedo::CommandDeletePoint* cmd = new UndoRedo::CommandDeletePoint(scene, vecPoints[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            leftMenu->removeFigureById(vecPoints[i].get());
        });
    }
    for (qsizetype i = 0; i < vecSections.size(); ++i) {
        UndoRedo::CommandDeleteSection* cmd = new UndoRedo::CommandDeleteSection(scene, vecSections[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            leftMenu->removeFigureById(vecSections[i].get());
        });
    }
    for (qsizetype i = 0; i < vecCircles.size(); ++i) {
        UndoRedo::CommandDeleteCircle* cmd = new UndoRedo::CommandDeleteCircle(scene, vecCircles[i]);
        txn.addCommand(cmd);
        vecCalls.push_back([=, this]() {
            leftMenu->removeFigureById(vecCircles[i].get());
        });
    }
    undoRedo.push(std::move(txn));
}


void Application::fillSelectedIDBuffer() {
    QVector<ID> bufferSelectedIDPoints = painter->getVecSelectedIDPoints();
    QVector<ID> bufferSelectedIDSections = painter->getVecSelectedIDSections();
    QVector<ID> bufferSelectedIDCircles = painter->getVecSelectedIDCircles();
    QVector<ID> bufferSelectedIDArcs = painter->getVecSelectedIDArcs();

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

    QObject::connect(&client, &Client::newStateReceived, [&](const QString& state) {
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
        QObject::connect(&mainWind, &MainWindow::oneRequirements, [this]() {
            auto pairSelectedID = painter->getPairSelectedID();
            if (pairSelectedID) {
                InputWindow window("Enter parameters: ", &mainWind);
                if (window.exec() == QDialog::Accepted) {
                    bool ok = false;
                    double parameters = window.getText().toDouble(&ok);
                    if (!ok) { return; }
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
                InputWindow window("Enter parameters: ", &mainWind);
                if (window.exec() == QDialog::Accepted) {
                    RequirementData reqData;
                    bool ok = false;
                    double parameters = window.getText().toDouble(&ok);
                    if (!ok) { return; }
                    addRequirement(ET_POINTPOINTDIST, pairSelectedID->first, pairSelectedID->second, parameters);
                    updateState();
                }
            } else {
                QVector<ID> vec_id = painter->getVecSelectedIDSections();
                if (vec_id.size() == 1) {
                    InputWindow window("Enter parameters: ", &mainWind);
                    if (window.exec() == QDialog::Accepted) {
                        RequirementData reqData;
                        bool ok = false;
                        double parameters = window.getText().toDouble(&ok);
                        if (!ok) {
                            return;
                        }
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
                InputWindow window("Enter parameters: ", &mainWind);
                if (window.exec() == QDialog::Accepted) {
                    RequirementData reqData;
                    bool ok = false;
                    double parameters = window.getText().toDouble(&ok);
                    if (!ok) { return; }
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
                InputWindow window("Enter parameters: ", &mainWind);
                if (window.exec() == QDialog::Accepted) {
                    bool ok = false;
                    double parameters = window.getText().toDouble(&ok);
                    if (!ok) { return; }
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
    if (leftMenu) {
        // Changing the settings in the left menu
        QObject::connect(leftMenu, &LeftMenuBar::figureParamsChanged,
                         [this](const long long id, const std::string& type, const std::vector<double>& parameters) {
                             if (id == 0) {
                                 mainWind.showError("ID = 0");
                                 return;
                             }
                             if (type == "Point" && parameters.size() == 2) {
                                 try {
                                     scene.setPoint(ID(id), parameters[0], parameters[1]);
                                     scene.paint();
                                 } catch (const std::exception& a) {
                                     mainWind.showError(a.what());
                                 }
                             } else if (type == "Circle" && parameters.size() == 3) {
                                 try {
                                     scene.setCircle(ID(id), parameters[0], parameters[1], parameters[2]);
                                     scene.paint();
                                 } catch (const std::exception& a) {
                                     mainWind.showError(a.what());
                                 }
                             } else if (type == "Section" && parameters.size() == 4) {
                                 try {
                                     scene.setSection(ID(id), parameters[0], parameters[1], parameters[2],
                                                      parameters[3]);
                                     scene.paint();
                                 } catch (const std::exception& a) {
                                     mainWind.showError(a.what());
                                 }
                             } else if (type == "Arc" && parameters.size() == 6) {
                                 try {
                                     // TODO The arch does not store the radius!
                                     // scene.setArc(ID(id), parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5]);
                                     scene.paint();
                                 } catch (const std::exception& a) {
                                     mainWind.showError(a.what());
                                 }
                             } else {
                                 mainWind.showError("Don't move element!");
                             }
                         });


        QObject::connect(leftMenu, &LeftMenuBar::reqParamChanged,
                         [](const long long id, const double& parameter) {
                             // TODO to change the requirement parameter by ID
                         });


        // Double-tap the left menu
        QObject::connect(leftMenu, &LeftMenuBar::doubleClickLeftMenu,
                         [this](const long long int id, const std::string& type) {
                             painter->selectedElemByID(ID(id), type);
                             scene.paint();
                         });
    }

}


void Application::setupAddingCommandsConnections() {

    // Console
    QObject::connect(&mainWind, &MainWindow::EnterPressed, [&](const QString& command) {
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
            try {
                scene.saveToFile(File.c_str());
            }
            catch (std::runtime_error& error) {
                qWarning("Don't save to file");
                mainWind.showError("Don't save to file");
                return;
            }
            scene.paint();
        }
        mainWind.showSuccess("The project is saved!");
    });

    //Load
    QObject::connect(&mainWind, &MainWindow::LoadFile, [&](const QString& fileName) {
        try {
            scene.clearImage();
            std::string File = fileName.toStdString();
            scene.loadFromFile(File.c_str());

            scene.paint();
            scene.paint();
            mainWind.showSuccess("The project is loaded!");
        } catch (std::exception& e) {
            qWarning(e.what());
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
            mainWind.showError("Undo failed");
        }
    });

    // REDO
    QObject::connect(&mainWind, &MainWindow::REDO, [this]() {
        bool b = undoRedo.redo();
        if (!b) {
            mainWind.showError("Redo failed");
        }
    });
}

void Application::updateState() {

    scene.paint();

    for (auto& call: vecCalls) {
        call();
    }

    vecCalls.clear();

    leftMenu->updateLeftMenu();

    auto IDs = leftMenu->getAllFigureIDs();
    for (const auto& pair : IDs) {
        qDebug() << "ID:" << pair.first << "Name:" << pair.second;
    }

}


void Application::handler(const QString& command) {

    QStringList commandParts = command.split(' ');

    if (commandParts[0] == "point" && commandParts.size() >= 3) {
        bool xOk, yOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);

        if (xOk && yOk) {
            addPoints(x, y);
        }

    } else if (commandParts[0] == "section" && commandParts.size() >= 5) {
        bool xOk, yOk, zOk, rOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);
        double z = commandParts[3].toDouble(&zOk);
        double r = commandParts[4].toDouble(&rOk);
        if (xOk && yOk && zOk && rOk) {
            addSections(x, y, z, r);
        }
    } else if (commandParts[0] == "circle" && commandParts.size() >= 4) {
        bool xOk, yOk, rOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);
        double r = commandParts[3].toDouble(&rOk);

        if (xOk && yOk && rOk) {
            addCircles(x, y, r);
        }

    } else if (commandParts[0] == "arc" && commandParts.size() >= 7) {
        bool x0Ok, y0Ok, x1Ok, y1Ok, cxOk, cyOk;
        double x1 = commandParts[1].toDouble(&x0Ok);
        double y1 = commandParts[2].toDouble(&y0Ok);
        double x2 = commandParts[3].toDouble(&x1Ok);
        double y2 = commandParts[4].toDouble(&y1Ok);
        double cx = commandParts[5].toDouble(&cxOk);
        double cy = commandParts[6].toDouble(&cyOk);
        if (x0Ok && y0Ok && x1Ok && y1Ok && cxOk && cyOk) {
            addArcs(x1, y1, x2, y2, cx, cy);
        }

    } else if (commandParts[0] == "exit") {
        mainWind.close();
    } else if (commandParts[0] == "clear") {
        ModeManager::setSave(true);
        scene.clearImage();
        scene.clear();
        vecCalls.push_back([=, this]() {
            leftMenu->clearAllFigures();
            leftMenu->clearAllRequirements();
        });
        updateState();
    } else if (commandParts[0] == "addReq" && commandParts.size() > 3) {
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
    } else if (commandParts[0] == "delReq" && commandParts.size() > 1) {
        ID reqID(commandParts[1].toInt());
        UndoRedo::CommandDeleteRequirement* cmd = new UndoRedo::CommandDeleteRequirement(scene, reqID);
        UndoRedo::Transaction txn(cmd->description());
        txn.addCommand(cmd);
        undoRedo.push(std::move(txn));
    } else if (commandParts[0] == "delObj" && commandParts.size() > 1) {
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
            leftMenu->addRequirementElem(name, name, RQ, id1.get(),
                                         id2.get(), parameters);
        });
    } catch (std::exception& e) {
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
            leftMenu->addRequirementElem(name, name, RQ, id1.get(),
                                         id2.get());
        });
    } catch (const std::exception& e) {
        mainWind.showError(e.what());
    }
}


void Application::addPoints(double x, double y) {
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
    std::vector<const double*> param = scene.getPointParams(id);

    vecCalls.push_back([=, this]() {
        leftMenu->addPointInLeftMenu("Point", id.get(), {param[0], param[1]});
    });
}


void Application::addSections(double x0, double y0, double x1, double y1) {
    ObjectData section;
    section.et = ET_SECTION;
    section.params.push_back(x0);
    section.params.push_back(y0);
    section.params.push_back(x1);
    section.params.push_back(y1);

    UndoRedo::CommandAddSection* cmd = new UndoRedo::CommandAddSection(scene, section);
    UndoRedo::Transaction txn(cmd->description());
    txn.addCommand(cmd);
    undoRedo.push(std::move(txn));
    ID id = cmd->getSectionID();
    std::vector<const double*> param = scene.getSectionParams(id);
    ModeManager::setSave(false);
    vecCalls.push_back([=, this]() {
        leftMenu->addSectionInLeftMenu("Section", "Point", "Point",
                                       id.get(), id.get() - 1, id.get() - 2,
                                       {param[0], param[1]}, {param[2], param[3]});
    });
}


void Application::addCircles(double x, double y, double r) {
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
    std::vector<const double*> param = scene.getCircleParams(id);

    ModeManager::setSave(false);
    vecCalls.push_back([=, this]() {
        leftMenu->addCircleInLeftMenu("Circle", "Center",
                                      id.get(), id.get() - 1,
                                      {param[0], param[1]}, r);
    });
}

void Application::addArcs(double x0, double y0, double x1, double y1, double cx, double cy) {
    ObjectData arc;
    arc.et = ET_ARC;
    arc.params.push_back(x0);
    arc.params.push_back(y0);
    arc.params.push_back(x1);
    arc.params.push_back(y1);
    arc.params.push_back(cx);
    arc.params.push_back(cy);
    arc.params.push_back(1);

    UndoRedo::CommandAddArc* cmd = new UndoRedo::CommandAddArc(scene, arc);
    UndoRedo::Transaction txn(cmd->description());
    txn.addCommand(cmd);
    undoRedo.push(std::move(txn));
    ID id = cmd->getArcID();
    std::vector<const double*> param = scene.getArcParams(id);

    ModeManager::setSave(false);
    vecCalls.push_back([=, this]() {
        leftMenu->addArcInLeftMenu("Arc", "Point", "Point", "Center",
                                   id.get(), id.get() - 1, id.get() - 2, id.get() - 3,
                                   {param[0], param[1]}, {param[2], param[3]}, {param[4], param[5]});
    });
}
