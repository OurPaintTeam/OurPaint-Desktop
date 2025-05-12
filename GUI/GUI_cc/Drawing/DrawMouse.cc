#include "DrawMouse.h"

DrawMouse::DrawMouse(QObject *parent)
        : QObject(parent), drawingInProgress(false), tabPressCount(0) {}

        // Серый цвет
QColor DrawMouse::hintColor() {
    return {169, 169, 169, 128};
}

// Сброс
void DrawMouse::resetCoordinates() {
    startCoordinates = QPoint(0, 0);
    closestStartPoint = QPoint(0, 0);
    closestPointNext = QPoint(0, 0);
}

// Нажатие таба
void DrawMouse::releaseTabIfPressed() {
    if (ModeManager::getActiveMode(KeyMode::Tab)) {
        ModeManager::setActiveMode(KeyMode::ReleasingTab);
    }
}

// Вычисление угла
double DrawMouse::snapAngle(double angle) {
    return std::round(angle / 45.0) * 45.0;
}

// Для отрисовки с шифтом
QPointF DrawMouse::getSnappedPoint(const QPointF &start, const QPointF &current) {
    double dx = current.x() - start.x();
    double dy = current.y() - start.y();
    double angle = std::atan2(dy, dx) * 180.0 / M_PI;
    double snappedAngle = snapAngle(angle);
    double length = std::hypot(dx, dy);

    double offsetX = 0;
    double offsetY = 0;

    const double diag = length / std::sqrt(2.0);

    if (snappedAngle == 0 || snappedAngle == 360 ) {
        offsetX = length; offsetY = 0;
    } else if (snappedAngle == 45) {
        offsetX = diag; offsetY = diag;
    } else if (snappedAngle == 90) {
        offsetX = 0; offsetY = length;
    } else if (snappedAngle == 135) {
        offsetX = -diag; offsetY = diag;
    } else if (snappedAngle == 180 || snappedAngle == -180) {
        offsetX = -length; offsetY = 0;
    } else if (snappedAngle == -135) {
        offsetX = -diag; offsetY = -diag;
    } else if (snappedAngle == -90) {
        offsetX = 0; offsetY = -length;
    } else if (snappedAngle == -45) {
        offsetX = diag; offsetY = -diag;
    }

    double snappedX = start.x() + offsetX;
    double snappedY = start.y() + offsetY;

    return {snappedX, snappedY};
}

// Предварительная серая линия
void DrawMouse::drawPreviewSection(QPainter &painter, const QPointF &start, const QPointF &end){
    DrawFigures::setPen(QPen(hintColor()));
    DrawFigures::drawSection(painter, start, end);
}

// Отрисовка мышью
void DrawMouse::DrawFiguresMouse(QPainter &painter) {

    painter.setPen(Qt::black);

    // Округление мыши до 1 знака после запятой
    double cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
    double cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;
    QPointF Cursor(cursorX, cursorY);

    bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);

    bool modePoint = ModeManager::getActiveMode(WorkModes::Point);
    bool modeSection = ModeManager::getActiveMode(WorkModes::Section);
    bool modeCircle = ModeManager::getActiveMode(WorkModes::Circle);
    bool modeArc = ModeManager::getActiveMode(WorkModes::Arc);


    bool shiftPressed = ModeManager::getActiveMode(KeyMode::Shift);

    if (modePoint && !leftClick) {
        DrawFigures::setPen(hintColor());
        DrawFigures::drawPoint(painter, Cursor);
    }

    if (leftClick) {
      if (modePoint) {
            // Если одно нажатие и точка => точка
            emit SigPoint(cursorX,cursorY);
        } else {
            if (!drawingInProgress) {
                ++tabPressCount;
                startCoordinates = Cursor;
                drawingInProgress = true;
            } else {
                double x0 = startCoordinates.x();
                double y0 = startCoordinates.y();

                if (modeSection) {

                    if (shiftPressed) {
                        // При нажатии фиксированы углы под 45
                        QPointF snapped = getSnappedPoint(startCoordinates, Cursor);
                        double x1 = snapped.x();
                        double y1 = snapped.y();

                        emit SigSection(x0, y0, x1, y1);
                    } else {
                        double x1 = cursorX;
                        double y1 = cursorY;
                        emit SigSection(x0, y0, x1, y1);
                    }
                } else if (modeCircle) {
                    double x1 = cursorX;
                    double y1 = cursorY;
                    double centerX = (x0 + x1) / 2.0;
                    double centerY = (y0 + y1) / 2.0;
                    double radius = std::hypot(x0 - centerX, y0 - centerY);
                    emit SigCircle(centerX, centerY, radius);
                }else if(modeArc){
                    double x1 = cursorX;
                    double y1 = cursorY;
                    double xc = (x1+x0)/2;
                    double yc = (y1+y0)/2;
                    emit SigArc(x1,y1,x0,y0,xc,yc);
                }
                resetCoordinates();
                tabPressCount = 0;
                drawingInProgress = false;


            }
        }
        ModeManager::setActiveMode(MouseMode::ReleasingLeft);
    }

    if (rightClick) {
        // Сброс всего правой клавишей
        resetCoordinates();
        tabPressCount = 0;
        drawingInProgress = false;
    }

    if (drawingInProgress) {
        if (modeCircle) {
            // Отрисовка круга
            double centerX = (startCoordinates.x() + cursorX) / 2;
            double centerY = (startCoordinates.y() + cursorY) / 2;
            double radius = std::hypot(startCoordinates.x() - centerX, startCoordinates.y() - centerY);

            DrawFigures::drawCircle(painter, QPointF(centerX, centerY), radius);
            DrawFigures::drawPoint(painter, QPointF(centerX, centerY));

            DrawFigures::setPen(hintColor());
            DrawFigures::drawSection(painter, QPointF(centerX, centerY), QPointF(cursorX, cursorY));

        } else if (modeSection) {
            drawSections(painter, startCoordinates);
        }else if(modeArc){
            QPointF center = (startCoordinates + Cursor) / 2.0;
            DrawFigures::drawArc(painter, Cursor,startCoordinates,center);
        }
    }
}

// Отрисовка линии
void DrawMouse::drawSections(QPainter &painter, const QPointF &startCoordinates)  {
    if (!ModeManager::getActiveMode(WorkModes::Section)) return;
    // Округление мыши до 1 знака после запятой
    double cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
    double cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;
    QPointF Cursor(cursorX, cursorY);
    bool shiftPressed = ModeManager::getActiveMode(KeyMode::Shift);

    if (shiftPressed) {
        QPointF snapped = getSnappedPoint(startCoordinates, Cursor);
        DrawFigures::drawSection(painter, startCoordinates, snapped);
    } else {
        DrawFigures::drawSection(painter, startCoordinates, Cursor);
    }
}

// Отрисовка подсказок
void DrawMouse::drawHints(QPainter &painter, const QPointF &closesPoint) {

    bool tabPressed = ModeManager::getActiveMode(KeyMode::Tab);
    bool modeSection = ModeManager::getActiveMode(WorkModes::Section);

    if (modeSection) {
        if (tabPressed) {
            ++tabPressCount;
            if (tabPressCount == 1) {
                startCoordinates = closestStartPoint;
            }
        }

        if (tabPressCount == 0) {
            closestStartPoint = closesPoint;
            // Округление мыши до 1 знака после запятой
            double cursorX = std::round(Scaling::logicCursorX() * 10.0) / 10.0;
            double cursorY = std::round(Scaling::logicCursorY() * 10.0) / 10.0;
            QPointF Cursor = QPointF(cursorX, cursorY);
            drawPreviewSection(painter, closestStartPoint, Cursor);
            releaseTabIfPressed();
        } else if (tabPressCount == 1) {
            drawingInProgress = true;
            closestPointNext = closesPoint;
            drawPreviewSection(painter, closestStartPoint, closestPointNext);
            releaseTabIfPressed();
        } else if (tabPressCount == 2) {
            releaseTabIfPressed();
            tabPressCount = 0;
            drawingInProgress = false;

            double x0 = (closestStartPoint.x());
            double y0 = (closestStartPoint.y());
            double x1 = (closestPointNext.x());
            double y1 = (closestPointNext.y());
            emit SigSection(x0, y0, x1, y1);

            resetCoordinates();
        }
    } else {
        tabPressCount = 0;
        resetCoordinates();
    }
}

void DrawMouse::clear() {
     closestStartPoint=QPointF();
     closestPointNext=QPointF();
     startCoordinates=QPointF();
     drawingInProgress=false;
     tabPressCount=0;
}