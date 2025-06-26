#ifndef OURPAINT_DRAWARCTOOL_H
#define OURPAINT_DRAWARCTOOL_H

#include <QObject>
#include <QPainter>
#include <QPointF>

#include "RenderArcs.h"
#include "DrawTool.h"

class DrawArcTool : public DrawTool {
Q_OBJECT
public:
    ~DrawArcTool() = default;
    DrawArcTool(QObject* parent = nullptr) : DrawTool(parent) {};

    void draw(QPainter& painter, const QPointF& nowCursor) override;
    void drawPreview(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) override;

signals:
    void SigArc(const QPointF& startPoint, const QPointF& endPoint, const QPointF& centerPoint);
};

#endif //OURPAINT_DRAWARCTOOL_H
