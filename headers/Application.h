#ifndef APPLICATION_H
#define APPLICATION_H

#include <QTranslator>
#include <QPixmap>
#include <memory>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QIcon>
#include <QTimer>


#include "Mainwindow.h"
#include "QTPainter.h"
#include "Server.h"
#include "Client.h"
#include "TreeModel.h"
#include "GUI_Logger.h"
#include "Scene.h"
#include "Painter.h"

#include "UndoRedo.h"
#include "Transaction.h"
#include "Command.h"
#include "CommandAddObject.h"
#include "CommandAddPoint.h"
#include "CommandAddSection.h"
#include "CommandAddCircle.h"
#include "CommandAddArc.h"
#include "CommandDeleteObject.h"
#include "CommandDeletePoint.h"
#include "CommandDeleteSection.h"
#include "CommandDeleteCircle.h"
#include "CommandDeleteArc.h"
#include "CommandAddRequirement.h"
#include "CommandDeleteRequirement.h"
#include "CommandMove.h"

#include "ConsoleManager.h"
#include "CommandFactory.h"

class Application : public QObject {
Q_OBJECT
private:
    QApplication app;
    MainWindow mainWind;
    QTPainter* painter;

    Scene scene;

    SceneQtAdapter sqa;
    LeftMenuBar* leftMenu;

    std::vector<std::function<void()>> vecCalls;
    std::vector<QString> vec_requirements;

    QString username;
    Server server;
    Client client;

    UndoRedo::UndoRedoManager undoRedo;

    CommandManager commandManager;

    void initialize();
    void initLogger();
    void setupQTPainterConnections();
    void setupServerConnections();
    void setupRequirementsConnections();
    void setupLeftMenuConnections();
    void setupAddingCommandsConnections();
    void updateState();
    void handler(const QString &command);
    void fillSelectedIDBuffer();
    void deleteOwnPoints(QVector<ID>& vecPoints,const QVector<ID>& vecSections,const QVector<ID>& vecCircles,const QVector<ID>& vecArcs);
    void deleteObjects(QVector<ID>& vecPoints,QVector<ID>& vecSections,QVector<ID>& vecCircles,QVector<ID>& vecArcs);
    void setupConsoleSystem();

private:
    std::vector<ObjectData> objectsBuffer;
    const QString pathTxtFileCommands = "../CommandsFile.txt";

    bool isStartMoving;
    std::vector<ObjectData> pre_move_object_states;

public:
    Application(int &argc, char **argv);
    int exec();
};

#endif // APPLICATION_H

