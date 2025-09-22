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
#include <QLineEdit>

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
#include "RenderDistance.h"
#include "MouseEventWorkWindow.h"
#include "KeyWorkWindow.h"

class QTPainter : public QFrame, public Painter {
Q_OBJECT
private:

    // Work objects
    std::unique_ptr<MouseDrawingManager> mouseManager = std::make_unique<MouseDrawingManager>();
    std::unique_ptr<DrawRectangleTool> rectTool = std::make_unique<DrawRectangleTool>();
    std::unique_ptr<MouseWorkWindow> mouseWW = std::make_unique<MouseWorkWindow>(this);          // For processing mouse events
    std::unique_ptr< KeyWorkWindow> keyWW =  std::make_unique<KeyWorkWindow>(this);               // For handling key events

    std::unordered_map<ID, render::pointShell> visiblePoints;
    std::unordered_map<ID, render::circleShell> visibleCircles;
    std::unordered_map<ID, render::lineShell> visibleLines;
    std::unordered_map<ID, render::arcShell> visibleArcs;

    std::unordered_map<ID, PointStyle> bufferPointStyle;
    std::unordered_map<ID, LineStyle> bufferLineStyle;
    std::unordered_map<ID, CircleStyle> bufferCircleStyle;
    std::unordered_map<ID, ArcStyle> bufferArcStyle;

    QVector<ID> selectedIDLengthLine;

    // To move a segment, we remember the point of pressing
    QPointF pressLineVecBeg;
    QPointF pressLineVecEnd;
    QPointF pressPointCircle;

    // To avoid having to process multiple clicks
    QElapsedTimer lastClickTime;
    bool leftClickFlag = true;
    bool drawing = false;

private:
    void createNormalPointStyle(PointStyle* style);
    void createNormalCircleStyle(CircleStyle* style);
    void createNormalLineStyle(LineStyle* style);
    void createNormalArcStyle(ArcStyle* style);
    void createSelectedPointStyle(PointStyle* style);
    void createSelectedLineStyle(LineStyle* style);
    void createSelectedCircleStyle(CircleStyle* style);
    void createSelectedArcStyle(ArcStyle* style);
    void inArea();

    bool leftClickTimer();
    bool findClosestObject();
    void drawingFigures(QPainter& painter);

    void managerMoving();
    void doubleClickEvent();
    void emitMoveFigures();
    void poseMovingFigures();

    void drawRectangle(QPainter& painter);
    void pointInRect(QRectF& rect);
    void lineInRect(QRectF& rect);
    void circleInRect(QRectF& rect);
    void arcsInRect(QRectF& rect);
public:
    QTPainter(QWidget* parent);

    MouseDrawingManager* getMouseManager();
    KeyWorkWindow* getKeyWW();

    QVector<ID> getVecSelectedIDPoints();
    QVector<ID> getVecSelectedIDLines();
    QVector<ID> getVecSelectedIDCircles();
    QVector<ID> getVecSelectedIDArcs();
    std::optional<QPair<ID, ID>> getPairSelectedID();

    void selectedClear();
    void saveToImage(const QString& fileName, QString& format);
    void selectedElemByID(ID id, const std::string& type);

    void drawGostFrame(QPainter* painter, const QSize& size);
protected:
    [[maybe_unused]] void resizeEvent(QResizeEvent*) override;
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
    // Mouse rendering
    void DoubleClickOnObject(ID id);

    void MovingPoint(const QVector<ID>& selectedIDPoints);
    void MovingSection(const QVector<ID>& selectedIDSections, const QPointF& begin, const QPointF& end);
    void MovingCircle(const QVector<ID>& selectedIDCircles, const QPointF& press);
    void MovingArc(const QVector<ID>& selectedIDArcs);
    void EndMoving();
};


#endif // QTPAINTER_H