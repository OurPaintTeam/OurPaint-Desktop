#ifndef OURPAINT_DRAWCIRCLETOOL_H
#define OURPAINT_DRAWCIRCLETOOL_H

#include <QObject>
#include <QPainter>
#include <QPointF>

#include "DrawFigures.h"
#include "DrawTool.h"

class DrawCircleTool : public DrawTool {
Q_OBJECT
public:
    ~DrawCircleTool() = default;
    DrawCircleTool(QObject* parent = nullptr) : DrawTool(parent) {};

    void draw(QPainter& painter, const QPointF& nowCursor) override;
    void drawPreview(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) override;

signals:
    void SigCircle(const QPointF& center, const qreal radius);
};

#endif //OURPAINT_DRAWCIRCLETOOL_H
