#ifndef OURPAINT_MOUSEDRAWINGMANAGER_H
#define OURPAINT_MOUSEDRAWINGMANAGER_H

#include <QObject>
#include <memory>
#include <unordered_map>

#include "DrawTool.h"
#include "DrawPointTool.h"
#include "DrawLineTool.h"
#include "DrawArcTool.h"
#include "DrawCircleTool.h"
#include "Modes.h"

class MouseDrawingManager : public QObject {
Q_OBJECT
private:
    std::unordered_map<WorkModes, std::unique_ptr<DrawTool>> tools;
    QPointF closestPoint{};
    DrawTool* currentTool();

    template<typename ToolType, typename Signal, typename Slot>
    void connectTool(WorkModes mode, Signal signal, Slot slot) {
        if (auto* tool = dynamic_cast<ToolType*>(tools[mode].get())) {
            connect(tool, signal, this, slot);
        }
    }

public:
    explicit MouseDrawingManager(QObject* parent = nullptr);
    void setClosestPoint(const QPointF& point);
    void clear();

    void managerMouseDrawing(QPainter& painter);

signals:

    void SigPoint(const QPointF& point);
    void SigCircle(const QPointF& center, qreal radius);
    void SigSection(const QPointF& startPoint, const QPointF& endPoint);
    void SigArc(const QPointF& startPoint, const QPointF& endPoint, const QPointF& centerPoint);


};

#endif // OURPAINT_MOUSEDRAWINGMANAGER_H
