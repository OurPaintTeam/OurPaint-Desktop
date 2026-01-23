#ifndef OURPAINT_DRAWRECTANGLETOOL_H
#define OURPAINT_DRAWRECTANGLETOOL_H
#include <QObject>
#include <QPainter>
#include <QPointF>

#include "DrawTool.h"

class DrawFigures;

class DrawRectangleTool final : public DrawTool{
private:
    QRectF rect;
public:
    ~DrawRectangleTool() override = default;
    explicit DrawRectangleTool(QObject* parent = nullptr) : DrawTool(parent) {}
    void draw(QPainter& painter, const QPointF& nowCursor) override;
    void drawPreview(QPainter& painter, const QPointF& start, const QPointF& end) override;
    void clear() override;
    void pressButton( const QPointF& cursor) override;
    void releasingButton();
    QRectF getRect() const;
};

#endif //OURPAINT_DRAWRECTANGLETOOL_H
