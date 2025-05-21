#ifndef OURPAINT_DRAWRECTANGLETOOL_H
#define OURPAINT_DRAWRECTANGLETOOL_H
#include <QObject>
#include <QPainter>
#include <QPointF>

#include "DrawFigures.h"
#include "DrawTool.h"

class DrawRectangleTool : public DrawTool{
private:
    QRectF rect;
public:
    ~DrawRectangleTool() = default;
    DrawRectangleTool(QObject* parent = nullptr) : DrawTool(parent) {}
    void draw(QPainter& painter, const QPointF& nowCursor) override;
    void drawPreview(QPainter& painter, const QPointF& start, const QPointF& end) override;
    void clear() override;
    void pressButton( const QPointF& cursor) override;
    void releasingButton();
    QRectF getRect();
};

#endif //OURPAINT_DRAWRECTANGLETOOL_H
