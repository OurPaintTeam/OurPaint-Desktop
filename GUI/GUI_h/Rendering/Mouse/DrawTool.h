#ifndef OURPAINT_DRAWTOOL_H
#define OURPAINT_DRAWTOOL_H
#include <QObject>
#include <QPainter>
#include <QPointF>

#include "DrawFigures.h"

class DrawTool : public QObject {
    Q_OBJECT
public:
    virtual ~DrawTool() = default;
    explicit DrawTool(QObject* parent = nullptr);
    virtual void draw(QPainter& painter, const QPointF& cursor) = 0;
    virtual void drawPreview(QPainter& painter, const QPointF& point, const QPointF& end) = 0;

    virtual void pressButton(const QPointF& cursor);
    virtual void clear();

protected:
    QPointF startCoordinates; // Cursor click point

    enum class DrawState : quint8 {
        NotStarted = 0,
        Started = 1,
        Completed = 2
    };
    DrawState state = DrawState::NotStarted;

    QPointF roundCursor(const QPointF& point);
    QColor hintColor() ;
};
#endif //OURPAINT_DRAWTOOL_H
