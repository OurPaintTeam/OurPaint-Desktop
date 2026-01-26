#include <QtConcurrent/QtConcurrent>
#include "Application.h"
#include "PainterController.h"
#include "MainWindController.h"
#include "LeftMenuController.h"
#include "Transaction.h"
#include "CommandFactory.h"
#include "ConsoleManager.h"
#include "Mainwindow.h"
#include "GUI_Logger.h"
#include "DocumentManager.h"

Application::Application(int& argc, char** argv)
        : documentManager(nullptr),
          sqa(nullptr),
          app(nullptr),
          mainWind(nullptr),
          painter(nullptr),
          leftMenu(nullptr),
          pc(nullptr),
          mwc(nullptr),
          lmc(nullptr)
          {
    try {
        initCore();
        initGUI(argc, argv);
        initLogger();
        initControllers();
    } catch (std::exception& e) {
        std::cout << e.what();
    }
}

void Application::initCore() {
    documentManager = new DocumentManager();
}

void Application::initGUI(int& argc, char** argv) {
    app = new QApplication(argc, argv);
    mainWind = new MainWindow();
    //sqa = new SceneQtAdapter(*scene);
    //username = new QString(mainWind->getUserName()); // TODO оно не тут

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
    //scene->setPainter(painter);
    leftMenu = mainWind->getLeftMenuBar();

    QObject::connect(sqa, &SceneQtAdapter::pointAddedQt, leftMenu, &LeftMenuBar::onPointAdded);
    QObject::connect(sqa, &SceneQtAdapter::sectionAddedQt, leftMenu, &LeftMenuBar::onSectionAdded);
    QObject::connect(sqa, &SceneQtAdapter::circleAddedQt, leftMenu, &LeftMenuBar::onCircleAdded);
    QObject::connect(sqa, &SceneQtAdapter::arcAddedQt, leftMenu, &LeftMenuBar::onArcAdded);
    QObject::connect(sqa, &SceneQtAdapter::reqAddedQt, leftMenu, &LeftMenuBar::onReqAdded);

    mainWind->show();
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
    pc = new PainterController(*documentManager, *mainWind);

    QObject::connect(painter->getMouseManager(), &MouseDrawingManager::SigPoint, pc, &PainterController::onSigPoint);
    QObject::connect(painter->getMouseManager(), &MouseDrawingManager::SigSection, pc, &PainterController::onSigSection);
    QObject::connect(painter->getMouseManager(), &MouseDrawingManager::SigCircle, pc, &PainterController::onSigCircle);
    QObject::connect(painter->getMouseManager(), &MouseDrawingManager::SigArc, pc, &PainterController::onSigArc);
    QObject::connect(painter, &QTPainter::MovingPoint, pc, &PainterController::onMovingPoint);
    QObject::connect(painter, &QTPainter::MovingSection, pc, &PainterController::onMovingSection);
    QObject::connect(painter, &QTPainter::MovingCircle, pc, &PainterController::onMovingCircle);
    QObject::connect(painter, &QTPainter::MovingArc, pc, &PainterController::onMovingArc);
    QObject::connect(painter, &QTPainter::EndMoving, pc, &PainterController::onEndMoving);

    mwc = new MainWindController(*painter, *documentManager, *mainWind, *leftMenu);

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

    QObject::connect(mainWind, &MainWindow::EnterCommand, mwc, &MainWindController::onEnterCommand); // Console
    QObject::connect(mainWind, &MainWindow::EnterMessage, mwc, &MainWindController::onEnterMessage);
    QObject::connect(mainWind, &MainWindow::EmitScript, mwc, &MainWindController::onEmitScript); // Script

    QObject::connect(mainWind, &MainWindow::SaveProject, mwc, &MainWindController::onSaveProject); // Save project
    QObject::connect(mainWind, &MainWindow::OpenProject, mwc, &MainWindController::onOpenProject); // Open project
    QObject::connect(mainWind, &MainWindow::CreateNewTab, mwc, &MainWindController::onCreateTab); // Create tab
    QObject::connect(mainWind, &MainWindow::ChangeTabs, mwc, &MainWindController::onChangeTab); // Change tab



    lmc = new LeftMenuController(*mainWind, *documentManager, *painter);

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


    /* free core */
    delete documentManager;

    /* free gui */
    delete app;
    delete leftMenu;
}



