#ifndef OURPAINT_HEADERS_CONTROLLERS_PAINTER_CONTROLLER_H_
#define OURPAINT_HEADERS_CONTROLLERS_PAINTER_CONTROLLER_H_

#include <QObject>

class Scene;
class CommandManager;
class Transaction;
namespace UndoRedo { class UndoRedoManager; };
class ID;
class ObjectData;
class MainWindow;

class PainterController : public QObject {
    Q_OBJECT
private:
    Scene& _scene;
    CommandManager& _commandManager;
    UndoRedo::UndoRedoManager& _undoRedo;
    MainWindow& _mainWind;
    bool _isStartMoving;
    std::vector<ObjectData> _pre_move_object_states;

public:
    PainterController(Scene& scene, CommandManager& commandManager, UndoRedo::UndoRedoManager& undoRedo, MainWindow& mainWind);

public slots:
    void onSigPoint(const QPointF& point);
    void onSigSection(const QPointF& startPoint, const QPointF& endPoint);
    void onSigCircle(const QPointF& center, const double radius);
    void onSigArc(const QPointF& startPoint, const QPointF& endPoint, const QPointF& centerPoint);
    void onMovingPoint(const QVector<ID>& vec_id);
    void onMovingSection(const QVector<ID>& vec_id, const QPointF& p1, const QPointF& p2);
    void onMovingCircle(const QVector<ID>& vec_id, const QPointF& offset);
    void onMovingArc(const QVector<ID>& vec_id);
    void onEndMoving();
};

#endif // ! OURPAINT_HEADERS_CONTROLLERS_PAINTER_CONTROLLER_H_