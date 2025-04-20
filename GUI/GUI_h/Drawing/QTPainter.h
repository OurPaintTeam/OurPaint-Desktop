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
    std::vector<long long int> selectedIdPoint;
    std::vector<long long int> selectedIdCircle;
    std::vector<long long int> selectedIdSection;

    // Указатели
    const std::unordered_map<ID, const Point*>* pointStorage;
    const std::unordered_map<ID, const Section*>* sectionStorage;
    const std::unordered_map<ID, const Circle*>* circleStorage;
    const BoundBox2D* Rectangle;

    // Элемент левого меню
    std::vector<double> LeftMenuElem;

    // Класс для отрисовки мышкой
    DrawMouse drawFigM;

    std::chrono::steady_clock::time_point lastClickTime;

    SelectedRectangle SelectedRectangle;

public:

    QTPainter(QWidget *parent);

    void draw(); // Обновляет экран отрисовки

    void clear(); // Очистка экрана и приводит все параметры к базовым

    void figureDelete(); // Удаление данных о фигурах

    std::vector<long long int> getVecID(); // Получение вектора айди для требований

    void selectedClear();  // Очистка данных выделенных обьектов

    bool matchesLeftMenu(const std::vector<double> &menu, const std::vector<double> &coords); // Вспомогательная функция поиска

    bool checkSelection(Element type, std::vector<ID> &selected, const std::vector<double> &coords); // Поиск выделенных обьектов

    void drawingFigures(QPainter &painter); // Отрисовка всех фигур, дополняется отрисовкой выделений

    bool findClosesObject(); // Функция проверки курсора мышки и обьекта

    void saveToImage(const QString &fileName, QString &format); // Функция сохранения изображения в указанный формат

    // Функция выделения обьекта в левом меню
    void selectedElemByID(std::vector<double> &parameters,unsigned long long int IDselected);

    void resizeRectangle();
    // Обработка сигналов если была отрисовка мышкой
    void onSigPoint(double x, double y) {
        emit SigPoint(x, y);
    }

    void onSigCircle(double x, double y, double r) {
        emit SigCircle(x, y, r);
    }

    void onSigSection(double x, double y, double x1, double y1) {
        emit SigSection(x, y, x1, y1);
    }



protected:
    // При выходе за границы мы масштабируем
    void changeSize(const BoundBox2D& allObjects) override {
        Rectangle=&allObjects;
    }

    unsigned long long getWeight() override {
        return Scaling::getActualMonitorWidth();
    }

    unsigned long long getHeight() override {
        return Scaling::getActualMonitorHeight();
    }

    void drawPoint(const std::unordered_map<ID, const Point*>& points) override {
        pointStorage = &points;
    }

    void drawSection(const std::unordered_map<ID, const Section*>& sections) override {
        sectionStorage = &sections;
    }

    void drawCircle(const std::unordered_map<ID, const Circle*>& circles) override {
        circleStorage = &circles;
    }



    // Функция для изменения размеров окна
    void resizeEvent(QResizeEvent *event)
    override;


    void paintEvent(QPaintEvent *event)
    override; // Стартовая точка для отрисовки всего холста

signals:
// Это невалидные сигналы. Сигналы не должны иметь реализацию
    void MovingFigures(); // При перемещении

    // При отрисовке мышкой
    void SigPoint(double x, double y);
    void SigCircle(double x, double y, double r);
    void SigSection(double x, double y, double x1, double y1);
    void SigSector();
    void SigArc();

    void DoubleClickOnObject(ID id);

private slots:

    void onWorkWindowResized();


};


#endif // QTPAINTER_H