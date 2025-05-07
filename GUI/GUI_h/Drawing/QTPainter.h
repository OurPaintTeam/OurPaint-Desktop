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

#include "Painter.h"
#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include "DrawFigures.h"
#include "ClosesPoint.h"
#include "DrawBackground.h"
#include "DrawMouse.h"
#include "SelectedRectangle.h"
#include "GeometricObjects.h"
#include "ID.h"
#include "BoundBox.h"

class QTPainter : public QFrame, public Painter {
Q_OBJECT

private:
    //  Выделенные обьекты
    std::vector<ID> selectedIDPoint;
    std::vector<ID> selectedIDCircle;
    std::vector<ID> selectedIDSection;
    std::vector<ID> selectedIDArc;

    // Указатели
     std::unordered_map<ID,  Point*>* pointStorage;
     std::unordered_map<ID,  Section*>* sectionStorage;
     std::unordered_map<ID,  Circle*>* circleStorage;
     std::unordered_map<ID,  Arc*>* arcStorage;
     const BoundBox2D* Rectangle;

     QPointF LineVecBeg;
     QPointF LineVecEnd;
     QPointF VecCircle;

    // Класс для отрисовки мышкой
    DrawMouse drawFigM;
    bool drawing=false;

    SelectedRectangle selectedRectangle;

    std::chrono::steady_clock::time_point lastClickTime;

public:
    QTPainter(QWidget *parent);

    std::vector<ID>& getVecSelectedIDPoints();
    std::vector<ID>& getVecSelectedIDSections();
    std::vector<ID>& getVecSelectedIDCircles();
    std::optional<std::pair<ID, ID>> getPairID();

    void draw();
    void clear();

    void selectedClear();
    void resizeRectangle();
    bool findClosesObject();
    void drawingFigures(QPainter &painter);
    void saveToImage(const QString &fileName, QString &format);

    void selectedElemByID();


    /*******   Сигналы   ******/
    void onSigPoint(double x, double y);
    void onSigCircle(double x, double y, double r);
    void onSigSection(double x, double y, double x1, double y1);
    void onSigArc(double x, double y, double x1, double y1, double xc, double yc);


protected:

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    unsigned long long getWeight() override ;
    unsigned long long getHeight() override;
    void getBoundBox(const BoundBox2D& allObjects) override;

    void initArc( std::unordered_map<ID,  Arc*>& arcs) override;
    void initPoint( std::unordered_map<ID,  Point*>& points) override;
    void initCircle( std::unordered_map<ID,  Circle*>& circles) override;
    void initSection( std::unordered_map<ID,  Section*>& sections) override;


signals:

    // Перемещение
    void MovingPoint(const std::vector<ID>& selectedIDPoints);
    void MovingSection(const std::vector<ID>& selectedIDSections,QPointF&,QPointF&);
    void MovingCircle(const std::vector<ID>& selectedIDCircles,QPointF&);
    void MovingArc(const std::vector<ID>& selectedIDArcs);

    // При отрисовке мышкой
    void SigPoint(double x, double y);
    void SigCircle(double x, double y, double r);
    void SigSection(double x, double y, double x1, double y1);
    void SigArc(double x, double y, double x1, double y1, double xc, double yc);


    void DoubleClickOnObject(ID id);

private slots:
    void onWorkWindowResized();


};


#endif // QTPAINTER_H