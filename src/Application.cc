#include <QtConcurrent/QtConcurrent>
#include "Application.h"
#include "PainterController.h"
#include "MainWindController.h"
#include "LeftMenuController.h"
#include "Transaction.h"
#include "CommandFactory.h"
#include "UndoRedo.h"
#include "ConsoleManager.h"
#include "Mainwindow.h"
#include "Server.h"
#include "Client.h"
#include "GUI_Logger.h"

Application::Application(int& argc, char** argv)
        : scene(nullptr),
          undoRedo(nullptr),
          commandManager(nullptr),
          sqa(nullptr),
          app(nullptr),
          mainWind(nullptr),
          painter(nullptr),
          leftMenu(nullptr),
          username(nullptr),
          server(nullptr),
          client(nullptr),
          pc(nullptr),
          mwc(nullptr),
          lmc(nullptr)
          {
    try {
        initCore();
        initGUI(argc, argv);
        initNetwork();
        initLogger();
        initControllers();
    } catch (std::exception& e) {
        std::cout << e.what();
    }
}

void Application::initCore() {
    scene = new Scene(nullptr);
    undoRedo = new UndoRedo::UndoRedoManager(100);
    commandManager = new CommandManager();

    commandManager->registerFactory(new PointFactory(*scene));
    commandManager->registerFactory(new LineFactory(*scene));
    commandManager->registerFactory(new CircleFactory(*scene));
    commandManager->registerFactory(new ArcFactory(*scene));
    commandManager->registerFactory(new ReqFactory(*scene));
    commandManager->registerFactory(new DelFactory(*scene));
    commandManager->registerFactory(new ClearFactory(scene));
}

void Application::initGUI(int& argc, char** argv) {
    app = new QApplication(argc, argv);
    mainWind = new MainWindow();
    sqa = new SceneQtAdapter(*scene);
    username = new QString(mainWind->getUserName());

    mainWind->setupConsoleCommands({
        "POINT ",
        "LINE ",
        "CIRCLE ",
        "ARC ",
        "DELETE ",
        "DELETE OBJ ",
        "DELETE REQ ",
        "CLEAR",
        "REQ ",
        "UNDO",
        "REDO"
    });

    QApplication::setStyle("Fusion");
    app->setWindowIcon(QIcon(R"(..\Static\logo\logo2.ico)"));

    if (mainWind->getLeftMenuBar() == nullptr) {
        mainWind->showWarning("Can't opened LeftMenu");
    }

    if (mainWind->getQTPainter() == nullptr) {
        mainWind->showWarning("Can't opened QTPainter");
    }

    painter =  mainWind->getQTPainter();
    scene->setPainter(painter);
    leftMenu = mainWind->getLeftMenuBar();

    QObject::connect(sqa, &SceneQtAdapter::pointAddedQt, leftMenu, &LeftMenuBar::onPointAdded);
    QObject::connect(sqa, &SceneQtAdapter::sectionAddedQt, leftMenu, &LeftMenuBar::onSectionAdded);
    QObject::connect(sqa, &SceneQtAdapter::circleAddedQt, leftMenu, &LeftMenuBar::onCircleAdded);
    QObject::connect(sqa, &SceneQtAdapter::arcAddedQt, leftMenu, &LeftMenuBar::onArcAdded);
    QObject::connect(sqa, &SceneQtAdapter::reqAddedQt, leftMenu, &LeftMenuBar::onReqAdded);

    mainWind->show();
}

void Application::initNetwork() {
    server = new Server(*username);
    client = new Client(*username);
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
        mainWind->showWarning("Can't open or create log file!");
    }
}

void Application::initControllers() {
    pc = new PainterController(*scene, *commandManager, *undoRedo, *mainWind, *leftMenu);

    QObject::connect(painter->getMouseManager(), &MouseDrawingManager::SigPoint, pc, &PainterController::onSigPoint);
    QObject::connect(painter->getMouseManager(), &MouseDrawingManager::SigSection, pc, &PainterController::onSigSection);
    QObject::connect(painter->getMouseManager(), &MouseDrawingManager::SigCircle, pc, &PainterController::onSigCircle);
    QObject::connect(painter->getMouseManager(), &MouseDrawingManager::SigArc, pc, &PainterController::onSigArc);
    QObject::connect(painter, &QTPainter::MovingPoint, pc, &PainterController::onMovingPoint);
    QObject::connect(painter, &QTPainter::MovingSection, pc, &PainterController::onMovingSection);
    QObject::connect(painter, &QTPainter::MovingCircle, pc, &PainterController::onMovingCircle);
    QObject::connect(painter, &QTPainter::MovingArc, pc, &PainterController::onMovingArc);
    QObject::connect(painter, &QTPainter::EndMoving, pc, &PainterController::onEndMoving);

    mwc = new MainWindController(*painter, *scene, *mainWind, *leftMenu, *undoRedo, *commandManager, *server, *client,
                                 *username);

    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::DELETE, mwc, &MainWindController::onDelete); // Deleting an element
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::COPY, mwc, &MainWindController::onCopy); // ctrl+c
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::PASTE, mwc, &MainWindController::onPaste); // ctrl+v
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::CUT, mwc, &MainWindController::onCut); // ctrl+x
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::UNDO, mwc, &MainWindController::onUNDO); // UNDO
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::REDO, mwc, &MainWindController::onREDO); // REDO
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::firstReq, mwc, &MainWindController::onOneRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::secondReq, mwc, &MainWindController::onTwoRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::thirdReq, mwc, &MainWindController::onThreeRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::fourthReq, mwc, &MainWindController::onFourRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::fifthReq, mwc, &MainWindController::onFiveRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::sixthReq, mwc, &MainWindController::onSixRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::seventhReq, mwc, &MainWindController::onSevenRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::eighthReq, mwc, &MainWindController::onEightRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::ninthReq, mwc, &MainWindController::onNineRequirements);
    QObject::connect(painter->getKeyWW(), &KeyWorkWindow::tenthReq, mwc, &MainWindController::onTenRequirements);

    QObject::connect(mainWind->getFirstBut(), &QPushButton::clicked, mwc, &MainWindController::onOneRequirements);
    QObject::connect(mainWind->getSecondBut(), &QPushButton::clicked, mwc, &MainWindController::onTwoRequirements);
    QObject::connect(mainWind->getThirdBut(), &QPushButton::clicked, mwc, &MainWindController::onThreeRequirements);
    QObject::connect(mainWind->getFourthBut(), &QPushButton::clicked, mwc, &MainWindController::onFourRequirements);
    QObject::connect(mainWind->getFifthBut(), &QPushButton::clicked, mwc, &MainWindController::onFiveRequirements);
    QObject::connect(mainWind->getSixthBut(), &QPushButton::clicked, mwc, &MainWindController::onSixRequirements);
    QObject::connect(mainWind->getSeventhBut(), &QPushButton::clicked, mwc, &MainWindController::onSevenRequirements);
    QObject::connect(mainWind->getEighthBut(),&QPushButton::clicked, mwc, &MainWindController::onEightRequirements);
    QObject::connect(mainWind->getNinthBut(), &QPushButton::clicked, mwc, &MainWindController::onNineRequirements);
    QObject::connect(mainWind->getTenthBut(),&QPushButton::clicked, mwc, &MainWindController::onTenRequirements);

    QObject::connect(mainWind, &MainWindow::EnterPressed, mwc, &MainWindController::onEnterPressed); // Console
    QObject::connect(mainWind, &MainWindow::ProjectSaved, mwc, &MainWindController::onProjectSaved); // Save
    QObject::connect(mainWind, &MainWindow::LoadFile, mwc, &MainWindController::onLoadFile); // Load
    QObject::connect(mainWind, &MainWindow::EmitScript, mwc, &MainWindController::onEmitScript); // Script
    QObject::connect(mainWind, &MainWindow::SigExitSession, mwc, &MainWindController::onSigExitSession);
    QObject::connect(mainWind, &MainWindow::SigOpenServer, mwc, &MainWindController::onSigOpenServer);
    QObject::connect(mainWind, &MainWindow::SigJoinServer, mwc, &MainWindController::onSigJoinServer);
    QObject::connect(mainWind, &MainWindow::EnterMessage, mwc, &MainWindController::onEnterMessage);
    QObject::connect(mainWind, &MainWindow::NameUsers, mwc, &MainWindController::onEnterMessage);

    lmc = new LeftMenuController(*mainWind, *scene, *painter);

    QObject::connect(leftMenu, &LeftMenuBar::figureParamsChanged, lmc,
                     &LeftMenuController::onFigureParamsChanged); // Changing the settings in the left menu
    QObject::connect(leftMenu, &LeftMenuBar::reqParamChanged, lmc, &LeftMenuController::onReqParamChanged);
    QObject::connect(leftMenu, &LeftMenuBar::doubleClickLeftMenu, lmc,
                     &LeftMenuController::onDoubleClickLeftMenu); // Double-tap the left menu
}

int Application::exec() {
    return app->exec();
}

Application::~Application() {
    /* free controller */
    delete pc;
    delete mwc;
    delete lmc;

    /* free network*/
    delete server;
    delete client;

    /* free core */
    delete scene;
    delete undoRedo;
    delete commandManager;

    /* free gui */
    delete app;
    delete mainWind;
    delete leftMenu;
}



