#include "MouseDrawingManager.h"


DrawTool* MouseDrawingManager::currentTool() {
    for (const auto& [mode, tool]: tools) {
        if (ModeManager::getActiveMode(mode)) {
            return tool.get();
        }
    }
    return nullptr;
}


MouseDrawingManager::MouseDrawingManager(QObject* parent) : QObject(parent) {
    tools[WorkModes::Point] = std::make_unique<DrawPointTool>();
    tools[WorkModes::Section] = std::make_unique<DrawLineTool>();
    tools[WorkModes::Circle] = std::make_unique<DrawCircleTool>();
    tools[WorkModes::Arc] = std::make_unique<DrawArcTool>();

    connectTool<DrawPointTool>(WorkModes::Point, &DrawPointTool::SigPoint, &MouseDrawingManager::SigPoint);
    connectTool<DrawLineTool>(WorkModes::Section, &DrawLineTool::SigSection, &MouseDrawingManager::SigSection);
    connectTool<DrawCircleTool>(WorkModes::Circle, &DrawCircleTool::SigCircle, &MouseDrawingManager::SigCircle);
    connectTool<DrawArcTool>(WorkModes::Arc, &DrawArcTool::SigArc, &MouseDrawingManager::SigArc);
}


void MouseDrawingManager::setClosestPoint(const QPointF& point) {
    closestPoint = point;
}


void MouseDrawingManager::clear() {
    for (auto& [_, tool]: tools) {
        tool->clear();
    }
}


void MouseDrawingManager::managerMouseDrawing(QPainter& painter) {
    const QPointF cursor = Scaling::logicCursor();
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);
    const bool move = ModeManager::getActiveMode(MouseMode::MouseMove);

    if (move && leftClick) {
        return;
    }

    if (auto* tool = currentTool()) {
        QPointF finalCursor = cursor;

        if (auto* lineTool = dynamic_cast<DrawLineTool*>(tool)) {
            if (ModeManager::getActiveMode(KeyMode::Shift)) {
                finalCursor = lineTool->getSnappedPoint(cursor);
            }
            if (ModeManager::getActiveMode(KeyMode::Tab)) {
                lineTool->tabPressed();
                ModeManager::setActiveMode(KeyMode::ReleasingTab);
            }
            if (!qFuzzyCompare(closestPoint.x() + 1, 1) || !qFuzzyCompare(closestPoint.y() + 1, 1)) {
                lineTool->drawHints(painter, closestPoint, finalCursor);
            }
        }

        if (leftClick) {
            ModeManager::setActiveMode(MouseMode::ReleasingLeft);
            tool->pressButton(finalCursor);
        }

        if (rightClick) {
            tool->clear();
        }

        tool->draw(painter, finalCursor);
    }
}

