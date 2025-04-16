#ifndef OURPAINT_DRAWBACKGROUND_H
#define OURPAINT_DRAWBACKGROUND_H

#include <QPainter>

#include "DrawAdditionalInf.h"
#include "Scaling.h"
#include "Modes.h"

//  Класс отвечающий за отрисовку фона виджета
//  Центр у нас в центре виджета
//  Клетки создают эффект передвижения
//  Оси создают эффект бесконечности


class DrawBackground {
private:

public:
    explicit DrawBackground() = default;

    // Чтобы на сетке не было сильно дробных чисел ищем близкое значение размера клетки
    static double Step(double rawStep);

    // Отрисовка фона
    static void drawFon(QPainter &painter);

};

#endif //OURPAINT_DRAWBACKGROUND_H