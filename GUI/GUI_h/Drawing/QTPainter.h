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

#include "GeometricObjects.h"
#include "Painter.h"
#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include "DrawFigures.h"
#include "ClosesPoint.h"
#include "DrawBackground.h"
#include "DrawMouse.h"

class QTPainter : public QFrame, public Painter {
Q_OBJECT

private:
    // Актуальные обьеткы
    std::vector<Point> points;
    std::vector<Circle> circles;
    std::vector<Section> sections;

    //  Выделенные обьекты
    std::vector<ID> selectedIdPoint;
    std::vector<ID> selectedIdCircle;
    std::vector<ID> selectedIdSection;

    // Указатели
    const std::list<Point>* pointStorage;
    const std::list<Circle>* circleStorage;
    const std::list<Section>* sectionStorage;

    // Элемент левого меню
    std::vector<double> LeftMenuElem;

    // Класс для отрисовки мышкой
    DrawMouse drawFigM;

    ID id; // Айди фигуры выделения
    ID IDmove; // Айди фигуры перемещения

    std::chrono::steady_clock::time_point lastClickTime;

public:

    QTPainter(QWidget *parent);

    void draw(); // Обновляет экран отрисовки

    void clear(); // Очистка экрана и приводит все параметры к базовым

    void figureDelete(); // Удаление данных о фигурах

    void setIdFigures(ID Id); // Присвоение айди выделенной фигуры

    ID getIdFigures(); // Получение айди фигуры перемещения

    std::vector<ID> getVecID(); // Получение вектора айди для требований

    void selectedClear();  // Очистка данных выделенных обьектов

    bool matchesLeftMenu(const std::vector<double> &menu, const std::vector<double> &coords); // Вспомогательная функция поиска

    bool checkSelection(Element type, std::vector<ID> &selected, const std::vector<double> &coords); // Поиск выделенных обьектов

    void drawingFigures(QPainter &painter); // Отрисовка всех фигур, дополняется отрисовкой выделений

    bool findClosesObject(); // Функция проверки курсора мышки и обьекта

    void saveToImage(const QString &format); // Функция сохранения изображения в указанный формат

    // Функция выделения обьекта в левом меню
    void selectedElemByID(std::vector<double> &parameters,unsigned long long int IDselected);

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

    void emitId(Element type, const Point& p) {
        emit getIdFigure(type, p.x, p.y);
    }
    void emitId(Element type, const Section& s) {
        emit getIdFigure(type, s.beg->x, s.beg->y, s.end->x, s.end->y);
    }
    void emitId(Element type, const Circle& c) {
        emit getIdFigure(type, c.center->x, c.center->y, c.R);
    }


protected:
    // При выходе за границы мы масштабируем
    void changeSize(const rectangle &allObjects) override {}

    unsigned long long getWeight() override {
        return Scaling::getActualMonitorWidth();
    }

    unsigned long long getHeight() override {
        return Scaling::getActualMonitorHeight();
    }

    void drawPointt(const std::list<Point>& p) override {
        pointStorage = &p;
    }

    void drawCirclee(const std::list<Circle>& c) override {
        circleStorage = &c;
    }

    void drawSectionn(const std::list<Section>& s) override {
        sectionStorage = &s;
    }

    // Для отрисовки точек
    void drawPoint(Point pt, bool isWhite) override {
        points.push_back(pt);
    }

    // Для отрисовки кругов
    void drawCircle(Circle c, bool isWhite) override {
        circles.push_back(c);
    }

    // Для отрисовки линий
    void drawSection(Section sec, bool isWhite) override {
        sections.push_back(sec);
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

    // При выделении
    void getIdFigure(Element F, double x, double y);
    void getIdFigure(Element F, double x, double y, double r);
    void getIdFigure(Element F, double x, double y, double x1, double y1);

    void DoubleClickOnObject(ID id);

private slots:

    void onWorkWindowResized();


};


#endif // QTPAINTER_H