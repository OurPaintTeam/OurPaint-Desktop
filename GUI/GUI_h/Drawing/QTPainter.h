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
#include "DrawAdditionalInf.h"
#include "DrawFigures.h"
#include "ClosestPoint.h"
#include "DrawBackground.h"
#include "DrawMouse.h"
#include "SelectedRectangle.h"
#include "GeometricObjects.h"
#include "ID.h"
#include "BoundBox.h"
#include "Colors.h"

class QTPainter : public QFrame, public Painter {

Q_OBJECT

private:
    // Selected objects
    std::unordered_map<ID, Color>selectedIDPoint;
    std::unordered_map<ID, Color> selectedIDCircle;
    std::unordered_map<ID, Color> selectedIDSection;
    std::unordered_map<ID, Color> selectedIDArc;

    // To move a segment, we remember the point of pressing
    QPointF pressLineVecBeg;
    QPointF pressLineVecEnd;
    QPointF pressPointCircle;

    // The highlighting area
    SelectedRectangle selectedRectangle;

    // Class for mouse rendering
    DrawMouse drawingWithMouse;
    bool drawing;

    // To avoid having to process multiple clicks
    QElapsedTimer lastClickTime;

public:
    QTPainter(QWidget* parent);

    QVector<ID> getVecSelectedIDPoints();
    QVector<ID> getVecSelectedIDSections();
    QVector<ID> getVecSelectedIDCircles();
    QVector<ID> getVecSelectedIDArcs();
    std::optional<QPair<ID, ID>> getPairSelectedID() const;

    void selectedClear();

    bool findClosesObject();
    void drawingFigures(QPainter& painter);
    void saveToImage(const QString& fileName, QString& format);
    void selectedElemByID(ID id, const std::string& type);

    void pointInRect(QRectF& rect);
    void sectionInRect(QRectF& rect);
    void circleInRect(QRectF& rect);
    void arcsInRect(QRectF& rect);

    /*******   Signals   ******/
    void onSigPoint(const QPointF& point);
    void onSigCircle(const QPointF& center,const qreal radius);
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
    void MovingSection(const QVector<ID>& selectedIDSections,const QPointF& begin,const QPointF& end);
    void MovingCircle(const QVector<ID>& selectedIDCircles,const QPointF& press);
    void MovingArc(const QVector<ID>& selectedIDArcs);

    void EndMoving();

    // Mouse rendering
    void SigPoint(const QPointF& point);
    void SigCircle(const QPointF& center,const qreal radius);
    void SigSection(const QPointF& stratPoint,const QPointF& endPoint);
    void SigArc(const QPointF& stratPoint,const QPointF& endPoint,const QPointF& centerPoint);

    void DoubleClickOnObject(ID id);

private slots:

    void onWorkWindowResized();
};

#endif // QTPAINTER_H