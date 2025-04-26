#include "Application.h"

Application::Application(int &argc, char **argv)
        : app(argc, argv),
          mainWind(),
          painter(nullptr),
          scene(nullptr),
          username("User"),
          server(username),
          client(username) {


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
    freopen(R"(..\\loggs\\guiLog.txt)", "a", stderr);
    logFile.setFileName("guiLog.txt");
    logFile.open(QIODevice::Append | QIODevice::Text);
    qInstallMessageHandler(guiLogger);
}

void Application::initialize() {
    QApplication::setStyle("Fusion");
    app.setWindowIcon(QIcon(R"(..\Static\logo\logo2.ico)"));

    painter = mainWind.getQTPainter();
    Scene scene_copy(painter);
    leftMenu = mainWind.getLeftMenuBar();
    scene = scene_copy;
    scene.setPainter(painter);
    mainWind.show();
    mainWind.resize();

    vec_requirements={
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

void Application::setupQTPainterConnections(){
    if (painter) {
        // Двойное нажатие на обьект и открытие его в левом меню
        QObject::connect(painter, &QTPainter::DoubleClickOnObject, [](ID id) {

        });

        // Перемещение точки
        QObject::connect(painter, &QTPainter::MovingPoint, [this](std::vector<ID> vec_id) {

            double Cx = Scaling::logicCursorX();
            double Cy = Scaling::logicCursorY();

            try {
                for(int i=0;i<vec_id.size();++i) {
                    scene.setPoint(vec_id[i], Cx, Cy);
                    vecCalls.push_back([=, this]() {
                       // leftMenu->updateParametersById(vec_id[i].get(),{Cx,Cy});
                    });
                }
            } catch (const std::exception &a) {
                mainWind.showError("Zheny kosyk ");
            }
            // updateState();
            painter->draw();
        });

        // Перемещение отрезка
        QObject::connect(painter, &QTPainter::MovingSection, [this](std::vector<ID> vec_id) {

            double dx = Scaling::logic(Scaling::getCursorDeltaX());
            double dy = Scaling::logic(Scaling::getCursorDeltaY());

            try {
                for(int i=0;i<vec_id.size();++i) {
                    scene.moveSection(vec_id[i], dx, dy);
                    vecCalls.push_back([=, this]() {
                      //  leftMenu->updateParametersById(vec_id[i].get(),{});
                    });
                }
            } catch (const std::exception &a) {
                mainWind.showError("Zheny kosyk ");
            }
            // updateState();
            painter->draw();

        });

        // Перемещение круга
        QObject::connect(painter, &QTPainter::MovingCircle, [this](std::vector<ID> vec_id) {

            double dx = Scaling::logic(Scaling::getCursorDeltaX());
            double dy = Scaling::logic(Scaling::getCursorDeltaY());

            try {
                for(int i=0;i<vec_id.size();++i) {
                scene.moveCircle(ID(vec_id[i]), dx, dy);
                    vecCalls.push_back([=, this]() {
                      //  leftMenu->updateParametersById(vec_id[i].get(),{});
                    });
                }
            } catch (const std::exception &a) {
                mainWind.showError("Zheny kosyk ");
            }
            // updateState();
            painter->draw();
        });

        // Перемещение арки
        QObject::connect(painter, &QTPainter::MovingArc, [this](std::vector<ID> vec_id) {

            double dx = Scaling::logic(Scaling::getCursorDeltaX());
            double dy = Scaling::logic(Scaling::getCursorDeltaY());

            try {
                for(int i=0;i<vec_id.size();++i) {
                    scene.moveArc(ID(vec_id[i]), dx, dy);
                    vecCalls.push_back([=, this]() {
                     //   leftMenu->updateParametersById(vec_id[i].get(),{});
                    });
                }
            } catch (const std::exception &a) {
                mainWind.showError("Zheny kosyk ");
            }
           // updateState();
            painter->draw();
        });

        // Отрисовка точки
        QObject::connect(painter, &QTPainter::SigPoint, [this](double x, double y) {
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
                ID id = scene.addObject(point);
                ModeManager::setSave(false);
                painter->draw();
                leftMenu->addElemLeftMenu("Point",id.get(),{x,y});
                leftMenu->updateLeftMenu();
            }
        });

        // Отрисовка круга
        QObject::connect(painter, &QTPainter::SigCircle, [this](double x, double y, double radius) {
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
                ID id = scene.addObject(circle);
                leftMenu->addElemLeftMenu("Circle",id.get(),{x,y,radius});
                leftMenu->addElemLeftMenu("Point",id.get()-1,{x,y});
                painter->draw();
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
                                     ID id = scene.addObject(section);
                                     ModeManager::setSave(false);
                                     leftMenu->addElemLeftMenu("Section",id.get(),{startX,startY,endX,endY});
                                     leftMenu->addElemLeftMenu("Point",id.get()-1,{startX,startY});
                                     leftMenu->addElemLeftMenu("Point",id.get()-2,{endX,endY});
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
                                 ID id = scene.addObject(section);
                                 ModeManager::setSave(false);
                                 painter->draw();
                                 leftMenu->addElemLeftMenu("Point",id.get()-1,{startX,startY});
                                 leftMenu->addElemLeftMenu("Point",id.get()-2,{endX,endY});
                                 leftMenu->addElemLeftMenu("Section",id.get(),{startX,startY,endX,endY});
                                 leftMenu->updateLeftMenu();
                             }
                         });


        // Отрисовка арки
        QObject::connect(painter, &QTPainter::SigArc,
                         [this](double x0, double y0, double x1, double y1,double xc,double yc) {
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
                                     ID id = scene.addObject(arc);
                                     ModeManager::setSave(false);
                                     leftMenu->addElemLeftMenu("Arc",id.get(),{x0,y0,x1,y1,xc,yc});
                                     leftMenu->addElemLeftMenu("Point",id.get()-1,{x0,y0});
                                     leftMenu->addElemLeftMenu("Point",id.get()-2,{x1,y1});
                                     leftMenu->addElemLeftMenu("Point",id.get()-2,{xc,yc});
                                     leftMenu->updateLeftMenu();
                                     painter->draw();
                                     server.sendToClients(QString::fromStdString(scene.to_string()));
                                 } else {
                                     client.sendCommandToServer("arc " + QString::number(x0) + " " +
                                                                QString::number(y0) + " " +
                                                                QString::number(x1) + " " +
                                                                QString::number(y1)+ " " +
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
                                 ID id = scene.addObject(arc);
                                 ModeManager::setSave(false);
                                 painter->draw();
                                 leftMenu->addElemLeftMenu("Arc",id.get(),{x0,y0,x1,y1,xc,yc});
                                 leftMenu->addElemLeftMenu("Point",id.get()-1,{x0,y0});
                                 leftMenu->addElemLeftMenu("Point",id.get()-2,{x1,y1});
                                 leftMenu->addElemLeftMenu("Point",id.get()-2,{xc,yc});
                                 leftMenu->updateLeftMenu();
                             }
                         });


    }

    // Удаление элемента
    QObject::connect(&mainWind, &MainWindow::DELETE, [this]() {

        std::vector<ID> vecPoint = painter->getVecIDPoints();
        std::vector<ID> vecSection = painter->getVecIDSections();
        std::vector<ID> vecCircle = painter->getVecIDCircles();

        for (int i = 0; i < vecPoint.size(); ++i) {
            scene.deletePoint(vecPoint[i]);
            vecCalls.push_back([=, this]() {
                leftMenu->removeFigureById(vecPoint[i].get());
            });

        }
        for (int i = 0; i < vecSection.size(); ++i) {
            scene.deleteSection(vecSection[i]);
            vecCalls.push_back([=, this]() {
                leftMenu->removeFigureById(vecSection[i].get());
            });
        }
        for (int i = 0; i < vecCircle.size(); ++i) {
            scene.deleteCircle(vecCircle[i]);
            vecCalls.push_back([=, this]() {
                leftMenu->removeFigureById(vecCircle[i].get());
            });
        }
        painter->selectedClear();
        painter->draw();
        updateState();
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

    QObject::connect(&mainWind, &MainWindow::SigOpenServer, [&](const QString &text) {
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
        auto pairID = painter->getPairID();
        if (pairID) {
            InputWindow window("Enter parameters: ");
            if (window.exec() == QDialog::Accepted) {
                bool ok = false;
                double parameters = window.getText().toDouble(&ok);
                if (!ok) return;
                addRequirement(ET_POINTSECTIONDIST,1,pairID->first,pairID->second,parameters);
                updateState();
            }
        }
    });


    QObject::connect(&mainWind, &MainWindow::twoRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            RequirementData reqData;
            addRequirement(ET_POINTONSECTION, 2, pairID->first, pairID->second);
            updateState();
        }
    });

    QObject::connect(&mainWind, &MainWindow::threeRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            InputWindow window("Enter parameters: ");
            if (window.exec() == QDialog::Accepted) {
                RequirementData reqData;
                bool ok = false;
                double parameters = window.getText().toDouble(&ok);
                if (!ok) return;
                addRequirement(ET_POINTPOINTDIST,3,pairID->first,pairID->second,parameters);
                updateState();
            }
        }
    });

    QObject::connect(&mainWind, &MainWindow::fourRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            addRequirement(ET_POINTONPOINT,4,pairID->first,pairID->second);
            updateState();
        }
    });

    QObject::connect(&mainWind, &MainWindow::fiveRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            InputWindow window("Enter parameters: ");
            if (window.exec() == QDialog::Accepted) {
                RequirementData reqData;
                bool ok = false;
                double parameters = window.getText().toDouble(&ok);
                if (!ok) return;
                addRequirement(ET_SECTIONCIRCLEDIST,5,pairID->first,pairID->second,parameters);
                updateState();
            }
        }
    });

    QObject::connect(&mainWind, &MainWindow::sixRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            InputWindow window("Enter parameters: ");
            if (window.exec() == QDialog::Accepted) {
                bool ok = false;
                double parameters = window.getText().toDouble(&ok);
                if (!ok) return;
                addRequirement(ET_SECTIONONCIRCLE, 6,pairID->first,pairID->second, parameters);
                updateState();
            }
        }
    });

    QObject::connect(&mainWind, &MainWindow::sevenRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            addRequirement(ET_SECTIONINCIRCLE,7,pairID->first,pairID->second);
            updateState();
        }
    });

    QObject::connect(&mainWind, &MainWindow::eightRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            addRequirement(ET_SECTIONSECTIONPARALLEL,8,pairID->first,pairID->second);
            updateState();
        }

    });

    QObject::connect(&mainWind, &MainWindow::nineRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            addRequirement(ET_SECTIONSECTIONPERPENDICULAR,9,pairID->first,pairID->second);
            updateState();
        }
    });

    QObject::connect(&mainWind, &MainWindow::tenRequirements, [this]() {
        auto pairID = painter->getPairID();
        if (pairID) {
            InputWindow window("Enter parameters: ");
            if (window.exec() == QDialog::Accepted) {
                bool ok = false;
                double parameters = window.getText().toDouble(&ok);
                if (!ok) return;
                addRequirement(ET_SECTIONSECTIONANGLE,10,pairID->first,pairID->second,parameters);
                updateState();
            }
        }
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
            scene.paint();
            painter->saveToImage(fileName, format);
        }
        else {
            std::string File = fileName.toStdString();
            scene.saveToFile(File.c_str());
            painter->draw();
        }
        mainWind.showSuccess("The project is saved!");
    });

    //Load
    QObject::connect(&mainWind, &MainWindow::LoadFile, [&](const QString &fileName) {
        painter->clear();
        std::string File = fileName.toStdString();
        scene.loadFromFile(File.c_str());
        painter->draw();
        scene.paint();
        mainWind.showSuccess("The project is loaded!");
    });

    // Script
    QObject::connect(&mainWind, &MainWindow::EmitScript, [&](const QString &fileName) {
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
            ID id = scene.addObject(point);
            ModeManager::setSave(false);
            vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Point", id.get(), {x, y});
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
            ID id = scene.addObject(circle);
            ModeManager::setSave(false);
           vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Circle", id.get(), {x, y, r});
                leftMenu->addElemLeftMenu("Point", id.get() - 1, {x, y});
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
            ID id = scene.addObject(section);
            ModeManager::setSave(false);
            vecCalls.push_back([=, this]() {
                leftMenu->addElemLeftMenu("Point", id.get() - 1, {x, y});
                leftMenu->addElemLeftMenu("Point", id.get() - 2, {z, r});
                leftMenu->addElemLeftMenu("Section", id.get(), {x, y, z, r});
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
                ID id = scene.addObject(arc);
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
        });
        updateState();
    }
    else if (commandParts[0] == "addreq" && commandParts.size() > 3) {
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
                    addRequirement(ET_POINTSECTIONDIST,req,obj1,obj2,parameters);
                    break;
                case 2:
                    addRequirement(ET_POINTONSECTION,req,obj1,obj2);
                    break;
                case 3:
                    addRequirement(ET_POINTPOINTDIST,req,obj1,obj2,parameters);
                    break;
                case 4:
                    addRequirement(ET_POINTONPOINT,req,obj1,obj2);
                    break;
                case 5:
                    addRequirement(ET_SECTIONCIRCLEDIST,req,obj1,obj2,parameters);
                    break;
                case 6:
                    addRequirement(ET_SECTIONONCIRCLE,req,obj1,obj2,parameters);
                    break;
                case 7:
                    addRequirement(ET_SECTIONINCIRCLE,req,obj1,obj2);
                    break;
                case 8:
                    addRequirement(ET_SECTIONSECTIONPARALLEL,req,obj1,obj2);
                    break;
                case 9:
                    addRequirement(ET_SECTIONSECTIONPERPENDICULAR,req,obj1,obj2);
                    break;
                case 10:
                    addRequirement(ET_SECTIONSECTIONANGLE,req,obj1,obj2,parameters);
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

void Application::addRequirement(Requirement RQ,int id,ID id1,ID id2,double parameters){
    RequirementData reqData;
    Requirement type=RQ;
    QString name=vec_requirements[id-1];

    reqData.req = type;
    reqData.objects.push_back(id1.get());
    reqData.objects.push_back(id2.get());
    reqData.params.push_back(parameters);
    scene.addRequirement(reqData);
    ModeManager::setSave(false);

    vecCalls.push_back([=, this]() {
        leftMenu->addRequirementElem(name, id, id1.get(),
                                     id2.get(), parameters);
    });
}

void Application::addRequirement(Requirement RQ,int id,ID id1,ID id2){
    RequirementData reqData;
    Requirement type=RQ;
    QString name=vec_requirements[id-1];

    reqData.req = type;
    reqData.objects.push_back(id1.get());
    reqData.objects.push_back(id2.get());
    scene.addRequirement(reqData);
    ModeManager::setSave(false);
    vecCalls.push_back([=, this]() {
        leftMenu->addRequirementElem(name, id, id1.get(),
                                     id2.get());
    });
}