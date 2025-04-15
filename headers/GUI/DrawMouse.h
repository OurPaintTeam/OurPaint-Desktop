#ifndef OURPAINT_DRAWMOUSE_H
#define OURPAINT_DRAWMOUSE_H

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <cmath>
#include <qDebug>
#include "scaling.h"
#include "drawAdditionalInf.h"
#include "drawFigures.h"

class drawMouse : public QObject {
Q_OBJECT
//  Класс для отрисовки мышью
// Добавлены подсказки по ближайшим точкам

private:

    QPointF closestStartPoint;
    QPointF closestPointNext;
    QPointF startCoordinates;
    bool drawingInProgress;
    int tabPressCount;

    static QColor hintColor() {
        return {169, 169, 169, 128};
    }

    void resetCoordinates() {
        startCoordinates = QPoint(0, 0);
        closestStartPoint = QPoint(0, 0);
        closestPointNext = QPoint(0, 0);
    }

    static void releaseTabIfPressed() {
        if (ModeManager::getActiveMode(KeyMode::Tab)) {
            ModeManager::setActiveMode(KeyMode::ReleasingTab);
        }
    }

    static QPointF getSnappedPoint(const QPointF &start, const QPointF &current) {
        double dx = current.x() - start.x();
        double dy = current.y() - start.y();
        double angle = std::atan2(dy, dx) * 180.0 / M_PI;
        double snappedAngle = snapAngle(angle);
        double radians = snappedAngle * M_PI / 180.0;
        double length = std::hypot(dx, dy);
        return {start.x() + length * std::cos(radians), start.y() + length * std::sin(radians)};
    }

    static void drawPreviewSection(QPainter &painter, const QPointF &start, const QPointF &end) {
        drawFigures::setPen(QPen(hintColor()));
        drawFigures::drawSection(painter, start, end, false);
    }

public:
    explicit drawMouse(QObject *parent = nullptr)
            : QObject(parent), drawingInProgress(false), tabPressCount(0) {}

    static double snapAngle(double angle) {
        return std::round(angle / 45.0) * 45.0;
    }

    void drawFiguresMouse(QPainter &painter) {

        painter.setPen(Qt::black);

        double cursorX = Scaling::logicCursorX();
        double cursorY = Scaling::logicCursorY();
        QPointF Cursor(cursorX, cursorY);

        bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
        bool rightClick = ModeManager::getActiveMode(MouseMode::RightClick);

        bool modePoint = ModeManager::getActiveMode(WorkModes::Point);
        bool modeSection = ModeManager::getActiveMode(WorkModes::Section);
        bool modeCircle = ModeManager::getActiveMode(WorkModes::Circle);

        bool shiftPressed = ModeManager::getActiveMode(KeyMode::Shift);

        if (modePoint && !leftClick) {
            drawFigures::setPen(hintColor());
            drawFigures::drawPoint(painter, Cursor, false);
        }

        if (leftClick) {
            if (modePoint) {
                emit SigPoint(cursorX,cursorY);  // так как у qt инвертированная ось у
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
                            QPointF snapped = getSnappedPoint(startCoordinates, Cursor);
                            double x1 = snapped.x();
                            double y1 = snapped.y();

                            emit SigSection(x0, y0, x1, y1);
                        } else {
                            double x1 = cursorX;
                            double y1 = cursorY;
                            emit SigSection(x0, y0, x1, y1);
                        }
                        tabPressCount = 0;
                    } else if (modeCircle) {
                        double x1 = cursorX;
                        double y1 = cursorY;
                        double centerX = (x0 + x1) / 2.0;
                        double centerY = (y0 + y1) / 2.0;
                        double radius = std::hypot(x0 - centerX, y0 - centerY);
                        emit SigCircle(centerX, centerY, radius);
                    }
                    resetCoordinates();
                    drawingInProgress = false;
                }
            }
            ModeManager::setActiveMode(MouseMode::ReleasingLeft);
        }

        if (rightClick) {
            resetCoordinates();
            tabPressCount = 0;
            drawingInProgress = false;
        }

        if (drawingInProgress) {
            if (modeCircle) {
                double centerX = (startCoordinates.x() + cursorX) / 2;
                double centerY = (startCoordinates.y() + cursorY) / 2;
                double radius = std::hypot(startCoordinates.x() - centerX, startCoordinates.y() - centerY);

                drawFigures::drawCircle(painter, QPointF(centerX, centerY), radius, false);
                drawFigures::drawPoint(painter, QPointF(centerX, centerY), false);

                drawFigures::setPen(hintColor());
                drawFigures::drawSection(painter, QPointF(centerX, centerY), QPointF(cursorX, cursorY), false);

            } else if (modeSection) {
                drawSections(painter, startCoordinates);
            }
        }
    }

    static void drawSections(QPainter &painter, const QPointF &startCoordinates) {
        if (!ModeManager::getActiveMode(WorkModes::Section)) return;

        double cursorX =Scaling::logicCursorX();
        double cursorY = Scaling::logicCursorY();
        QPointF Cursor(cursorX, cursorY);
        bool shiftPressed = ModeManager::getActiveMode(KeyMode::Shift);

        if (shiftPressed) {
            QPointF snapped = getSnappedPoint(startCoordinates, Cursor);
            drawFigures::drawSection(painter, startCoordinates, snapped.toPoint(), false);
        } else {
            drawFigures::drawSection(painter, startCoordinates, Cursor, false);
        }
    }

    void drawHints(QPainter &painter, const QPointF &closesPoint) {

        bool tabPressed = ModeManager::getActiveMode(KeyMode::Tab);
        bool modeSection = ModeManager::getActiveMode(WorkModes::Section);

        if (modeSection) {
            if (tabPressed) {
                ++tabPressCount;
                if (tabPressCount == 1 && (closestStartPoint.x() != 0 || closestStartPoint.y() != 0)) {
                    startCoordinates = closestStartPoint;
                }
            }

            if (tabPressCount == 0) {
                closestStartPoint = closesPoint;
                QPointF Cursor = QPointF(Scaling::logicCursorX(), Scaling::logicCursorY());
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

signals:

    void SigPoint(double x, double y);

    void SigCircle(double x, double y, double r);

    void SigSection(double x, double y, double x1, double y1);
};

#endif // OURPAINT_DRAWMOUSE_H