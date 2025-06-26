#ifndef OURPAINT_DRAWPOINTTOOL_H
#define OURPAINT_DRAWPOINTTOOL_H

#include <QObject>
#include <QPainter>
#include <QPointF>

#include "RenderPoints.h"
#include "DrawTool.h"

class DrawPointTool : public DrawTool{
Q_OBJECT
public:
    ~DrawPointTool() = default;
    DrawPointTool(QObject* parent = nullptr) : DrawTool(parent) {}
    void draw(QPainter& painter, const QPointF& nowCursor) override;
    void drawPreview(QPainter& painter, const QPointF& point, const QPointF& /*unused*/) override;

signals:
    void SigPoint(const QPointF& point);
};

#endif //OURPAINT_DRAWPOINTTOOL_H
