#ifndef OURPAINT_HEADERS_CONTROLLERS_MAIN_WINDOW_CONTROLLER_H_
#define OURPAINT_HEADERS_CONTROLLERS_MAIN_WINDOW_CONTROLLER_H_

#include <QObject>
#include <fstream>
#include "Objects.h"

class QTPainter;
class ID;
class Scene;
class MainWindow;
class LeftMenuBar;
namespace UndoRedo { class UndoRedoManager; }
class CommandManager;
class InputWindow;
class SaveLoadJson;

class MainWindController final : public QObject {
Q_OBJECT
private:
    QTPainter& _painter;
    Scene& _scene;
    MainWindow& _mainWind;
    LeftMenuBar& _lmb;
    UndoRedo::UndoRedoManager& _urm;
    CommandManager& _cm;
    QVector<std::function<void()>> vecCalls;
    QVector<QString> vec_requirements;
    QVector<ObjectData> objectsBuffer;
    QString pathTxtFileCommands;

public:
    MainWindController(QTPainter& painter,
                       Scene& scene,
                       MainWindow& mainWind,
                       LeftMenuBar& lmb,
                       UndoRedo::UndoRedoManager& urm,
                       CommandManager& cm);

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
    void onProjectSaved(const QString& fileName);
    void onLoadFile(const QString& fileName);
    void onEmitScript(const QString& fileName);
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