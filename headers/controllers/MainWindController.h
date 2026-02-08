#ifndef OURPAINT_HEADERS_CONTROLLERS_MAIN_WINDOW_CONTROLLER_H_
#define OURPAINT_HEADERS_CONTROLLERS_MAIN_WINDOW_CONTROLLER_H_

#include <QObject>
#include <fstream>
#include "Objects.h"

class QTPainter;
class ID;
class MainWindow;
class LeftMenuBar;
class Server;
class Client;
class DocumentManager;
class InputWindow;
class SaveLoadJson;
class SceneQtAdapter;

class MainWindController final : public QObject {
Q_OBJECT
private:
    QTPainter& _painter;
    MainWindow& _mainWind;
    LeftMenuBar& _lmb;
    DocumentManager& _documentManager;
    SceneQtAdapter& _sceneQtAdapter;
    std::vector<std::function<void()>> vecCalls;
    std::vector<QString> vec_requirements;
    std::vector<ObjectData> objectsBuffer;
    QString pathTxtFileCommands = "../CommandsFile.txt";

public:
    MainWindController(QTPainter& painter,
                       DocumentManager& documentManager,
                       MainWindow& mainWind,
                       LeftMenuBar& lmb,
                       SceneQtAdapter& sceneQtAdapter);

public slots:
    void onDelete();
    void onCopy();
    void onPaste();
    void onCut();

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

    void onEnterCommand(const QString& command);
    void onEmitScript(const QString& fileName);

    // Project level
    void onSaveProject();
    void onOpenProject(const QString& workDir);
    void onCreateProject(const QString& workDir);

    // Document level
    void onCreateTab(const QString& tabName);
    void onChangeTab(const QString& tabName);
    void onRenameTab(const QString& oldName, const QString& newName);
    void onDeleteTab(const QString& tabName);
    void onOpenFile(const QString& fileName);
    void onCloseProject();

    void onUNDO();
    void onREDO();

    void onEnterMessage(const QString& text);

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