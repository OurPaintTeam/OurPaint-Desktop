#include "Application.h"

Application::Application(int &argc, char **argv)
        : app(argc, argv),
          mainWind(),
          painter(nullptr),
          scene(nullptr),
          username("User"),
          server(username),
          client(username) {


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

void Application::initialize() {
    app.setWindowIcon(QIcon(R"(..\Static\logo\logo2.ico)"));
    QApplication::setStyle("Fusion");

    painter.reset(mainWind.getQTPainter());
    Scene scene_copy(painter.get());
    leftMenu.reset(mainWind.getLeftMenuBar());
    scene = scene_copy;
    scene.setPainter(painter.get());
    mainWind.show();
    mainWind.resize();
}

void Application::setupQTPainterConnections(){
    if (painter) {
        // Двойное нажатие на обьект и открытие его в левом меню
        QObject::connect(painter.get(), &QTPainter::DoubleClickOnObject, [this](ID id) {
            unsigned long long obj = id.get();
            //  leftMenu->FocusOnItemById(obj);
        });

        // Перемещение точки
        QObject::connect(painter.get(), &QTPainter::MovingPoint, [this](std::vector<ID> vec_id) {

            double Cx = Scaling::logicCursorX();
            double Cy = Scaling::logicCursorY();

            double dx = Scaling::logic(Scaling::getCursorDeltaX());
            double dy = Scaling::logic(Scaling::getCursorDeltaY());

            try {
                for(int i=0;i<vec_id.size();++i) {
                scene.moveObject(vec_id[i], dx, dy);
                }
            } catch (const std::exception &a) {
                mainWind.showError("Zheny kosyk ");
            }
            painter->draw();
            // TODO сделать изменение обьекта по айди
            //   leftMenu->parameterChanged(id.get(),)
        });
        // Перемещение отрезка
        QObject::connect(painter.get(), &QTPainter::MovingSection, [this](std::vector<ID> vec_id) {

            double Cx = Scaling::logicCursorX();
            double Cy = Scaling::logicCursorY();

            double dx = Scaling::logic(Scaling::getCursorDeltaX());
            double dy = Scaling::logic(Scaling::getCursorDeltaY());

            try {
                for(int i=0;i<vec_id.size();++i) {
                    scene.moveObject(vec_id[i], dx, dy);
                }
            } catch (const std::exception &a) {
                mainWind.showError("Zheny kosyk ");
            }
            painter->draw();
            // TODO сделать изменение обьекта по айди
            //   leftMenu->parameterChanged(id.get(),)
        });
        // Перемещение круга
        QObject::connect(painter.get(), &QTPainter::MovingCircle, [this](std::vector<ID> vec_id) {

            double Cx = Scaling::logicCursorX();
            double Cy = Scaling::logicCursorY();

            double dx = Scaling::logic(Scaling::getCursorDeltaX());
            double dy = Scaling::logic(Scaling::getCursorDeltaY());


            try {
                for(int i=0;i<vec_id.size();++i) {
                scene.moveObject(ID(vec_id[i]), dx, dy);
                }
            } catch (const std::exception &a) {
                mainWind.showError("Zheny kosyk ");
            }
            painter->draw();
            // TODO сделать изменение обьекта по айди
            //   leftMenu->parameterChanged(id.get(),)
        });


        // Отрисовка точки
        QObject::connect(painter.get(), &QTPainter::SigPoint, [this](double x, double y) {
            if (ModeManager::getConnection()) {
                if (ModeManager::getFlagServer()) {
                    ObjectData point;
                    ID id = scene.addObject(point);
                    point.et = ET_POINT;
                    point.params.push_back(x);
                    point.params.push_back(y);
                    ModeManager::setSave(false);
                    painter->draw();
                    leftMenu->addElemLeftMenu("Point",id.get(),{x,y});
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
                ID id = scene.addObject(point);
                ModeManager::setSave(false);
                painter->draw();
                leftMenu->addElemLeftMenu("Point",id.get(),{x,y});
            }
        });

        // Отрисовка круга
        QObject::connect(painter.get(), &QTPainter::SigCircle, [this](double x, double y, double radius) {
            ObjectData circle;
            if (ModeManager::getConnection()) {
                if (ModeManager::getFlagServer()) {
                    circle.et = ET_CIRCLE;
                    circle.params.push_back(x);
                    circle.params.push_back(y);
                    circle.params.push_back(radius);
                    ModeManager::setSave(false);
                    ID id = scene.addObject(circle);
                    leftMenu->addElemLeftMenu("Circle",id.get(),{x,y,radius});
                    leftMenu->addElemLeftMenu("Point",id.get()-1,{x,y});
                    painter->draw();
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
                ID id = scene.addObject(circle);
                leftMenu->addElemLeftMenu("Circle",id.get(),{x,y,radius});
                leftMenu->addElemLeftMenu("Point",id.get()-1,{x,y});
                painter->draw();
            }
        });

        // Отрисовка линии
        QObject::connect(painter.get(), &QTPainter::SigSection,
                         [this](double startX, double startY, double endX, double endY) {

                             ObjectData section;
                             if (ModeManager::getConnection()) {
                                 if (ModeManager::getFlagServer()) {
                                     section.et = ET_SECTION;
                                     section.params.push_back(startX);
                                     section.params.push_back(startY);
                                     section.params.push_back(endX);
                                     section.params.push_back(endY);
                                     ID id = scene.addObject(section);
                                     ModeManager::setSave(false);
                                     leftMenu->addElemLeftMenu("Section",id.get(),{startX,startY,endX,endY});
                                     leftMenu->addElemLeftMenu("Point",id.get()-1,{startX,startY});
                                     leftMenu->addElemLeftMenu("Point",id.get()-2,{endX,endY});
                                     painter->draw();
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
                                 ID id = scene.addObject(section);
                                 ModeManager::setSave(false);
                                 painter->draw();
                                 leftMenu->addElemLeftMenu("Point",id.get()-1,{startX,startY});
                                 leftMenu->addElemLeftMenu("Point",id.get()-2,{endX,endY});
                                 leftMenu->addElemLeftMenu("Section",id.get(),{startX,startY,endX,endY});
                             }
                         });

        // Отрисовка сектора
        QObject::connect(painter.get(), &QTPainter::SigSector,
                         [this]() {});

        // Отрисовка арки
        QObject::connect(painter.get(), &QTPainter::SigArc,
                         [this]() {});


    }

    // Удаление элемента
    QObject::connect(&mainWind, &MainWindow::DELETE, [this]() {

        std::vector<ID> vecPoint = painter->getVecIDPoints();
        std::vector<ID> vecSection = painter->getVecIDSections();
        std::vector<ID> vecCircle = painter->getVecIDCircles();

        for (int i = 0; i < vecPoint.size(); ++i) {
            scene.deletePoint(vecPoint[i]);
            leftMenu->removeFigureById(vecPoint[i].get());
        }
        for (int i = 0; i < vecSection.size(); ++i) {
            scene.deleteSection(vecSection[i]);
            leftMenu->removeFigureById(vecSection[i].get());
        }
        for (int i = 0; i < vecCircle.size(); ++i) {
            scene.deleteCircle(vecCircle[i]);
            leftMenu->removeFigureById(vecCircle[i].get());
        }
        painter->selectedClear();
        painter->draw();
    });

    // Изменение размера
    QObject::connect(&mainWind, &MainWindow::resize, [this]() {painter->update();});
}

void Application::setupServerConnections(){
    QObject::connect(&client, &Client::serverShutdown, [this]() {
        mainWind.showSuccess("Server shutdown!(");
        ModeManager::setConnection(false);
        ModeManager::setFlagServer(false);

    });
    QObject::connect(&mainWind, &MainWindow::NameUsers, [this](const QString &text) {
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
                mainWind.showSuccess("Successfully stopped!");
            } else {
                client.disconnectFromServer();
                ModeManager::setConnection(false);
                mainWind.showSuccess("Successfully disconnected!");
            }
        } else {
            mainWind.showError("Firstly connect to server!");
        }
    });
    QObject::connect(&mainWind, &MainWindow::SigOpenServer, [&](const QString &text) {
        if (ModeManager::getConnection() || ModeManager::getFlagServer()) {
            mainWind.showError("Firstly disconnect!");
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
        mainWind.showSuccess("Successfully connected to server!");
    });
    QObject::connect(&mainWind, &MainWindow::SigJoinServer, [&](const QString &text) {
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
    QObject::connect(&server, &Server::newCommandReceived, [&](const QString &cmd) {
        handler(cmd);
        updateState();
        server.sendToClients(QString::fromStdString(scene.to_string()));
    });
    QObject::connect(&client, &Client::newStateReceived, [&](const QString &state) {
        // TODO scene.loadFromString(state.toStdString());
        updateState();
    });

    // Chat
    QObject::connect(&client, &Client::newChatMessageReceived, [&](const QString &msg, const QString &name) {
        mainWind.setMessage(name.toStdString(), msg.toStdString());
        updateState();
    });
    QObject::connect(&server, &Server::newMessageReceived, [&](const QString &msg, const QString &name) {
        mainWind.setMessage(name.toStdString(), msg.toStdString());
        updateState();
    });
    QObject::connect(&server, &Server::newConnection, [this]() {
        server.sendToClients(QString::fromStdString(scene.to_string()));
    });
    QObject::connect(&mainWind, &MainWindow::EnterMessage, [this](const QString &text) {
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

void Application::setupRequirementsConnections(){
    // Требования
    QObject::connect(&mainWind, &MainWindow::oneRequirements, [this]() {
    });

    QObject::connect(&mainWind, &MainWindow::twoRequirements, [this]() {
        // Обработка сигнала twoReqirements
    });

    QObject::connect(&mainWind, &MainWindow::threeRequirements, [this]() {
        // Обработка сигнала threeReqirements
    });

    QObject::connect(&mainWind, &MainWindow::fourRequirements, [this]() {
        // Обработка сигнала fourReqirements
    });

    QObject::connect(&mainWind, &MainWindow::fiveRequirements, [this]() {
        // Обработка сигнала fiveReqirements
    });

    QObject::connect(&mainWind, &MainWindow::sixRequirements, [this]() {
        // Обработка сигнала sixReqirements
    });

    QObject::connect(&mainWind, &MainWindow::sevenRequirements, [this]() {
        // Обработка сигнала sevenReqirements
    });

    QObject::connect(&mainWind, &MainWindow::eightRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            ID obj1(pairID->first);
            ID obj2(pairID->second);
            RequirementData reqData;
            Requirement type;
            type = ET_SECTIONSECTIONPARALLEL;
            reqData.req = type;
            reqData.objects.push_back(obj1.get());
            reqData.objects.push_back(obj2.get());
            scene.addRequirement(reqData);
            ModeManager::setSave(false);
            // Изменение по айди
            // leftMenu->parameterChanged()

        }

    });

    QObject::connect(&mainWind, &MainWindow::nineRequirements, [this]() {

    });
    QObject::connect(&mainWind, &MainWindow::tenRequirements, [this]() {

    });
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

    // Изменение настроек
    QObject::connect(&mainWind, &MainWindow::changeSettings, [this]() {

    });

    // Console
    QObject::connect(&mainWind, &MainWindow::EnterPressed, [&](const QString &command) {
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
    QObject::connect(&mainWind, &MainWindow::projectSaved, [this](const QString &fileName, QString format) {
        if (format != (".ourp")) {
            painter->saveToImage(fileName, format);
            scene.paint();
        }
        else {
            std::string File = fileName.toStdString();
            scene.saveToFile(File.c_str());
            painter->draw();
        }
    });

    //Load
    QObject::connect(&mainWind, &MainWindow::LoadFile, [&](const QString &fileName) {
        painter->clear();
        scene.paint();
        std::string File = fileName.toStdString();
        scene.loadFromFile(File.c_str());
        painter->draw();
    });

    // Script
    QObject::connect(&mainWind, &MainWindow::EmitScript, [&](const QString &fileName) {
        std::string File = fileName.toStdString();
        scene.paint();
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

    });
/*
    // REDO
    QObject::connect(&mainWind, &MainWindow::REDO, [this]() {
        try {
            scene.redo();
            ModeManager::setSave(true);

            std::vector<std::pair<ID, ObjectData>> elements = scene.getAllElementsInfo();
            for (auto element: elements) {
                if (element.second.et == ET_POINT) {
                    double x = element.second.params[0];
                    double y = element.second.params[1];
                    leftMenu->addElemLeftMenu("Point", element.first.get(), {x, y});
                } else if (element.second.et == ET_CIRCLE) {
                    double x = element.second.params[0];
                    double y = element.second.params[1];
                    double r = element.second.params[2];
                    leftMenu->addElemLeftMenu("Circle", element.first.get(), {x, y, r});
                } else if (element.second.et == ET_SECTION) {
                    double x1 = element.second.params[0];
                    double y1 = element.second.params[1];
                    double x2 = element.second.params[2];
                    double y2 = element.second.params[3];
                    leftMenu->addElemLeftMenu("Section", element.first.get(), {x1, y1, x2, y2});
                }
            }
        } catch (std::exception &e) {
            mainWind.showWarning(e.what());
        }

    });

    //UNDO
    QObject::connect(&mainWind, &MainWindow::UNDO, [this]() {
        try {
            scene.undo();
            ModeManager::setSave(true);

            std::vector<std::pair<ID, ObjectData>> elements = scene.getAllElementsInfo();
            for (auto element: elements) {
                QString name;
                if (element.second.et == ET_POINT) {
                    double x = element.second.params[0];
                    double y = element.second.params[1];
                    leftMenu->addElemLeftMenu("Point", element.first.get(), {x, y});
                } else if (element.second.et == ET_CIRCLE) {
                    double x = element.second.params[0];
                    double y = element.second.params[1];
                    double r = element.second.params[2];
                    leftMenu->addElemLeftMenu("Circle", element.first.get(), {x, y, r});
                } else if (element.second.et == ET_SECTION) {
                    double x1 = element.second.params[0];
                    double y1 = element.second.params[1];
                    double x2 = element.second.params[2];
                    double y2 = element.second.params[3];
                    leftMenu->addElemLeftMenu("Section", element.first.get(), {x1, y1, x2, y2});
                }
            }
        } catch (std::exception &e) {
        }
    });
*/
}

void Application::updateState() {
    painter->draw();

    QFuture<void> future = QtConcurrent::run([this]() {
        for (const auto& call : this->vecCalls) {
            call();
        }
    });


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
            ID id = scene.addObject(point);
            ModeManager::setSave(false);
            vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Point", id.get(), {x, y});
            });
        }

    } else if (commandParts[0] == "circle" && commandParts.size() >= 4) {
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
            ID id = scene.addObject(circle);
            ModeManager::setSave(false);
           vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Circle", id.get(), {x, y, r});
                leftMenu->addElemLeftMenu("Point", id.get() - 1, {x, y});
            });
        }

    } else if (commandParts[0] == "section" && commandParts.size() >= 5) {
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
            ID id = scene.addObject(section);
            ModeManager::setSave(false);
           vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Point", id.get() - 1, {x, y});
                leftMenu->addElemLeftMenu("Point", id.get() - 2, {z, r});
                leftMenu->addElemLeftMenu("Section", id.get(), {x, y, z, r});
            });
        }

    } else if (commandParts[0] == "exit") {
        mainWind.close();
    } else if (commandParts[0] == "clear") {
        ModeManager::setSave(true);
        painter->clear();
        leftMenu->clearAllFigures();
        leftMenu->clearAllRequirements();
        scene.clear();
    } else if (commandParts[0] == "addreq" && commandParts.size() > 3) {
        int req = commandParts[1].toInt();
        ID obj1(commandParts[2].toInt());
        ID obj2(commandParts[3].toInt());
        RequirementData reqData;
        Requirement type;
        double parameters = 0;

        if (commandParts.size() >= 5) {
            parameters = commandParts[4].toDouble();
        }
        try {
            switch (req) {
                case 1:
                    type = ET_POINTSECTIONDIST;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    reqData.params.push_back(parameters);
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get(), parameters);
                    });
                    break;
                case 2:
                    type = ET_POINTONSECTION;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get());
                    });
                    break;
                case 3:
                    type = ET_POINTPOINTDIST;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    reqData.params.push_back(parameters);
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get(), parameters);
                    });
                    break;
                case 4:
                    type = ET_POINTONPOINT;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get());
                    });
                    break;
                case 5:
                    type = ET_SECTIONCIRCLEDIST;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    reqData.params.push_back(parameters);
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get(), parameters);
                    });
                    break;
                case 6:
                    type = ET_SECTIONONCIRCLE;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get());
                    });
                    break;
                case 7:
                    type = ET_SECTIONINCIRCLE;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get());
                    });
                    break;
                case 8:
                    type = ET_SECTIONSECTIONPARALLEL;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get());
                    });
                    break;
                case 9:
                    type = ET_SECTIONSECTIONPERPENDICULAR;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get());
                    });
                    break;
                case 10:
                    type = ET_SECTIONSECTIONANGLE;
                    reqData.req = type;
                    reqData.objects.push_back(obj1.get());
                    reqData.objects.push_back(obj2.get());
                    reqData.params.push_back(parameters);
                    scene.addRequirement(reqData);
                    ModeManager::setSave(false);
                   vecCalls.push_back([=, this]() {
                        leftMenu->addRequirementElem("PointSectionDist", req, obj1.get(),
                                                     obj2.get(), parameters);
                    });
                    break;
                default:
                    mainWind.showError("Not right number of req");
                    break;
            }
        } catch (std::exception &e) {
            mainWind.showError(e.what());
        }
    }

}