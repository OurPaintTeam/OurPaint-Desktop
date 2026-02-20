#ifndef OURPAINT_HEADERS_CONTROLLERS_PAINTER_CONTROLLER_H_
#define OURPAINT_HEADERS_CONTROLLERS_PAINTER_CONTROLLER_H_

#include <QObject>

#include "LeftMenuBar.h"

class Scene;
class CommandManager;
class Transaction;
class ID;
class MainWindow;
class DocumentManager;

class PainterController : public QObject {
    Q_OBJECT
private:
    DocumentManager& _documentManager;
    MainWindow& _mainWind;
    bool _isStartMoving;
    std::vector<ObjectData> _pre_move_object_states;

public:
    PainterController(DocumentManager& documentManager, MainWindow& mainWind);

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