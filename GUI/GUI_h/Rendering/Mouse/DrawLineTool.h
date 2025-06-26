#ifndef OURPAINT_DRAWLINETOOL_H
#define OURPAINT_DRAWLINETOOL_H

#include <QObject>
#include <QPainter>
#include <QPointF>

#include "RenderLines.h"
#include "Modes.h"
#include "DrawTool.h"


class DrawLineTool : public DrawTool {
Q_OBJECT
public:
    ~DrawLineTool() = default;
    DrawLineTool(QObject* parent = nullptr) : DrawTool(parent) {};

    void drawPreview(QPainter& painter, const QPointF& start, const QPointF& end) override;
    void draw(QPainter& painter, const QPointF& nowCursor) override;
    void pressButton(const QPointF& cursor) override;

    void drawRunningSection(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor);
    void drawHints(QPainter& painter, const QPointF& closestPoint, const QPointF& cursor);
    QPointF getSnappedPoint(const QPointF& current);
    void tabPressed();

signals:
    void SigSection(const QPointF& startPoint, const QPointF& endPoint);

private:
    QPointF closestStartPoint;
    QPointF closestPointNext;
    inline qreal snapAngle(qreal angle);


};

#endif //OURPAINT_DRAWLINETOOL_H
