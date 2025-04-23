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

//#include "Painter.h"
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

    // Элемент левого меню
    std::vector<double> LeftMenuElem;

    // Класс для отрисовки мышкой
    DrawMouse drawFigM;
    bool drawing=false;

    std::chrono::steady_clock::time_point lastClickTime;

    SelectedRectangle SelectedRectangle;

public:
    QTPainter(QWidget *parent);

    void draw(); // Обновляет экран отрисовки

    void clear(); // Очистка экрана и приводит все параметры к базовым

    std::optional<std::pair<ID, ID>> getPairID() {
        std::vector<ID> IDs;

        for (const ID& id : selectedIDPoint) {
            IDs.push_back(id);
            if (IDs.size() > 2) { return std::nullopt; }
        }

        for (const ID& id : selectedIDSection) {
            IDs.push_back(id);
            if (IDs.size() > 2) { return std::nullopt; }
        }

        for (const ID& id : selectedIDCircle) {
            IDs.push_back(id);
            if (IDs.size() > 2) { return std::nullopt; }
        }

        if (IDs.size() == 2) {
            return std::make_pair(IDs[0], IDs[1]);
        }

        return std::nullopt;
    }


    std::vector<ID>& getVecIDPoints(){
        return selectedIDPoint;
    }
    std::vector<ID>& getVecIDSections(){
        return selectedIDSection;
    }
    std::vector<ID>& getVecIDCircles(){
        return selectedIDCircle;
    }

    void selectedClear();  // Очистка данных выделенных обьектов

    void drawingFigures(QPainter &painter); // Отрисовка всех фигур, дополняется отрисовкой выделений

    bool findClosesObject(); // Функция проверки курсора мышки и обьекта

    void saveToImage(const QString &fileName, QString &format); // Функция сохранения изображения в указанный формат

    // Функция выделения обьекта в левом меню
    void selectedElemByID(std::vector<double> &parameters,long long int IDmenu);

    // Отдаляет сцену
    void resizeRectangle();

    // Обработка сигналов для точки
    void onSigPoint(double x, double y) {
        emit SigPoint(x, y);
    }

    // Обработка сигналов для окружности
    void onSigCircle(double x, double y, double r) {
        emit SigCircle(x, y, r);
    }

    // Обработка сигналов для отрезка
    void onSigSection(double x, double y, double x1, double y1) {
        emit SigSection(x, y, x1, y1);
    }

    // Обработка сигналов для отрезка
    void onSigArc(double x, double y, double x1, double y1) {
        emit SigArc(x, y, x1, y1);
    }


protected:

    // Функция для изменения размеров окна
    void resizeEvent(QResizeEvent *event) override;

    // Стартовая точка для отрисовки всего холста
    void paintEvent(QPaintEvent *event) override;

    // При выходе за границы мы масштабируем
    void getBoundBox(const BoundBox2D& allObjects) override {
        Rectangle=&allObjects;
    }

    unsigned long long getWeight() override {
        return Scaling::getActualMonitorWidth();
    }

    unsigned long long getHeight() override {
        return Scaling::getActualMonitorHeight();
    }

    void initPoint( std::unordered_map<ID,  Point*>& points) override {
        pointStorage = &points;
    }

    void initSection( std::unordered_map<ID,  Section*>& sections) override {
        sectionStorage = &sections;
    }

    void initCircle( std::unordered_map<ID,  Circle*>& circles) override {
        circleStorage = &circles;
    }

    void initArc( std::unordered_map<ID,  Arc*>& arcs) override {
        arcStorage = &arcs;
    }

signals:

    // Перемещение
    void MovingPoint(const std::vector<ID>& selectedIDPoints);
    void MovingSection(const std::vector<ID>& selectedIDSections);
    void MovingCircle(const std::vector<ID>& selectedIDCircles);
    void MovingArc(const std::vector<ID>& selectedIDArcs);

    // При отрисовке мышкой
    void SigPoint(double x, double y);
    void SigCircle(double x, double y, double r);
    void SigSection(double x, double y, double x1, double y1);
    void SigArc(double x, double y, double x1, double y1);

    void SigSector();

    // Для отображения в левом меню
    void DoubleClickOnObject(ID id);

private slots:

    // Для связи с масштабирвоанием главного окна
    void onWorkWindowResized();


};


#endif // QTPAINTER_H