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

#include "DrawRectangleTool.h"
#include "KeyWorkWindow.h"
#include "MouseDrawingManager.h"
#include "MouseEventWorkWindow.h"
#include "RenderPoints.h"
#include "RenderCircles.h"
#include "RenderLines.h"
#include "RenderArcs.h"
#include "Objects.h"

class Scaling;
class DrawFigures;
class ClosestPoint;
class BoundBox;
class DrawBackground;
class GeometricObjects;
class ID;
class Colors;
class MouseDrawingManager;
class DrawRectangleTool;
class RenderStyle;
struct RenderPoints;
struct RenderLines;
struct RenderCircles;
struct RenderArcs;
class RenderDistance;
class MouseEventWorkWindow;
class KeyWorkWindow;

struct Container {
    // Work objects
    std::unique_ptr<MouseDrawingManager> mouseManager = std::make_unique<MouseDrawingManager>();
    std::unique_ptr<DrawRectangleTool> rectTool = std::make_unique<DrawRectangleTool>();

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
    bool leftClickFlag;
    bool drawing;

    Container() : pressLineVecBeg(0, 0),
      pressLineVecEnd(0, 0),
      pressPointCircle(0, 0),
      leftClickFlag(true),
      drawing(false)
    {
        lastClickTime.start();
    }
};

class QTPainter final : public QFrame {
Q_OBJECT

private:
    std::unique_ptr<MouseWorkWindow> mouseWW = std::make_unique<MouseWorkWindow>(this);          // For processing mouse events
    std::unique_ptr<KeyWorkWindow> keyWW =  std::make_unique<KeyWorkWindow>(this);               // For handling key events

    Container* activeContainer = nullptr;
    ObjectContainer* objectContainer = nullptr;
    std::unordered_map<QString, std::unique_ptr<Container>> namedContainers;

private:
    void inArea();

    bool leftClickTimer() const;
    bool findClosestObject() const;
    void drawingFigures(QPainter& painter) const;

    void managerMoving();
    void doubleClickEvent();
    void emitMoveFigures();
    void poseMovingFigures() const;

    void drawRectangle(QPainter& painter) const;
    void pointInRect(const QRectF& rect) const;
    void lineInRect(const QRectF& rect) const;
    void circleInRect(const QRectF& rect) const;
    void arcsInRect(const QRectF& rect) const;

    void initArcCase(std::unordered_map<ID, Arc*>& arcs);
    void initPointCase(std::unordered_map<ID, Point*>& points);
    void initCircleCase(std::unordered_map<ID, Circle*>& circles);
    void initLineCase(std::unordered_map<ID, Section*>& sections);
    void getBoundBox(const BoundBox2D& allObjects);

public:
    explicit QTPainter(QWidget* parent);

    void initObjectContainer(ObjectContainer& container);

    bool createNewContainer(const QString& name);
    bool setActiveContainer(const QString& name);
    bool deleteContainer(const QString& name);

    MouseDrawingManager* getMouseManager() const;
    KeyWorkWindow* getKeyWW() const;

    QVector<ID> getVecSelectedIDPoints() const;
    QVector<ID> getVecSelectedIDLines() const;
    QVector<ID> getVecSelectedIDCircles() const;
    QVector<ID> getVecSelectedIDArcs() const;
    std::optional<QPair<ID, ID>> getPairSelectedID();

    void selectedClear() const;
    void saveToImage(const QString& fileName, QString& format);
    void selectedElemByID(ID id, const std::string& type) const;

    static void drawGostFrame(QPainter* painter, const QSize& size);

protected:
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent* event) override;

    void clear();
    void draw();

    unsigned long long getWeight();
    unsigned long long getHeight();




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