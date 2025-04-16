#include "Application.h"
#include <QIcon>
#include "LoadSettingsApplications.h"
#include "SaveSettingsApplications.h"


Application::Application(int &argc, char **argv)
        : app(argc, argv),
          w(),
          painter(nullptr),
          screen(nullptr),
          server("User"),
          client("User"),
          isConnected(false),
          isServer(false) {

    painter.reset(w.getQTPainter());
    Paint screen_copy(painter.get());
    screen = screen_copy;

    initialize();
    setupConnections();

}

int Application::exec() {
    return app.exec();
}

void Application::initialize() {
    app.setWindowIcon(QIcon(R"(..\Static\logo\logo2.ico)"));
    QApplication::setStyle("Fusion");

    w.show();
    w.resized();

    username = "User";
    const std::string settingsFile = "SettingsSet";

    std::ifstream inFile(settingsFile);
    if (!(inFile.good() && inFile.peek() == std::ifstream::traits_type::eof())) {
        LoadSettingsApplications LoadSet("Settings", &w);
        std::vector<bool> settings;
        QString NameUsers;
        LoadSet.LoadSettings(settings, NameUsers);
        w.loadSettings(settings, NameUsers);
        if (!NameUsers.isEmpty()) {
            username = NameUsers;
        }
    }
    inFile.close();
    Server server(username);
    Client client(username);


}

void Application::setupConnections() {

    // Двойное нажатие на обьект и открытие его в левом меню
    QObject::connect(painter.get(), &QTPainter::DoubleClickOnObject, [this](ID id) {
        unsigned long long obj = id.id;
        w.getLeftMenuBar()->FocusOnItemById(obj);
    });

    QObject::connect(painter.get(), &QTPainter::MovingFigures, [this]() {

        double Cx = Scaling::logicCursorX();
        double Cy = Scaling::logicCursorY();

        double dx = Scaling::logic(Scaling::getCursorDeltaX());
        double dy = Scaling::logic(Scaling::getCursorDeltaY());

        ID id = painter->getIdFigures();// <- id фигуры перемещения

        try {
            screen.parallelMove(id, Cx, Cy, dx, dy);
        } catch (const std::exception &a) {
            w.showError("Zheny kosyk ");
        }

        updateState();
    });

    QObject::connect(painter.get(),
                     static_cast<void (QTPainter::*)(Element, double, double)>(&QTPainter::getIdFigure),
                     [this](Element F, double x, double y) {
                         ElementData elem;
                         elem.et = F;
                         double X = x;
                         double Y = y;
                         elem.params = {X, Y};
                         ID id = screen.findElement(elem);
                         painter->setIdFigures(id.id);
                         //if (painter->getDoubleClick()) {
                         //  w.FocusOnItemById(1);
                         //подсветка левого меню по айди
                         // }
                     }
    );

    QObject::connect(painter.get(),
                     static_cast<void (QTPainter::*)(Element, double, double, double)>(&QTPainter::getIdFigure),
                     [this](Element F, double x, double y, double r) {
                         ElementData elem;
                         elem.et = F;
                         double X = x;
                         double Y = y;
                         double R = r;
                         elem.params = {X, Y, R};
                         ID id = screen.findElement(elem);
                         painter->setIdFigures(id.id);
                         // if (painter->getDoubleClick()) {
                         //  w.FocusOnItemById(1);
                         //подсветка левого меню по айди
                         //  }
                     }
    );


    QObject::connect(painter.get(),
                     static_cast<void (QTPainter::*)(Element, double, double, double, double)>(&QTPainter::getIdFigure),
                     [this](Element F, double x, double y, double x1, double y1) {
                         ElementData elem;
                         elem.et = F;
                         double X = x;
                         double Y = y;
                         double X1 = x1;
                         double Y1 = y1;
                         elem.params = {X, Y, X1, Y1};
                         ID id = screen.findElement(elem);
                         painter->setIdFigures(id.id);
                         // if (painter->getDoubleClick()) {
                         //  w.FocusOnItemById(1);
                         //подсветка левого меню по айди
                         //  }
                     }
    );

    QObject::connect(&w, &MainWindow::oneReqirements, [this]() {
        std::vector<ID> vec_id = painter->getVecID();
        for (const auto &id: vec_id) {
            //qDebug() << id.id;
        }
    });

    QObject::connect(&w, &MainWindow::twoReqirements, [this]() {
        // Обработка сигнала twoReqirements
    });

    QObject::connect(&w, &MainWindow::threeReqirements, [this]() {
        // Обработка сигнала threeReqirements
    });

    QObject::connect(&w, &MainWindow::fourReqirements, [this]() {
        // Обработка сигнала fourReqirements
    });

    QObject::connect(&w, &MainWindow::fiveReqirements, [this]() {
        // Обработка сигнала fiveReqirements
    });

    QObject::connect(&w, &MainWindow::sixReqirements, [this]() {
        // Обработка сигнала sixReqirements
    });

    QObject::connect(&w, &MainWindow::sevenReqirements, [this]() {
        // Обработка сигнала sevenReqirements
    });

    QObject::connect(&w, &MainWindow::eightReqirements, [this]() {
        std::vector<ID> vec_id = painter->getVecID();
        if (!vec_id.empty()) {
            ID obj1 = vec_id[0].id;
            ID obj2 = vec_id[1].id;
            RequirementData reqData;
            Requirement type;
            type = ET_SECTIONSECTIONPARALLEL;
            reqData.req = type;
            reqData.objects.push_back(obj1);
            reqData.objects.push_back(obj2);
            screen.addRequirement(reqData);
            w.setSave(false);
            updateState();
            // w.getLeftMenuBar()->printReq(8,"ParallelSections",vec_id[0].id,vec_id[1].id,0);
        }

    });

    QObject::connect(&w, &MainWindow::nineReqirements, [this]() {
        painter->saveToImage("pdf");
    });

    QObject::connect(painter.get(), &QTPainter::SigPoint, [this](double x, double y) {
        if (isConnected) {
            if (isServer) {
                ElementData point;
                point.et = ET_POINT;
                point.params.push_back(x);
                point.params.push_back(y);
                ID id = screen.addElement(point);
                w.setSave(false);
                updateState();
                server.sendToClients(QString::fromStdString(screen.to_string()));
            } else {
                client.sendCommandToServer("point " + QString::number(y) + " " +
                                           QString::number(x));
            }
        } else {
            ElementData point;
            point.et = ET_POINT;
            point.params.push_back(x);
            point.params.push_back(y);
            ID id = screen.addElement(point);
            w.setSave(false);
            updateState();
        }
    });

    QObject::connect(painter.get(), &QTPainter::SigCircle, [this](double x, double y, double radius) {
        ElementData circle;
        if (isConnected) {
            if (isServer) {
                circle.et = ET_CIRCLE;
                circle.params.push_back(x);
                circle.params.push_back(y);
                circle.params.push_back(radius);
                ID id = screen.addElement(circle);
                w.setSave(false);
                updateState();
                server.sendToClients(QString::fromStdString(screen.to_string()));
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
            ID id = screen.addElement(circle);
            w.setSave(false);
            updateState();
        }
    });
    QObject::connect(painter.get(), &QTPainter::SigSection,
                     [this](double startX, double startY, double endX, double endY) {

                         ElementData section;
                         if (isConnected) {
                             if (isServer) {
                                 section.et = ET_SECTION;
                                 section.params.push_back(startX);
                                 section.params.push_back(startY);
                                 section.params.push_back(endX);
                                 section.params.push_back(endY);
                                 ID id = screen.addElement(section);
                                 w.setSave(false);
                                 updateState();
                                 server.sendToClients(QString::fromStdString(screen.to_string()));
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
                             ID id = screen.addElement(section);
                             w.setSave(false);
                             updateState();
                         }
                     });


    QObject::connect(&client, &Client::serverShutdown, [this]() {
        w.showSuccess("Server shutdown!(");
        isConnected = false;
        isServer = false;
    });
    QObject::connect(&w, &MainWindow::NameUsers, [this](const QString &text) {
        username = text;
        if (!isConnected) {
            server.setName(username);
            client.setName(username);
        }
    });
    QObject::connect(&client, &Client::disconnectedFromServer, [this]() {
        isConnected = false;
        w.showSuccess("You disconnected from server");
    });
    QObject::connect(&w, &MainWindow::changeSettings, [this]() {
        SaveSettingsApplications saveSet("Settings", &w);
        saveSet.clear();
        auto [figures, requirements, settings, name] = w.saveSettings();
        saveSet.SaveSettings(settings, name);
    });

    // Server button
    QObject::connect(&w, &MainWindow::SigExitSession, [this]() {
        if (isConnected) {
            if (isServer) {
                server.stopServer();
                isServer = false;
                isConnected = false;
                w.showSuccess("Successfully stopped!");
            } else {
                client.disconnectFromServer();
                isConnected = false;
                w.showSuccess("Successfully disconnected!");
            }
        } else {
            w.showError("Firstly connect to server!");
        }
    });
    QObject::connect(&w, &MainWindow::SigOpenServer, [&](const QString &text) {
        if (isConnected || isServer) {
            w.showError("Firstly disconnect!");
            return;
        }
        bool ok = false;
        text.toUShort(&ok);
        if (!ok) {
            w.showError("Error! This is not valid port!");
            return;
        }
        server.startServer(text.toUShort(&ok));
        isServer = true;
        isConnected = true;
        w.showSuccess("Successfully connected to server!");
    });
    QObject::connect(&w, &MainWindow::SigJoinServer, [&](const QString &text) {
        if (isConnected || isServer) {
            w.showError("Firstly disconnect!");
            return;
        }
        bool ok = false;
        QStringList texts = text.split(':');
        texts[1].toUShort(&ok);
        if (!ok) {
            w.showError("Error! This is not valid port!");
            return;
        }
        client.connectToServer(texts[0], texts[1].toUShort(&ok));
        isConnected = true;
        w.showSuccess("Successfully connected to server!");
    });

    // Servers
    QObject::connect(&server, &Server::newCommandReceived, [&](const QString &cmd) {
        handler(cmd);
        server.sendToClients(QString::fromStdString(screen.to_string()));
    });
    QObject::connect(&client, &Client::newStateReceived, [&](const QString &state) {
        screen.loadFromString(state.toStdString());
        updateState();
    });

    // Chat
    QObject::connect(&client, &Client::newChatMessageReceived, [&](const QString &msg, const QString &name) {
        w.setMessage(name.toStdString(), msg.toStdString());
        updateState();
    });
    QObject::connect(&server, &Server::newMessageReceived, [&](const QString &msg, const QString &name) {
        w.setMessage(name.toStdString(), msg.toStdString());
        updateState();
    });
    QObject::connect(&server, &Server::newConnection, [this]() {
        server.sendToClients(QString::fromStdString(screen.to_string()));
    });
    QObject::connect(&w, &MainWindow::EnterMessage, [this](const QString &text) {
        if (isConnected) {
            if (isServer) {
                w.setMessage(username.toStdString(), text.toStdString());
                server.sendChatToClients(text, username);
            } else {
                if (!text.isEmpty()) {
                    client.sendChatMessage(text);
                }
            }
        } else {
            w.showError("Firstly connect to server!");
            return;
        }
    });

    // Console
    QObject::connect(&w, &MainWindow::EnterPressed, [&](const QString &command) {
        QStringList commandParts = command.split(' ');
        if (isConnected) {
            if (isServer) {
                handler(command);
                server.sendToClients(QString::fromStdString(screen.to_string()));
            } else {
                client.sendCommandToServer(command);
            }
        } else {
            handler(command);
        }
    });


    QObject::connect(w.getLeftMenuBar(), &LeftMenuBar::parameterChanged,
                     [this](unsigned long long Id, const std::vector<double> &parameters) {
                         if (w.getLeftMenuBar()->isFiguresExpanded()) {
                             ID id = 0;
                             id.id = Id;
                             screen.LeftMenuMove(id, parameters);
                             // Сеттинг состояния для отмены сохранения
                             w.setSave(false);

                             screen.paint();
                             w.Draw();
                         }
                     });

    QObject::connect(w.getLeftMenuBar(), &LeftMenuBar::DoubleClickLeftMenu,
                     [this](std::vector<double> &parameters, unsigned long long int ID) {
                         if (w.getLeftMenuBar()->isFiguresExpanded()) {

                             painter->selectedElemByID(parameters, ID);
                         }
                     });

    // Resize
    QObject::connect(&w, &MainWindow::resized, [this]() {
        screen.paint();
        w.Draw();
    });
    QObject::connect(&w, &MainWindow::KeyPlus, [this]() {
        screen.paint();
        w.Draw();

    });
    QObject::connect(&w, &MainWindow::KeyMinus, [this]() {
        screen.paint();
    });
    QObject::connect(&w, &MainWindow::KeyZero, [this]() {
        screen.paint();
    });

    // Undo/Redo
    QObject::connect(&w, &MainWindow::REDO, [this]() {
        try {
            screen.redo();
            updateState();
            w.setSave(true);

            auto [figures, requirements, settings, name] = w.saveSettings();
            w.getLeftMenuBar()->printObject(0, "Clear", {});

            std::vector<std::pair<ID, ElementData>> elements = screen.getAllElementsInfo();
            for (auto element: elements) {
                std::string name;
                for (int i = 0; i < figures.size(); ++i) {
                    if (element.first.id == figures[i][1].toLongLong()) {
                        name = figures[i][0].toStdString();
                        break;
                    }
                }
                if (element.second.et == ET_POINT) {
                    double x = element.second.params[0];
                    double y = element.second.params[1];
                    if (!name.empty()) {
                        w.getLeftMenuBar()->printObject(element.first.id, name, {x, y});
                    } else {
                        w.getLeftMenuBar()->printObject(element.first.id, "Point", {x, y});
                    }
                } else if (element.second.et == ET_CIRCLE) {
                    double x = element.second.params[0];
                    double y = element.second.params[1];
                    double r = element.second.params[2];
                    if (!name.empty()) {
                        w.getLeftMenuBar()->printObject(element.first.id, name, {x, y, r});
                    } else {
                        w.getLeftMenuBar()->printObject(element.first.id, "Circle", {x, y, r});
                    }
                } else if (element.second.et == ET_SECTION) {
                    double x1 = element.second.params[0];
                    double y1 = element.second.params[1];
                    double x2 = element.second.params[2];
                    double y2 = element.second.params[3];
                    if (!name.empty()) {
                        w.getLeftMenuBar()->printObject(element.first.id, name, {x1, y1, x2, y2});
                    } else {
                        w.getLeftMenuBar()->printObject(element.first.id, "Section", {x1, y1, x2, y2});
                    }
                }
            }
        } catch (std::exception &e) {
            w.showWarning(e.what());
        }

    });
    QObject::connect(&w, &MainWindow::UNDO, [this]() {
        try {
            screen.undo();
            updateState();
            w.setSave(true);

            auto [figures, requirements, settings, name] = w.saveSettings();
            w.getLeftMenuBar()->printObject(0, "Clear", {});
            std::vector<std::pair<ID, ElementData>> elements = screen.getAllElementsInfo();
            for (auto element: elements) {
                std::string name;
                for (int i = 0; i < figures.size(); ++i) {
                    if (element.first.id == figures[i][1].toLongLong()) {
                        name = figures[i][0].toStdString();
                        break;
                    }
                }
                if (element.second.et == ET_POINT) {
                    double x = element.second.params[0];
                    double y = element.second.params[1];
                    if (!name.empty()) {
                        w.getLeftMenuBar()->printObject(element.first.id, name, {x, y});
                    } else {
                        w.getLeftMenuBar()->printObject(element.first.id, "Point", {x, y});
                    }
                } else if (element.second.et == ET_CIRCLE) {
                    double x = element.second.params[0];
                    double y = element.second.params[1];
                    double r = element.second.params[2];
                    if (!name.empty()) {
                        w.getLeftMenuBar()->printObject(element.first.id, name, {x, y, r});
                    } else {
                        w.getLeftMenuBar()->printObject(element.first.id, "Circle", {x, y, r});
                    }
                } else if (element.second.et == ET_SECTION) {
                    double x1 = element.second.params[0];
                    double y1 = element.second.params[1];
                    double x2 = element.second.params[2];
                    double y2 = element.second.params[3];
                    if (!name.empty()) {
                        w.getLeftMenuBar()->printObject(element.first.id, name, {x1, y1, x2, y2});
                    } else {
                        w.getLeftMenuBar()->printObject(element.first.id, "Section", {x1, y1, x2, y2});
                    }
                }
            }
        } catch (std::exception &e) {
        }
    });

    // Save/Load
    QObject::connect(&w, &MainWindow::saveBMP, [this](const QString &fileName) {
        try {
            screen.exportToBMP(fileName.toStdString().c_str());
            w.showSuccess("Saved to " + fileName);
        } catch (std::exception &e) {
            w.showWarning(e.what());
        }
    });
    QObject::connect(&w, &MainWindow::loadBMP, [&](const QString &fileName) {

    });
    QObject::connect(&w, &MainWindow::projectSaved, [this](const QString &fileName) {
        std::string File = fileName.toStdString();
        auto [figures, requirements, settings, name] = w.saveSettings();
        SaveSettingsApplications saveSet(File.c_str(), &w);
        saveSet.clear();
        saveSet.SaveFigures(figures);
        saveSet.SaveRequirements(requirements);
        saveSet.SaveSettings(settings, name);
        screen.saveToFile(File.c_str());
        screen.paint();
        w.Draw();
    });
    QObject::connect(&w, &MainWindow::LoadFile, [&](const QString &fileName) {
        w.WorkWindowClear();
        w.getLeftMenuBar()->printObject(0, "Clear", {});
        std::string File = fileName.toStdString();
        screen.loadFromFile(File.c_str());
        screen.paint();
        w.Draw();

        LoadSettingsApplications LoadSet(File.c_str(), &w);
        std::vector<std::vector<QString>> figures;
        std::vector<std::vector<QString>> requirements;
        std::vector<bool> settings;
        QString NameUsers;
        LoadSet.LoadFigures(figures);
        LoadSet.LoadRequirements(requirements);
        LoadSet.LoadSettings(settings, NameUsers);

        std::vector<std::pair<ID, ElementData>> elements = screen.getAllElementsInfo();
        for (auto element: elements) {
            std::string name;
            for (int i = 0; i < figures.size(); ++i) {
                if (element.first.id == figures[i][1].toLongLong()) {
                    name = figures[i][0].toStdString();
                    break;
                }
            }
            if (element.second.et == ET_POINT) {
                double x = element.second.params[0];
                double y = element.second.params[1];
                if (!name.empty()) {
                    w.getLeftMenuBar()->printObject(element.first.id, name, {x, y});
                } else {
                    w.getLeftMenuBar()->printObject(element.first.id, "Point", {x, y});
                }
            } else if (element.second.et == ET_CIRCLE) {
                double x = element.second.params[0];
                double y = element.second.params[1];
                double r = element.second.params[2];
                if (!name.empty()) {
                    w.getLeftMenuBar()->printObject(element.first.id, name, {x, y, r});
                } else {
                    w.getLeftMenuBar()->printObject(element.first.id, "Circle", {x, y, r});
                }
                break;
            } else if (element.second.et == ET_SECTION) {
                double x1 = element.second.params[0];
                double y1 = element.second.params[1];
                double x2 = element.second.params[2];
                double y2 = element.second.params[3];
                if (!name.empty()) {
                    w.getLeftMenuBar()->printObject(element.first.id, name, {x1, y1, x2, y2});
                } else {
                    w.getLeftMenuBar()->printObject(element.first.id, "Section", {x1, y1, x2, y2});
                }
                break;
            }
        }


        w.loadSettings(settings, NameUsers);


    });
    QObject::connect(&w, &MainWindow::EmitScript, [&](const QString &fileName) {
        std::string File = fileName.toStdString();

        std::ifstream Script(File);

        // TODO logs

        // TODO UNDO/REDO

        std::string command;
        while (std::getline(Script, command)) {
            QString qCommand = QString::fromStdString(command);

            // TODO logs

            if (isConnected) {
                if (isServer) {
                    handler(qCommand);
                    server.sendToClients(QString::fromStdString(screen.to_string()));
                } else {
                    client.sendCommandToServer(qCommand);
                }
            } else {
                handler(qCommand);
            }
        }
        updateState();
    });

}

void Application::updateState() {

    //painter->getUsers(false);
    screen.paint();

    w.Draw();
/*
    auto [figures, requirements, settings, name] = w.saveSettings();
    w.getLeftMenuBar()->printObject(0, "Clear", {});
    w.getLeftMenuBar()->printReq(0, "Clear", 0, 0, 0);
    std::vector<std::pair<ID, ElementData>> elements = screen.getAllElementsInfo();

    for (auto element: elements) {
        std::string name;

        for (int i = 0; i < figures.size(); ++i) {
            if (element.first.id == figures[i][1].toLongLong()) {
                name = figures[i][0].toStdString();
                break;
            }
        }

        if (element.second.et == ET_POINT) {
            double x = element.second.params[0];
            double y = element.second.params[1];
            if (!name.empty()) {
                w.getLeftMenuBar()->printObject(element.first.id, name, {x, y});
            } else {
                w.getLeftMenuBar()->printObject(element.first.id, "Point", {x, y});
            }
        } else if (element.second.et == ET_SECTION) {
            double x1 = element.second.params[0];
            double y1 = element.second.params[1];
            double x2 = element.second.params[2];
            double y2 = element.second.params[3];
            if (!name.empty()) {
                w.getLeftMenuBar()->printObject(element.first.id, name, {x1, y1, x2, y2});
            } else {
                w.getLeftMenuBar()->printObject(element.first.id, "Section", {x1, y1, x2, y2});
            }
        } else if (element.second.et == ET_CIRCLE) {
            double x = element.second.params[0];
            double y = element.second.params[1];
            double r = element.second.params[2];
            if (!name.empty()) {
                w.getLeftMenuBar()->printObject(element.first.id, name, {x, y, r});
            } else {
                w.getLeftMenuBar()->printObject(element.first.id, "Circle", {x, y, r});
            }
        }

    }

    std::vector<std::pair<ID, RequirementData>> req = screen.getAllRequirementsInfo();

    w.getLeftMenuBar()->printReq(0, "Clear", 0, 0, 0);

    for (const auto &element: req) {
        QString text;
        double param = 0;
        if (element.second.params.size() == 1) {
            param = element.second.params[0];
        }
        switch (element.second.req) {
            case ET_POINTSECTIONDIST:
                text = QString("PointSectionDist");
                break;
            case ET_POINTONSECTION:
                text = QString("PointOnSection");
                break;
            case ET_POINTPOINTDIST:
                text = QString("PointPointDist");
                break;
            case ET_POINTONPOINT:
                text = QString("PointOnPoint");
                break;
            case ET_SECTIONCIRCLEDIST:
                text = QString("SectionCircleDist");
                break;
            case ET_SECTIONONCIRCLE:
                text = QString("SectionOnCircle");
                break;
            case ET_SECTIONINCIRCLE:
                text = QString("SectionInCircle");
                break;
            case ET_SECTIONSECTIONPARALLEL:
                text = QString("ParallelSections");
                break;
            case ET_SECTIONSECTIONPERPENDICULAR:
                text = QString("PerpendicularSections");
                break;
            case ET_SECTIONSECTIONANGEL:
                text = QString("AngleBetweenSections");
                break;
            case ET_POINTONCIRCLE:
                text = QString("PointOnCircle");
                break;
        }
        w.getLeftMenuBar()->printReq(element.first.id + 1, text.toStdString(), element.second.objects[0].id,
                                     element.second.objects[1].id, param);
    }
*/
}

void Application::handler(const QString &command) {
    QStringList commandParts = command.split(' ');
    bool commandRight = false;

    if (commandParts[0] == "point" && commandParts.size() >= 3) {
        bool xOk, yOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);

        if (xOk && yOk) {
            ElementData point;
            point.et = ET_POINT;
            point.params.push_back(x);
            point.params.push_back(y);
            ID id = screen.addElement(point);
            w.setSave(false);
            commandRight = true;
        }

    } else if (commandParts[0] == "circle" && commandParts.size() >= 4) {
        bool xOk, yOk, rOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);
        double r = commandParts[3].toDouble(&rOk);

        if (xOk && yOk && rOk) {

            ElementData circle;
            circle.et = ET_CIRCLE;
            circle.params.push_back(x);
            circle.params.push_back(y);
            circle.params.push_back(r);
            ID id = screen.addElement(circle);
            w.setSave(false);
            commandRight = true;
        }

    } else if (commandParts[0] == "section" && commandParts.size() >= 5) {
        bool xOk, yOk, zOk, rOk;
        double x = commandParts[1].toDouble(&xOk);
        double y = commandParts[2].toDouble(&yOk);
        double z = commandParts[3].toDouble(&zOk);
        double r = commandParts[4].toDouble(&rOk);
        if (xOk && yOk && zOk && rOk) {
            ElementData section;
            section.et = ET_SECTION;
            section.params.push_back(x);
            section.params.push_back(y);
            section.params.push_back(z);
            section.params.push_back(r);
            ID id = screen.addElement(section);
            w.setSave(false);
            commandRight = true;
        }

    } else if (commandParts[0] == "exit") {
        commandRight = true;
        w.close();
    } else if (commandParts[0] == "clear") {
        commandRight = true;
        w.setSave(true);
        w.WorkWindowClear();
        w.getLeftMenuBar()->printObject(0, "Clear", {});
        w.getLeftMenuBar()->printReq(0, "Clear", 0, 0, 0);
        screen.clear();
    } else if (commandParts[0] == "addreq" && commandParts.size() > 3) {
        commandRight = true;
        int req = commandParts[1].toInt();
        ID obj1 = commandParts[2].toInt();
        ID obj2 = commandParts[3].toInt();
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
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    reqData.params.push_back(parameters);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 2:
                    type = ET_POINTONSECTION;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 3:
                    type = ET_POINTPOINTDIST;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    reqData.params.push_back(parameters);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 4:
                    type = ET_POINTONPOINT;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 5:
                    type = ET_SECTIONCIRCLEDIST;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    reqData.params.push_back(parameters);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 6:
                    type = ET_SECTIONONCIRCLE;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 7:
                    type = ET_SECTIONINCIRCLE;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 8:
                    type = ET_SECTIONSECTIONPARALLEL;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 9:
                    type = ET_SECTIONSECTIONPERPENDICULAR;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                case 10:
                    type = ET_SECTIONSECTIONANGEL;
                    reqData.req = type;
                    reqData.objects.push_back(obj1);
                    reqData.objects.push_back(obj2);
                    reqData.params.push_back(parameters);
                    screen.addRequirement(reqData);
                    w.setSave(false);
                    break;
                default:
                    commandRight = false;
                    w.showError("Not right number of req");
                    break;
            }
        } catch (std::exception &e) {
            commandRight = false;
            w.showError(e.what());
        }
    }
    if (commandRight) {
        updateState();
    }
}