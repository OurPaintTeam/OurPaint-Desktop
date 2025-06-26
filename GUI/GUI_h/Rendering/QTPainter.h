#ifndef QTPAINTER_H
#define QTPAINTER_H

#include <QPainter>
#include <QPdfWriter>
#include <QWidget>
#include <QPaintEvent>
#include <QFrame>
#include <vector>
#include <QFileDialog>
#include <QMessageBox>
#include <list>
#include <QSvgGenerator>
#include <unordered_map>
#include <algorithm>
#include <QPixmap>

#include "Painter.h"
#include "Scaling.h"
#include "DrawFigures.h"
#include "ClosestPoint.h"
#include "DrawBackground.h"
#include "GeometricObjects.h"
#include "ID.h"
#include "BoundBox.h"
#include "Colors.h"
#include "MouseDrawingManager.h"
#include "DrawRectangleTool.h"
#include "RenderStyle.h"
#include "RenderPoints.h"
#include "RenderLines.h"
#include "RenderCircles.h"
#include "RenderArcs.h"

class QTPainter : public QFrame, public Painter {
Q_OBJECT
private:


    // Work objects
    std::unique_ptr<MouseDrawingManager> mouseManager;
    std::unique_ptr<DrawRectangleTool> rectTool;

    std::unordered_map<ID, render::pointShell> visiblePoints;
    std::unordered_map<ID, render::circleShell> visibleCircles;
    std::unordered_map<ID, render::lineShell> visibleLines;
    std::unordered_map<ID, render::arcShell> visibleArcs;

    // Selected objects
    QVector<ID> selectedIDPoint;
    QVector<ID> selectedIDCircle;
    QVector<ID> selectedIDSection;
    QVector<ID> selectedIDArc;

    // To move a segment, we remember the point of pressing
    QPointF pressLineVecBeg;
    QPointF pressLineVecEnd;
    QPointF pressPointCircle;

    // To avoid having to process multiple clicks
    QElapsedTimer lastClickTime;
    bool leftClickFlag;
    bool drawing;


    inline PointStyle createNormalPointStyle() {
        PointStyle style;
        style.figure.object.color = Color::Black;
        style.figure.object.size = 1;
        style.pointRadius = 1;
        style.figure.glow.activity=false;
        style.figure.id.activity=false;
        return style;
    }

    inline LineStyle createNormalLineStyle() {
        LineStyle style;
        style.figure.object.color = Color::Black;
        style.figure.object.size = 1;
        return style;
    }

    inline ArcStyle createNormalArcStyle() {
        ArcStyle style;
        style.figure.object.color = Color::Black;
        style.figure.object.size = 1;
        return style;
    }

    inline CircleStyle createNormalCircleStyle() {
        CircleStyle style;
        style.figure.object.color = Color::Black;
        style.figure.object.size = 1;
        return style;
    }

    inline PointStyle createSelectedPointStyle() {
        PointStyle style;
        style.figure.object.color = Color::Black;
        style.figure.object.size = 1;
        style.pointRadius = 2;
        style.figure.glow.color = Color::Blue;
        style.figure.glow.size = 5;
        style.figure.glow.activity=true;
        style.figure.id.activity=true;
        return style;
    }

    inline LineStyle createSelectedLineStyle() {
        LineStyle style;
        style.figure.object.color = Color::Black;
        style.figure.object.size = 2;
        style.figure.glow.color = Color::Blue;
        style.figure.glow.size = 5;
        style.figure.glow.activity=true;
        style.figure.id.activity=true;
        return style;
    }


    inline CircleStyle createSelectedCircleStyle() {
        CircleStyle style;
        style.figure.object.color = Color::Black;
        style.figure.object.size = 2;
        style.figure.glow.color = Color::Blue;
        style.figure.glow.size = 5;
        style.figure.glow.activity=true;
        style.figure.id.activity=true;
        return style;
    }

    inline ArcStyle createSelectedArcStyle() {
        ArcStyle style;
        style.figure.object.color = Color::Black;
        style.figure.object.size = 2;
        style.figure.glow.color = Color::Blue;
        style.figure.glow.size = 5;
        style.figure.glow.activity=true;
        style.figure.id.activity=true;
        return style;
    }

public:
    QTPainter(QWidget* parent);

    void inArea() {
        QSize size = Scaling::getActualMonitorSize();
        QRectF visibleRect(QPointF(0, 0), size);

        visiblePoints.clear();
        visibleLines.clear();
        visibleCircles.clear();
        visibleArcs.clear();

        // Точки
        for (const auto& [id, point] : *casePoints) {
            QPointF screenPos(
                    Scaling::scaleCoordinate(point->x) + Scaling::getDelta().x() + Scaling::getCenteredCoordinates().width(),
                    Scaling::scaleCoordinate(point->y) + Scaling::getDelta().y() + Scaling::getCenteredCoordinates().height()
            );

            if (visibleRect.contains(screenPos)) {
                PointStyle style=createNormalPointStyle();
                visiblePoints.emplace(id, render::pointShell{point, style});
            }
        }

        // Отрезки
        for (const auto& [id, section] : *caseSections) {
            QPointF p1(
                    Scaling::scaleCoordinate(section->beg->x) + Scaling::getDelta().x() + Scaling::getCenteredCoordinates().width(),
                    Scaling::scaleCoordinate(section->beg->y) + Scaling::getDelta().y() + Scaling::getCenteredCoordinates().height()
            );
            QPointF p2(
                    Scaling::scaleCoordinate(section->end->x) + Scaling::getDelta().x() + Scaling::getCenteredCoordinates().width(),
                    Scaling::scaleCoordinate(section->end->y) + Scaling::getDelta().y() + Scaling::getCenteredCoordinates().height()
            );

            QRectF lineBounding = QRectF(p1, p2).normalized();
            if (visibleRect.contains(p1) || visibleRect.contains(p2) || visibleRect.intersects(lineBounding)) {
                LineStyle style=createNormalLineStyle();
                visibleLines.emplace(id, render::lineShell{section, style});
            }
        }

        // Окружности
        for (const auto& [id, circle] : *caseCircles) {
            QPointF center(
                    Scaling::scaleCoordinate(circle->center->x) + Scaling::getDelta().x() + Scaling::getCenteredCoordinates().width(),
                    Scaling::scaleCoordinate(circle->center->y) + Scaling::getDelta().y() + Scaling::getCenteredCoordinates().height()
            );
            qreal r = circle->r  * Scaling::getZoom();

            QRectF bounding(center - QPointF(r, r), QSizeF(2*r, 2*r));
            if (visibleRect.intersects(bounding)) {
                CircleStyle style=createNormalCircleStyle();
                visibleCircles.emplace(id, render::circleShell{circle, style});
            }
        }

        // Дуги (грубо по boundingRect)
        for (const auto& [id, arc] : *caseArcs) {
            QPointF center(
                    Scaling::scaleCoordinate(arc->center->x) + Scaling::getDelta().x() + Scaling::getCenteredCoordinates().width(),
                    Scaling::scaleCoordinate(arc->center->y) + Scaling::getDelta().y() + Scaling::getCenteredCoordinates().height()
            );
            qreal r = arc->r  * Scaling::getZoom();

            QRectF bounding(center - QPointF(r, r), QSizeF(2*r, 2*r));
            if (visibleRect.intersects(bounding)) {
                ArcStyle style=createNormalArcStyle();
                visibleArcs.emplace(id, render::arcShell{arc, style});
            }
        }
    }


    QVector<ID> getVecSelectedIDPoints();

    QVector<ID> getVecSelectedIDSections();

    QVector<ID> getVecSelectedIDCircles();

    QVector<ID> getVecSelectedIDArcs();

    std::optional<QPair<ID, ID>> getPairSelectedID() const;

    void selectedClear();

    bool leftClickTimer();

    bool findClosestObject();

    void drawingFigures(QPainter& painter);

    void saveToImage(const QString& fileName, QString& format);

    void selectedElemByID(ID id, const std::string& type);

    void managerMoving();

    void doubleClickEvent();

    void emitMoveFigures();

    void poseMovingFigures();

    void drawRectangle(QPainter& painter);

    void pointInRect(QRectF& rect);

    void sectionInRect(QRectF& rect);

    void circleInRect(QRectF& rect);

    void arcsInRect(QRectF& rect);

    /*******   Signals   ******/
    void onSigPoint(const QPointF& point);

    void onSigCircle(const QPointF& center, const qreal radius);

    void onSigSection(const QPointF& startPoint, const QPointF& endPoint);

    void onSigArc(const QPointF& startPoint, const QPointF& endPoint, const QPointF& center);

protected:

    [[maybe_unused]] void resizeEvent(QResizeEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

    void clear() override;

    void draw() override;

    unsigned long long getWeight() override;

    unsigned long long getHeight() override;

    void getBoundBox(const BoundBox2D& allObjects) override;

    void initArcCase(std::unordered_map<ID, Arc*>& arcs) override;

    void initPointCase(std::unordered_map<ID, Point*>& points) override;

    void initCircleCase(std::unordered_map<ID, Circle*>& circles) override;

    void initSectionCase(std::unordered_map<ID, Section*>& sections) override;

signals:

    void MovingPoint(const QVector<ID>& selectedIDPoints);

    void MovingSection(const QVector<ID>& selectedIDSections, const QPointF& begin, const QPointF& end);

    void MovingCircle(const QVector<ID>& selectedIDCircles, const QPointF& press);

    void MovingArc(const QVector<ID>& selectedIDArcs);

    void EndMoving();

    // Mouse rendering
    void SigPoint(const QPointF& point);

    void SigCircle(const QPointF& center, const qreal radius);

    void SigSection(const QPointF& startPoint, const QPointF& endPoint);

    void SigArc(const QPointF& startPoint, const QPointF& endPoint, const QPointF& centerPoint);

    void DoubleClickOnObject(ID id);

private slots:

    void onWorkWindowResized();
};

#endif // QTPAINTER_H