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

class Application : public QObject {

Q_OBJECT
private:
    QApplication app;
    MainWindow mainWind;
    Scene scene;
    QTPainter *painter;
    LeftMenuBar* leftMenu;
    QTimer* autoSaveTimer;

    std::vector<std::function<void()>> vecCalls;
    std::vector<QString> vec_requirements;

    QString username;
    Server server;
    Client client;

    UndoRedo::UndoRedoManager undoRedo;

    void initialize();
    void initLogger();
    void setupQTPainterConnections();
    void setupServerConnections();
    void setupRequirementsConnections();
    void setupLeftMenuConnections();
    void setupAddingCommandsConnections();
    void updateState();
    void handler(const QString &command);
    void addRequirement(Requirement RQ,ID id1,ID id2,double parameters);
    void addRequirement(Requirement RQ,ID id1,ID id2);
    void addPoints(double x,double y);
    void addSections(double x0,double y0,double x1,double y1);
    void addCircles(double x,double y,double r);
    void addArcs(double x0,double y0,double x1,double y1,double cx,double cy);

private:
    const QString pathTxtFileCommands = "../CommandsFile.txt";

private slots:
    void autoSave();

public:
    Application(int &argc, char **argv);
    int exec();
};

#endif // APPLICATION_H