#ifndef OURPAINT_HEADERS_CONTROLLERS_MAIN_WINDOW_CONTROLLER_H_
#define OURPAINT_HEADERS_CONTROLLERS_MAIN_WINDOW_CONTROLLER_H_

#include <QObject>
#include "Objects.h"

class QTPainter;
class ID;
class Scene;
class MainWindow;
class LeftMenuBar;
namespace UndoRedo { class UndoRedoManager; }
class CommandManager;
class Server;
class Client;

class MainWindController : public QObject {
Q_OBJECT
private:
    QTPainter& _painter;
    Scene& _scene;
    MainWindow& _mainWind;
    LeftMenuBar& _lmb;
    UndoRedo::UndoRedoManager& _urm;
    CommandManager& _cm;
    Server& _s;
    Client& _c;
    std::vector<std::function<void()>> vecCalls;
    std::vector<QString> vec_requirements;
    std::vector<ObjectData> objectsBuffer;
    QString _username;
    const QString pathTxtFileCommands = "../CommandsFile.txt";

public:
    MainWindController(QTPainter& painter,
                       Scene& scene,
                       MainWindow& mainWind,
                       LeftMenuBar& lmb,
                       UndoRedo::UndoRedoManager& urm,
                       CommandManager& cm,
                       Server& s,
                       Client& c,
                       QString& username);

public slots:
    void onDelete();
    void onCopy();
    void onPaste();
    void onCut();
    void onResize();

    void onOneRequirements();
    void onTwoRequirements();
    void onThreeRequirements();
    void onFourRequirements();
    void onFiveRequirements();
    void onSixRequirements();
    void onSevenRequirements();
    void onEightRequirements();
    void onNineRequirements();
    void onTenRequirements();
    void onElevenRequirements();
    void onTwelveRequirements();

    void onEnterPressed(const QString& command);
    void onProjectSaved(const QString& fileName, QString format);
    void onLoadFile(const QString& fileName);
    void onEmitScript(const QString& fileName);
    void onUNDO();
    void onREDO();

    void onSigExitSession();
    void onSigOpenServer(const QString& text);
    void onSigJoinServer(const QString& text);

    void onEnterMessage(const QString& text);
    void onNameUsers(const QString& text);

private:
    void deleteOwnPoints(QVector<ID>& vecPoints,
                         const QVector<ID>& vecSections,
                         const QVector<ID>& vecCircles,
                         const QVector<ID>&);

    void deleteObjects(QVector<ID>& vecPoints,
                       QVector<ID>& vecSections,
                       QVector<ID>& vecCircles,
                       QVector<ID>&);

    void fillSelectedIDBuffer();
    void updateState();
};

#endif // ! OURPAINT_HEADERS_CONTROLLERS_MAIN_WINDOW_CONTROLLER_H_