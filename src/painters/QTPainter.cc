#include "QTPainter.h"

QTPainter::QTPainter(Ui::MainWindow *ui, QWidget *parent)
        : QFrame(parent), ui(ui), scaling(ui->workWindow->width(), ui->workWindow->height()),
          minCellSize(10), maxCellSize(20), CellSize(8), CellView(true), currentCellSize(1), cursorX(0), cursorY(0),
          editor(true), Circle_(false), Section_(false), Point_(false), Drawing(false), leftClick(false),
          ReleaseLeftClick(false),
          leftDoubleClick(false), Shift(false), tab(0), id(0) {

    setMouseTracking(true); // Отслеживанием мышь

    resize(parent->size());
    setStyleSheet("background: \"#ffffff\"");

    // Отслеживаем размер окна родителя
    connect(ui->workWindow->window(), SIGNAL(resized()), this, SLOT(onWorkWindowResized()));
    setAttribute(Qt::WA_AcceptTouchEvents);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

QPointF QTPainter::MouseCoordinate() {
    double dX = cursorX - currentCursorX;
    double dY = -cursorY + currentCursorY;
    /* const double EPS=0.00001;

    if (Shift) {
         if (std::abs(dX) - std::abs(dY)>EPS) {
             return {scaling.logic(cursorX - currentCursorX), 0.0};
         } else {
             return {0.0, scaling.logic(dY)};
         }

     }*/
    return {scaling.logic(dX), scaling.logic(dY)};
}

void QTPainter::clear() {
    points.clear();
    circles.clear();
    sections.clear();
    scaling.setZoomZero();
    update();
}


void QTPainter::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
}

void QTPainter::drawPoint(struct Point pt, bool isWhite) {
    points.push_back(pt);
}

void QTPainter::drawCircle(struct Circle c, bool isWhite) {
    circles.push_back(c);
}

void QTPainter::drawSection(struct Section sec, bool isWhite) {
    sections.push_back(sec);
}

void QTPainter::setCell(bool On_Off) {
    CellView = On_Off;
}

void QTPainter::draw() {
    update();
}

void QTPainter::setZoomPlus() {
    double max_zoom = (ui->workWindow->width()) / 16.0 / scaling.getScale();
    scaling.setZoomPlus(max_zoom);
    update();
}

void QTPainter::setZoomMinus() {
    scaling.setZoomMinus();
    update();
}

void QTPainter::setZoomZero() {
    scaling.setZoomZero();
    points.clear();
    circles.clear();
    sections.clear();
    update();
}

std::vector<double> QTPainter::FindMaxMin() {
    std::vector<double> size;

    if (!points.empty() || !circles.empty() || !sections.empty()) {
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();

        if (!points.empty()) {
            for (const auto &pt: points) {
                minX = std::min(minX, pt.x);
                maxX = std::max(maxX, pt.x);
                minY = std::min(minY, pt.y);
                maxY = std::max(maxY, pt.y);
            }
        }

        if (!circles.empty()) {
            for (const auto &c: circles) {
                minX = std::min(minX, c.center->x - c.R);
                maxX = std::max(maxX, c.center->x + c.R);
                minY = std::min(minY, c.center->y - c.R);
                maxY = std::max(maxY, c.center->y + c.R);
            }
        }

        if (!sections.empty()) {
            for (const auto &sec: sections) {
                minX = std::min({minX, sec.beg->x, sec.end->x});
                maxX = std::max({maxX, sec.beg->x, sec.end->x});
                minY = std::min({minY, sec.beg->y, sec.end->y});
                maxY = std::max({maxY, sec.beg->y, sec.end->y});
            }
        }

        size = {maxX, minX, maxY, minY};
    } else {
        size = {};
    }

    return size;
}

// Функция проверки координат курсора и точки для перемещения
bool QTPainter::moving(double x, double y) {
    double RANGE = 3.0;
    if (RANGE / scaling.getZoom() > 1.0) {
        RANGE /= scaling.getZoom();
    }
    double _width = width() / 2.0;
    double _height = height() / 2.0;

    double cursorXDouble = scaling.logicX(cursorX - _width);
    double cursorYDouble = scaling.logicY(_height - cursorY);

    if (std::abs(cursorXDouble - x) <= RANGE &&
        std::abs(cursorYDouble - y) <= RANGE) {
        emit Move(ET_POINT, x, y);
        return true;
    }

    return false;
}

// Функция проверки координат курсора и круга для перемещения
bool QTPainter::moving(double x0, double y0, double r) {
    double _width = width() / 2.0;
    double _height = height() / 2.0;

    double mouseX = scaling.logicX(cursorX - _width);
    double mouseY = scaling.logicY(_height - cursorY);

    double dx = (mouseX - x0);
    double dy = (mouseY - y0);

    double distance = std::sqrt(dx * dx + dy * dy);

    const double RANGE = 5.0;

    if (std::abs(distance - r) <= RANGE) {
        emit Move(ET_CIRCLE, x0, y0, r);
        return true;
    }

    return false;
}

// Функция проверки координат курсора и прямой для перемещения
bool QTPainter::moving(double x0, double y0, double x1, double y1) {
    // Вычисляем вектор направления между начальной и конечной точками
    double dx = x1 - x0;
    double dy = y1 - y0;
    double dist = sqrt(dx * dx + dy * dy);
/*    if (dist > 7) {
        // Нормализуем вектор направления
        double unitX = dx / dist;
        double unitY = dy / dist;

        // Сдвигаем начальную и конечную точки на 7 в сторону друг друга
        x0 += unitX * 7;
        y0 += unitY * 7;
        x1 -= unitX * 7;
        y1 -= unitY * 7;
    }
*/
    double RANGE = 2.0;

    if (RANGE / scaling.getZoom() > 1.0) {
        RANGE /= scaling.getZoom();
    }

    double _width = width() / 2.0;
    double _height = height() / 2.0;

    double mouseX = scaling.logicX(cursorX - _width);
    double mouseY = scaling.logicY(_height - cursorY);

    double distance = distancePointToSection(mouseX, mouseY, x0, y0, x1, y1);

    if (distance <= RANGE) {
        emit Move(ET_SECTION,x0, y0, x1, y1);
        std::cout << x0 << " " << y0 << " " << x1 << " " << y1 << std::endl;
        return true;
    }

    return false;
}

// Вспомогательная функция для проверки курсора и линии
double QTPainter::distancePointToSection(double px, double py, double x0, double y0, double x1, double y1) {
    double dx = x1 - x0;
    double dy = y1 - y0;

    if (dx * dx + dy * dy == 0.0) {
        dx = px - x0;
        dy = py - y0;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Параметр t для проекции
    double t = ((px - x0) * dx + (py - y0) * dy) / (dx * dx + dy * dy);

    // Погрешность
    t = std::max(0.0, std::min(1.0, t));

    // Ближайшая точка на отрезке
    double nearestX = x0 + t * dx;
    double nearestY = y0 + t * dy;

    // Расстояние до ближайшей точки
    dx = px - nearestX;
    dy = py - nearestY;

    return std::sqrt(dx * dx + dy * dy);
}

// Отрисовка фона
void QTPainter::drawBackground(QPainter &painter) {
    double scale = scaling.getScale();
    double zoom = scaling.getZoom();
    double _width = width() / 2.0;
    double _height = height() / 2.0;
    int deltaX = scaling.getDeltaX();
    int deltaY = scaling.getDeltaY();

// Отрисовка клетчатого фона
    if (CellView) {
        double ZoomCell = zoom;
        currentCellSize = static_cast<int>(CellSize * scale * ZoomCell);
        if (currentCellSize > maxCellSize) {
            while (currentCellSize > maxCellSize) {
                ZoomCell /= 2.0;
                currentCellSize = static_cast<int>(CellSize * scale * ZoomCell);
            }
        } else if (currentCellSize < minCellSize) {
            while (currentCellSize < minCellSize) {
                ZoomCell *= 2.0;
                currentCellSize = static_cast<int>(CellSize * scale * ZoomCell);
            }
        }

        int index = 1; // Делаем блоки 5 на 5
        for (int x = _width - currentCellSize, x1 = _width + currentCellSize;x + abs(deltaX) >= 0; x -= currentCellSize, x1 += currentCellSize) {
            index % 5 == 0 ? painter.setPen(Qt::darkGray) : painter.setPen(Qt::lightGray);
            painter.drawLine(x + deltaX, 0, x + deltaX, height());
            painter.drawLine(x1 + deltaX, 0, x1 + deltaX, height());
            ++index;
        }
        index = 1;
        for (int y = _height - currentCellSize, y1 = _height + currentCellSize;
             y + abs(deltaY) >= 0; y -= currentCellSize, y1 += currentCellSize) {
            index % 5 == 0 ? painter.setPen(Qt::darkGray) : painter.setPen(Qt::lightGray);
            painter.drawLine(0, y + deltaY, width(), y + deltaY);
            painter.drawLine(0, y1 + deltaY, width(), y1 + deltaY);
            ++index;
        }
    }


// Оси координат
    painter.setPen(Qt::black);
    painter.drawLine(static_cast<int>(_width) + deltaX, 0, static_cast<int>(_width) + deltaX,
                     static_cast<int>(height()));
    painter.drawLine(0, static_cast<int>(_height) + deltaY, static_cast<int>(width()),
                     static_cast<int>(_height) + deltaY);


// Отрисовка значений координат
    int widthX = static_cast<int>(_width / (scale * zoom));
    int heightY = static_cast<int>(_height / (scale * zoom));
    if ((width() + deltaX - 30 > width()) || (_height + deltaY - 5 > height()) ||
        (_width + deltaX - 30 > width()) || (deltaY + 10 > height())) {
        painter.drawText(10, 10, QString::number(widthX));
        painter.drawText(10, 30, QString::number(heightY));
    } else {
        painter.drawText(static_cast<int>(width()) + deltaX - 30, static_cast<int>(_height) + deltaY - 5,
                         QString::number(widthX));
        painter.drawText(static_cast<int>(_width) + deltaX - 30, deltaY + 10, QString::number(heightY));
    }

    // Отрисовка координат на курсоре
    painter.setPen(Qt::black);
    int logicalX = static_cast<int>(scaling.logicX(cursorX - _width));
    int logicalY = static_cast<int>(scaling.logicY(_height - cursorY));
    if (id == 0) {
        painter.drawText(cursorX + 10, cursorY - 10, QString("X: %1, Y: %2").arg(logicalX).arg(logicalY));
    } else {
        painter.drawText(cursorX + 10, cursorY - 10,
                         QString("X: %1, Y: %2, ID: %3").arg(logicalX).arg(logicalY).arg(id.id));
    }
}

// Отрисовка отрезков
void QTPainter::drawSections(QPainter &painter) {

    double _width = width() / 2.0;
    double _height = height() / 2.0;

    bool AllClicked = false;

    std::vector<Point> newSelectedPoints;
    std::vector<Circle> newSelectedCircles;
    std::vector<Section> newSelectedSections;


    if (leftClick && (Moving || editor)) {

        // Обработка точек
        for (const auto &pt: points) {
            if (moving(pt.x, pt.y)) {
                AllClicked = true;
                bool selected = false;
                for (const auto &c: ReqieredPoints) {
                    if (c.x == pt.x && c.y == pt.y) {
                        selected = true;
                        break;
                    }
                }


                if (!selected) {
                    Point ptr;
                    ptr.x = pt.x;
                    ptr.y = pt.y;
                    newSelectedPoints.push_back(ptr);
                }
            }
        }

        // Обработка кругов
        for (const auto &pt: circles) {
            if (moving(pt.center->x, pt.center->y, pt.R)) {
                AllClicked = true;
                bool selected = false;
                for (const auto &c: ReqieredCircles) {
                    if (c.center->x == pt.center->x && c.center->y == pt.center->y && c.R == pt.R) {
                        selected = true;
                        break;
                    }
                }

                if (!selected) {
                    Circle cir;
                    cir.center = new Point();
                    cir.center->x = pt.center->x;
                    cir.center->y = pt.center->y;
                    cir.R = pt.R;
                    newSelectedCircles.push_back(cir);
                }
            }
        }

        // Обработка линий
        for (const auto &sec: sections) {
            if (moving(sec.beg->x, sec.beg->y, sec.end->x, sec.end->y)) {
                AllClicked = true;
                bool selected = false;
                for (const auto &sect: ReqieredSections) {
                    if (sec.beg->x == sect.beg->x && sec.beg->y == sect.beg->y &&
                        sec.end->x == sect.end->x && sec.end->y == sect.end->y) {
                        selected = true;
                        break;
                    }
                }

                if (!selected) {
                    Section newSec;
                    newSec.beg = new Point();
                    newSec.end = new Point();

                    newSec.beg->x = sec.beg->x;
                    newSec.beg->y = sec.beg->y;

                    newSec.end->x = sec.end->x;
                    newSec.end->y = sec.end->y;

                    newSelectedSections.push_back(newSec);
                }
            }
        }
        qDebug() << Shift;

        if (Shift) {
            if (id != 0) {
                ID _id(id);
                if (std::find(vec_id.begin(), vec_id.end(), _id) == vec_id.end()) {
                    vec_id.push_back(_id);
                }
            }
        } else {
            vec_id.clear();
        }

        ReqieredPoints.insert(ReqieredPoints.end(), newSelectedPoints.begin(), newSelectedPoints.end());
        ReqieredCircles.insert(ReqieredCircles.end(), newSelectedCircles.begin(), newSelectedCircles.end());
        ReqieredSections.insert(ReqieredSections.end(), newSelectedSections.begin(), newSelectedSections.end());

        if (!AllClicked) {
            ReqieredPoints.clear();
            ReqieredCircles.clear();
            ReqieredSections.clear();
        }

    }


    // Отрисовка точек
    for (const auto &pt: points) {
        int X = static_cast<int>(scaling.scaleCoordinateX(pt.x) + _width);
        int Y = static_cast<int>(scaling.scaleCoordinateY(-pt.y) + _height );

        bool flag = false;
        if (editor || Moving) {
            for (const auto &c: ReqieredPoints) {
                if (c.x == pt.x && c.y == pt.y) {
                    flag = true;
                    break;
                }
            }
        }

        if (flag) {
            // Рисуем свечение для выбранной точки
            qreal glowRadius = 5.0;
            QRadialGradient radialGradient(X, Y, glowRadius, X, Y);
            radialGradient.setColorAt(0.0, QColor(0, 128, 255, 200));
            radialGradient.setColorAt(1.0, QColor(0, 128, 255, 0));

            QBrush glowBrush(radialGradient);
            painter.setBrush(glowBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - glowRadius, Y - glowRadius, glowRadius * 2, glowRadius * 2);

            // Рисуем основную черную точку поверх свечения
            qreal pointRadius = 2.0;
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2);
        } else {
            // Рисуем обычную черную точку без свечения
            qreal pointRadius = 1.0;
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2);
        }
    }

    // Отрисовка кругов
    for (const auto &c: circles) {
        int X = static_cast<int>(scaling.scaleCoordinateX(c.center->x) + _width );
        int Y = static_cast<int>(scaling.scaleCoordinateY(-c.center->y) + _height );
        int Radius = 2 * static_cast<int>(scaling.scaleCoordinate(c.R));

        bool flag = false;
        if (editor || Moving) {
            for (const auto &cir: ReqieredCircles) {
                if (c.center->x == cir.center->x && c.center->y == cir.center->y && c.R == cir.R) {
                    flag = true;
                    break;
                }
            }
        }

        if (flag) {
            // Параметры свечения
            qreal glowDistance = 20.0; // Расстояние свечения от обводки
            qreal gradientRadius = (Radius / 2.0) + glowDistance; // Общий радиус градиента

            // Вычисляем относительные позиции для цветовых остановок
            qreal stopTransparent1 = (Radius / 2.0 - glowDistance) / gradientRadius;
            qreal stopCyan = (Radius / 2.0) / gradientRadius;
            qreal stopTransparent2 = (Radius / 2.0 + glowDistance) / gradientRadius;

            // Создаём радиальный градиент
            QRadialGradient radialGradient(X, Y, gradientRadius, X, Y);
            radialGradient.setColorAt(0.0, QColor(0, 255, 255, 0));         // Прозрачный в центре
            radialGradient.setColorAt(stopTransparent1,
                                      QColor(0, 255, 255, 0));   // Прозрачный до внутренней границы свечения
            radialGradient.setColorAt(stopCyan, QColor(0, 255, 255, 150));      // Насыщенный циан на обводке
            radialGradient.setColorAt(stopTransparent2,
                                      QColor(0, 255, 255, 0)); // Прозрачный после внешней границы свечения
            radialGradient.setColorAt(1.0, QColor(0, 255, 255, 0));            // Прозрачный за пределами свечения

            // Настраиваем перо для свечения
            QPen glowPen(QBrush(radialGradient), 4); // Толщина пера 4 пикселя
            glowPen.setJoinStyle(Qt::RoundJoin);
            glowPen.setCapStyle(Qt::RoundCap);
            painter.setPen(glowPen);
            painter.setBrush(Qt::NoBrush); // Без заливки

            // Рисуем круг с градиентной обводкой для свечения
            painter.drawEllipse(X - Radius / 2.0, Y - Radius / 2.0, Radius, Radius);

            // Рисуем основную обводку черного круга поверх свечения
            QPen mainPen(Qt::black, 2);
            mainPen.setJoinStyle(Qt::RoundJoin);
            mainPen.setCapStyle(Qt::RoundCap);
            painter.setPen(mainPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(X - Radius / 2.0, Y - Radius / 2.0, Radius, Radius);
        } else {
            QPen blackPen(Qt::black, 1);
            blackPen.setJoinStyle(Qt::RoundJoin);
            blackPen.setCapStyle(Qt::RoundCap);
            painter.setPen(blackPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(X - Radius / 2.0, Y - Radius / 2.0, Radius, Radius);
        }
    }

    // Отрисовка линий
    for (const auto &sec: sections) {

        int BegX = static_cast<int>(scaling.scaleCoordinateX(sec.beg->x) + _width );
        int BegY = static_cast<int>(scaling.scaleCoordinateY(-sec.beg->y) + _height );
        int EndX = static_cast<int>(scaling.scaleCoordinateX(sec.end->x) + _width );
        int EndY = static_cast<int>(scaling.scaleCoordinateY(-sec.end->y) + _height );

        bool flag = false;
        if (editor || Moving) {
            for (const auto &Rsec: ReqieredSections) {
                if (sec.beg->x == Rsec.beg->x && sec.beg->y == Rsec.beg->y &&
                    sec.end->x == Rsec.end->x && sec.end->y == Rsec.end->y) {
                    flag = true;
                    break;
                }
            }
        }

        if (flag) {
            // Вычисляем направление линии
            qreal dx = EndX - BegX;
            qreal dy = EndY - BegY;
            qreal length = qSqrt(dx * dx + dy * dy);

            if (length == 0) length = 1;

            // Перпендикулярный вектор
            qreal perpX = -dy / length;
            qreal perpY = dx / length;

            // Задаем расстояние для градиента
            qreal glowDistance = 10.0;

            // Определяем точки начала и конца градиента
            qreal gradStartX = BegX + perpX * glowDistance;
            qreal gradStartY = BegY + perpY * glowDistance;
            qreal gradEndX = BegX - perpX * glowDistance;
            qreal gradEndY = BegY - perpY * glowDistance;

            // Создаём линейный градиент перпендикулярный линии
            QLinearGradient gradient(gradStartX, gradStartY, gradEndX, gradEndY);
            gradient.setColorAt(0.0, QColor(0, 255, 255, 150)); // Голубой с прозрачностью
            gradient.setColorAt(1.0, QColor(0, 255, 255, 0));   // Прозрачный

            QPen glowPen;
            glowPen.setWidth(8);
            glowPen.setBrush(gradient);
            glowPen.setCapStyle(Qt::RoundCap); // Устанавливаем закругленные концы
            painter.setPen(glowPen);

            painter.drawLine(BegX, BegY, EndX, EndY);

            // Рисуем основную линию поверх свечения
            QPen blackPen(Qt::black);
            blackPen.setWidth(2);
            blackPen.setCapStyle(Qt::RoundCap); // Устанавливаем закругленные концы
            painter.setPen(blackPen);
            painter.drawLine(BegX, BegY, EndX, EndY);

            QFont font = painter.font();
            font.setPointSize(10);
            painter.setFont(font);
            painter.setPen(Qt::black);
        } else {
            QPen blackPen(Qt::black);
            blackPen.setWidth(1);
            blackPen.setCapStyle(Qt::RoundCap); // Устанавливаем закругленные концы
            painter.setPen(blackPen);
            painter.drawLine(BegX, BegY, EndX, EndY);
        }

        drawCoordinateLine(painter, sec.beg->x, sec.beg->y, sec.end->x, sec.end->y);
    }
}

// Отрисовка кругов
void QTPainter::drawCircles(QPainter &painter) {

    double _width = width() / 2.0;
    double _height = height() / 2.0;

    bool AllClicked = false;

    std::vector<Point> newSelectedPoints;
    std::vector<Circle> newSelectedCircles;
    std::vector<Section> newSelectedSections;


    if (leftClick && (Moving || editor)) {

        // Обработка точек
        for (const auto &pt: points) {
            if (moving(pt.x, pt.y)) {
                AllClicked = true;
                bool selected = false;
                for (const auto &c: ReqieredPoints) {
                    if (c.x == pt.x && c.y == pt.y) {
                        selected = true;
                        break;
                    }
                }


                if (!selected) {
                    Point ptr;
                    ptr.x = pt.x;
                    ptr.y = pt.y;
                    newSelectedPoints.push_back(ptr);
                }
            }
        }

        // Обработка кругов
        for (const auto &pt: circles) {
            if (moving(pt.center->x, pt.center->y, pt.R)) {
                AllClicked = true;
                bool selected = false;
                for (const auto &c: ReqieredCircles) {
                    if (c.center->x == pt.center->x && c.center->y == pt.center->y && c.R == pt.R) {
                        selected = true;
                        break;
                    }
                }

                if (!selected) {
                    Circle cir;
                    cir.center = new Point();
                    cir.center->x = pt.center->x;
                    cir.center->y = pt.center->y;
                    cir.R = pt.R;
                    newSelectedCircles.push_back(cir);
                }
            }
        }

        // Обработка линий
        for (const auto &sec: sections) {
            if (moving(sec.beg->x, sec.beg->y, sec.end->x, sec.end->y)) {
                AllClicked = true;
                bool selected = false;
                for (const auto &sect: ReqieredSections) {
                    if (sec.beg->x == sect.beg->x && sec.beg->y == sect.beg->y &&
                        sec.end->x == sect.end->x && sec.end->y == sect.end->y) {
                        selected = true;
                        break;
                    }
                }

                if (!selected) {
                    Section newSec;
                    newSec.beg = new Point();
                    newSec.end = new Point();

                    newSec.beg->x = sec.beg->x;
                    newSec.beg->y = sec.beg->y;

                    newSec.end->x = sec.end->x;
                    newSec.end->y = sec.end->y;

                    newSelectedSections.push_back(newSec);
                }
            }
        }
        qDebug() << Shift;

        if (Shift) {
            if (id != 0) {
                ID _id(id);
                if (std::find(vec_id.begin(), vec_id.end(), _id) == vec_id.end()) {
                    vec_id.push_back(_id);
                }
            }
        } else {
            vec_id.clear();
        }

        ReqieredPoints.insert(ReqieredPoints.end(), newSelectedPoints.begin(), newSelectedPoints.end());
        ReqieredCircles.insert(ReqieredCircles.end(), newSelectedCircles.begin(), newSelectedCircles.end());
        ReqieredSections.insert(ReqieredSections.end(), newSelectedSections.begin(), newSelectedSections.end());

        if (!AllClicked) {
            ReqieredPoints.clear();
            ReqieredCircles.clear();
            ReqieredSections.clear();
        }

    }


    // Отрисовка точек
    for (const auto &pt: points) {
        int X = static_cast<int>(scaling.scaleCoordinateX(pt.x) + _width);
        int Y = static_cast<int>(scaling.scaleCoordinateY(-pt.y) + _height );

        bool flag = false;
        if (editor || Moving) {
            for (const auto &c: ReqieredPoints) {
                if (c.x == pt.x && c.y == pt.y) {
                    flag = true;
                    break;
                }
            }
        }

        if (flag) {
            // Рисуем свечение для выбранной точки
            qreal glowRadius = 5.0;
            QRadialGradient radialGradient(X, Y, glowRadius, X, Y);
            radialGradient.setColorAt(0.0, QColor(0, 128, 255, 200));
            radialGradient.setColorAt(1.0, QColor(0, 128, 255, 0));

            QBrush glowBrush(radialGradient);
            painter.setBrush(glowBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - glowRadius, Y - glowRadius, glowRadius * 2, glowRadius * 2);

            // Рисуем основную черную точку поверх свечения
            qreal pointRadius = 2.0;
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2);
        } else {
            // Рисуем обычную черную точку без свечения
            qreal pointRadius = 1.0;
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2);
        }
    }

    // Отрисовка кругов
    for (const auto &c: circles) {
        int X = (scaling.scaleCoordinateX(c.center->x) + _width);
        int Y = static_cast<int>(scaling.scaleCoordinateY(-c.center->y) + _height );
        int Radius = 2 * static_cast<int>(scaling.scaleCoordinate(c.R));

        bool flag = false;
        if (editor || Moving) {
            for (const auto &cir: ReqieredCircles) {
                if (c.center->x == cir.center->x && c.center->y == cir.center->y && c.R == cir.R) {
                    flag = true;
                    break;
                }
            }
        }

        if (flag) {
            // Параметры свечения
            qreal glowDistance = 20.0; // Расстояние свечения от обводки
            qreal gradientRadius = (Radius / 2.0) + glowDistance; // Общий радиус градиента

            // Вычисляем относительные позиции для цветовых остановок
            qreal stopTransparent1 = (Radius / 2.0 - glowDistance) / gradientRadius;
            qreal stopCyan = (Radius / 2.0) / gradientRadius;
            qreal stopTransparent2 = (Radius / 2.0 + glowDistance) / gradientRadius;

            // Создаём радиальный градиент
            QRadialGradient radialGradient(X, Y, gradientRadius, X, Y);
            radialGradient.setColorAt(0.0, QColor(0, 255, 255, 0));         // Прозрачный в центре
            radialGradient.setColorAt(stopTransparent1,
                                      QColor(0, 255, 255, 0));   // Прозрачный до внутренней границы свечения
            radialGradient.setColorAt(stopCyan, QColor(0, 255, 255, 150));      // Насыщенный циан на обводке
            radialGradient.setColorAt(stopTransparent2,
                                      QColor(0, 255, 255, 0)); // Прозрачный после внешней границы свечения
            radialGradient.setColorAt(1.0, QColor(0, 255, 255, 0));            // Прозрачный за пределами свечения

            // Настраиваем перо для свечения
            QPen glowPen(QBrush(radialGradient), 4); // Толщина пера 4 пикселя
            glowPen.setJoinStyle(Qt::RoundJoin);
            glowPen.setCapStyle(Qt::RoundCap);
            painter.setPen(glowPen);
            painter.setBrush(Qt::NoBrush); // Без заливки

            // Рисуем круг с градиентной обводкой для свечения
            painter.drawEllipse(X - Radius / 2.0, Y - Radius / 2.0, Radius, Radius);

            // Рисуем основную обводку черного круга поверх свечения
            QPen mainPen(Qt::black, 2);
            mainPen.setJoinStyle(Qt::RoundJoin);
            mainPen.setCapStyle(Qt::RoundCap);
            painter.setPen(mainPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(X - Radius / 2.0, Y - Radius / 2.0, Radius, Radius);
        } else {
            QPen blackPen(Qt::black, 1);
            blackPen.setJoinStyle(Qt::RoundJoin);
            blackPen.setCapStyle(Qt::RoundCap);
            painter.setPen(blackPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(X - Radius / 2.0, Y - Radius / 2.0, Radius, Radius);
        }
    }

    // Отрисовка линий
    for (const auto &sec: sections) {

        int BegX = static_cast<int>(scaling.scaleCoordinateX(sec.beg->x) + _width );
        int BegY = static_cast<int>(scaling.scaleCoordinateY(-sec.beg->y) + _height );
        int EndX = static_cast<int>(scaling.scaleCoordinateX(sec.end->x) + _width );
        int EndY = static_cast<int>(scaling.scaleCoordinateY(-sec.end->y) + _height );

        bool flag = false;
        if (editor || Moving) {
            for (const auto &Rsec: ReqieredSections) {
                if (sec.beg->x == Rsec.beg->x && sec.beg->y == Rsec.beg->y &&
                    sec.end->x == Rsec.end->x && sec.end->y == Rsec.end->y) {
                    flag = true;
                    break;
                }
            }
        }

        if (flag) {
            // Вычисляем направление линии
            qreal dx = EndX - BegX;
            qreal dy = EndY - BegY;
            qreal length = qSqrt(dx * dx + dy * dy);

            if (length == 0) length = 1;

            // Перпендикулярный вектор
            qreal perpX = -dy / length;
            qreal perpY = dx / length;

            // Задаем расстояние для градиента
            qreal glowDistance = 10.0;

            // Определяем точки начала и конца градиента
            qreal gradStartX = BegX + perpX * glowDistance;
            qreal gradStartY = BegY + perpY * glowDistance;
            qreal gradEndX = BegX - perpX * glowDistance;
            qreal gradEndY = BegY - perpY * glowDistance;

            // Создаём линейный градиент перпендикулярный линии
            QLinearGradient gradient(gradStartX, gradStartY, gradEndX, gradEndY);
            gradient.setColorAt(0.0, QColor(0, 255, 255, 150)); // Голубой с прозрачностью
            gradient.setColorAt(1.0, QColor(0, 255, 255, 0));   // Прозрачный

            QPen glowPen;
            glowPen.setWidth(8);
            glowPen.setBrush(gradient);
            glowPen.setCapStyle(Qt::RoundCap); // Устанавливаем закругленные концы
            painter.setPen(glowPen);

            painter.drawLine(BegX, BegY, EndX, EndY);

            // Рисуем основную линию поверх свечения
            QPen blackPen(Qt::black);
            blackPen.setWidth(2);
            blackPen.setCapStyle(Qt::RoundCap); // Устанавливаем закругленные концы
            painter.setPen(blackPen);
            painter.drawLine(BegX, BegY, EndX, EndY);

            QFont font = painter.font();
            font.setPointSize(10);
            painter.setFont(font);
            painter.setPen(Qt::black);
        } else {
            QPen blackPen(Qt::black);
            blackPen.setWidth(1);
            blackPen.setCapStyle(Qt::RoundCap); // Устанавливаем закругленные концы
            painter.setPen(blackPen);
            painter.drawLine(BegX, BegY, EndX, EndY);
        }

        drawCoordinateLine(painter, sec.beg->x, sec.beg->y, sec.end->x, sec.end->y);
    }
}


void QTPainter::drawExp(QPainter &painter) {

    double _width = width() / 2.0;
    double _height = height() / 2.0;

    // Отрисовка точек
    for (const auto &pt: points) {
        int X = static_cast<int>(scaling.scaleCoordinateX(pt.x) + _width);
        int Y = static_cast<int>(scaling.scaleCoordinateY(-pt.y) + _height );

        bool flag = false;
        if (leftClick && (editor || Moving) && moving(pt.x, pt.y)) {
            if (Shift) {
                bool key=true;
                for (const auto &ptr: ReqieredPoints) {
                    if (ptr.x == pt.x && ptr.y == pt.y) {
                        key=false;
                        break;
                    }
                }

                if(key) {
                    Point ptr;
                    ptr.x = pt.x;
                    ptr.y = pt.y;
                    ReqieredPoints.push_back(ptr);
                }
            }
            flag = true;
        }

        if (!flag && Shift)
            for (auto &ptr: ReqieredPoints) {
                if (ptr.x == pt.x && ptr.y == pt.y) {
                    flag=true;
                    break;
                }
            }

        if(!Shift){
            ReqieredPoints.clear();
        }

        if (flag) {
            // Рисуем свечение для выбранной точки
            qreal glowRadius = 5.0;
            QRadialGradient radialGradient(X, Y, glowRadius, X, Y);
            radialGradient.setColorAt(0.0, QColor(0, 128, 255, 200));
            radialGradient.setColorAt(1.0, QColor(0, 128, 255, 0));

            QBrush glowBrush(radialGradient);
            painter.setBrush(glowBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - glowRadius, Y - glowRadius, glowRadius * 2, glowRadius * 2);

            // Рисуем основную черную точку поверх свечения
            qreal pointRadius = 2.0;
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2);
        } else {
            // Рисуем обычную черную точку без свечения
            qreal pointRadius = 1.0;
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2);
        }
    }
}

// Отрисовка точек
void QTPainter::drawPoints(QPainter &painter) {

    double _width = width() / 2.0;
    double _height = height() / 2.0;

    bool NotClicked = false;

    std::vector<Point> newSelectedPoints;

    if (leftClick && (Moving || editor)) {

        // Обработка точек
        for (const auto &pt: points) {
            if (moving(pt.x, pt.y)) {
                if (Shift && Moving) {
                    NotClicked = true;
                    bool selected = false;
                    for (const auto &c: ReqieredPoints) {
                        if (c.x == pt.x && c.y == pt.y) {
                            selected = true;
                            break;
                        }
                    }


                    if (!selected) {
                        Point ptr;
                        ptr.x = pt.x;
                        ptr.y = pt.y;
                        newSelectedPoints.push_back(ptr);
                    }
                } else {
                    Point ptr;
                    ptr.x = pt.x;
                    ptr.y = pt.y;
                    newSelectedPoints.push_back(ptr);
                }
            }
        }

        if (Shift) {
            if (id != 0) {
                ID _id(id);
                if (std::find(vec_id.begin(), vec_id.end(), _id) == vec_id.end()) {
                    vec_id.push_back(_id);
                }
            }
        } else {
            vec_id.clear();
        }

        ReqieredPoints.insert(ReqieredPoints.end(), newSelectedPoints.begin(), newSelectedPoints.end());

        if (!NotClicked) {
            ReqieredPoints.clear();
        }

    }


    // Отрисовка точек
    for (const auto &pt: points) {
        int X = static_cast<int>(scaling.scaleCoordinateX(pt.x) + _width);
        int Y = static_cast<int>(scaling.scaleCoordinateY(-pt.y) + _height );

        bool flag = false;
        if (editor || Moving) {
            for (const auto &c: ReqieredPoints) {
                if (c.x == pt.x && c.y == pt.y) {
                    flag = true;
                    break;
                }
            }
        }

        if (flag) {
            // Рисуем свечение для выбранной точки
            qreal glowRadius = 5.0;
            QRadialGradient radialGradient(X, Y, glowRadius, X, Y);
            radialGradient.setColorAt(0.0, QColor(0, 128, 255, 200));
            radialGradient.setColorAt(1.0, QColor(0, 128, 255, 0));

            QBrush glowBrush(radialGradient);
            painter.setBrush(glowBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - glowRadius, Y - glowRadius, glowRadius * 2, glowRadius * 2);

            // Рисуем основную черную точку поверх свечения
            qreal pointRadius = 2.0;
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2);
        } else {
            // Рисуем обычную черную точку без свечения
            qreal pointRadius = 1.0;
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2);
        }
    }

}

// Отрисовка линий подсказок
void QTPainter::drawHints(QPainter &painter) {
    double _width = width() / 2.0;
    double _height = height() / 2.0;


    if (Section_ && (!points.empty() || !sections.empty() || !circles.empty())) {
        if (tab == 0) {
            closestPoint = findPoint();
            int X = static_cast<int>(scaling.scaleCoordinateX(closestPoint.x()) + _width);
            int Y = static_cast<int>(scaling.scaleCoordinateY(-closestPoint.y()) + _height);

            double mouseX = scaling.logicX(cursorX - _width);
            double mouseY = scaling.logicY(_height - cursorY);

            painter.setPen(QPen(QColor(169, 169, 169, 128), 2)); // Тускло-серая линия
            painter.drawLine(X, Y, cursorX, cursorY);
            drawCoordinateLine(painter, closestPoint.x(), closestPoint.y(), mouseX, mouseY);

        } else if (tab == 1) {
            Drawing = true;

            sectionStartPoint.setX(closestPoint.x());
            sectionStartPoint.setY(closestPoint.y());


            if (points.size() == 2 && sections.size() == 1) {
                closestPointNext.setX(cursorX);
                closestPointNext.setY(cursorY);
            } else {
                closestPointNext = findPoint();
                int BegX = static_cast<int>(scaling.scaleCoordinateX(sectionStartPoint.x()) + _width );
                int BegY = static_cast<int>(scaling.scaleCoordinateY(-sectionStartPoint.y()) + _height );
                int EndX = static_cast<int>(scaling.scaleCoordinateX(closestPointNext.x()) + _width );
                int EndY = static_cast<int>(scaling.scaleCoordinateY(-closestPointNext.y()) + _height );
                drawCoordinateLine(painter, sectionStartPoint.x(), sectionStartPoint.y(), closestPointNext.x(),
                                   closestPointNext.y());
                painter.setPen(QPen(QColor(169, 169, 169, 128), 2)); // Тускло-серая линия
                painter.drawLine(BegX, BegY, EndX, EndY);
            }

        } else if (tab == 2) {
            tab = 0;
            Drawing = false;


            emit SigSection(closestPoint.x(), closestPoint.y(), closestPointNext.x(), closestPointNext.y());
        }

    } else {
        closestPoint.setX(0);
        closestPoint.setY(0);
        closestPointNext.setX(0);
        closestPointNext.setY(0);
    }
}

// Отрисовка координат
void QTPainter::drawCoordinateLine(QPainter &painter, double x0, double y0, double x1, double y1) {
    double _width = width() / 2.0;
    double _height = height() / 2.0;
    double BegX = scaling.scaleCoordinateX(x0) + _width;
    double BegY = scaling.scaleCoordinateY(-y0) + _height;
    double EndX = scaling.scaleCoordinateX(x1) + _width;
    double EndY = scaling.scaleCoordinateY(-y1) + _height;
    qreal dx = x1 - x0;
    qreal dy = y0 - y1;
    qreal length = qSqrt(dx * dx + dy * dy);

    // Вычисляем середину линии
    qreal midX = (BegX + EndX) / 2;
    qreal midY = (BegY + EndY) / 2;

    QString lengthText = QString::number(length, 'f', 2);

    // Вычисляем угол наклона линии в градусах
    qreal angle = qAtan2(dy, dx) * 180.0 / M_PI;

    // Определяем смещение текста вверх от линии
    qreal textOffset = 15.0;

    // Вычисляем перпендикулярный вектор
    qreal perpAngle = angle + 90.0;
    qreal rad = qDegreesToRadians(perpAngle);
    qreal offsetX = textOffset * qCos(rad);
    qreal offsetY = textOffset * qSin(rad);

    // Сохраняем состояние painter
    painter.save();

    // Переносим систему координат в середину линии
    painter.translate(midX, midY);

    // Поворачиваем на угол наклона линии
    painter.rotate(angle);

    // Смещаем текст по перпендикуляру
    painter.translate(offsetX, offsetY);

    // Центрируем текст по горизонтали
    QRectF textRect = painter.fontMetrics().boundingRect(lengthText);
    painter.drawText(-textRect.width() / 2, textRect.height() / 2, lengthText);

    painter.restore();
}

// Поиск ближайших точек к курсору
QPointF QTPainter::findPoint() {
    QPointF closest;
    float minDistance = std::numeric_limits<float>::max(); // Инициализация максимальным значением
    double _width = width() / 2.0;
    double _height = height() / 2.0;
    for (const auto &pt: points) {

        int X = static_cast<int>(scaling.scaleCoordinateX(pt.x) + _width);
        int Y = static_cast<int>(scaling.scaleCoordinateY(-pt.y) + _height);

        float distance = std::sqrt(std::pow(X - cursorX, 2) + std::pow(Y - cursorY, 2));

        if (distance < minDistance) {
            minDistance = distance;

            closest.setX(pt.x);
            closest.setY(pt.y);
        }
    }

    return closest;
}

// Отрисовка фигур мышью
void QTPainter::drawMouse(QPainter &painter) {
    double _width = width() / 2.0;
    double _height = height() / 2.0;
    int logicalX = static_cast<int>(scaling.logicX(cursorX - _width));
    int logicalY = static_cast<int>(scaling.logicY(_height - cursorY));

    if (Circle_ && Drawing) {
        int logicalPerimeterX = perimeterPoint.x();
        int logicalPerimeterY = perimeterPoint.y();
        int centerX = (logicalPerimeterX + logicalX) / 2;
        int centerY = (logicalPerimeterY + logicalY) / 2;

        int radius = static_cast<int>(std::hypot(logicalPerimeterX - centerX, logicalPerimeterY - centerY));
        int screenCenterX = static_cast<int>(scaling.scaleCoordinateX(centerX) + _width);
        int screenCenterY = static_cast<int>(scaling.scaleCoordinateY(-centerY) + _height);
        int scaledRadius = static_cast<int>(scaling.scaleCoordinate(radius));
        painter.drawEllipse(screenCenterX - scaledRadius, screenCenterY - scaledRadius, 2 * scaledRadius,
                            2 * scaledRadius);
    } else if (Section_ && Drawing) {
        int screenStartX = static_cast<int>(scaling.scaleCoordinateX(sectionStartPoint.x()) + _width);
        int screenStartY = static_cast<int>(scaling.scaleCoordinateY(-sectionStartPoint.y()) + _height);

        if (Shift) {
            int dX = cursorX - currentCursorX;
            int dY = cursorY - currentCursorY;
            if (std::abs(dX) > std::abs(dY)) {
                drawCoordinateLine(painter, sectionStartPoint.x(), sectionStartPoint.y(), logicalX,
                                   sectionStartPoint.y());
                painter.drawLine(screenStartX, screenStartY, cursorX, screenStartY);
            } else {
                drawCoordinateLine(painter, sectionStartPoint.x(), sectionStartPoint.y(), sectionStartPoint.x(),
                                   logicalY);
                painter.drawLine(screenStartX, screenStartY, screenStartX, cursorY);
            }
        } else {


            drawCoordinateLine(painter, sectionStartPoint.x(), sectionStartPoint.y(), logicalX, logicalY);
            painter.drawLine(screenStartX, screenStartY, cursorX, cursorY);
        }
    }
}

// Отрисовка
void QTPainter::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::lightGray);

    // Масштабируем
    //   Scaling.scaling(width(), height(), FindMaxMin());



    double _width = width() / 2.0;
    double _height = height() / 2.0;

    drawBackground(painter); // Фон
    drawPoints(painter); // Точки
  //  drawExp(painter);
    drawCircles(painter); // Круги
    drawSections(painter); // Отрезки
    drawHints(painter); // Отрисовка линий для подсказок
    drawMouse(painter); // Отрисовка мышью

    points.clear();
    circles.clear();
    sections.clear();

}

void QTPainter::changeSize(const rectangle &allObjects) {}

unsigned long long QTPainter::getWeight() {
    return width();
}

unsigned long long QTPainter::getHeight() {
    return height();
}

void QTPainter::mousePressEvent(QMouseEvent *event) {

    if (event->button() == Qt::RightButton) {
        scaling.startMousePress(event->pos());
        emit RightPress();

        if ((Circle_ || Section_ || Point_) && Drawing) {
            scaling.resetUsersResize();
            Drawing = false;
        }
        update();
    }

    if (event->button() == Qt::LeftButton) {
        leftClick = true;
        ReleaseLeftClick = false;
    }


    if ((Point_ || Section_ || Circle_) && event->button() == Qt::LeftButton) {

        QPoint Position = event->pos();
        double _width = width() / 2.0;
        double _height = height() / 2.0;
        int logicalX = static_cast<int>(scaling.logicX(Position.x() - _width));
        int logicalY = static_cast<int>(scaling.logicY(_height - Position.y()));

        if (Point_) {
            Position.setX(logicalX);
            Position.setY(logicalY);
            emit SigPoint(Position);
        }

        if (Section_) {
            if (!Drawing) {
                Drawing = true;
                sectionStartPoint.setX(logicalX);
                sectionStartPoint.setY(logicalY);
            } else {
                QPoint EndPoint(logicalX, logicalY);
                if (Shift) {
                    int dX = cursorX - currentCursorX;
                    int dY = cursorY - currentCursorY;
                    if (std::abs(dX) > std::abs(dY)) {
                        emit SigSection(sectionStartPoint.x(), sectionStartPoint.y(), EndPoint.x(),
                                        sectionStartPoint.y());
                    } else {
                        emit SigSection(sectionStartPoint.x(), sectionStartPoint.y(), sectionStartPoint.x(),
                                        EndPoint.y());
                    }
                } else {
                    emit SigSection(sectionStartPoint.x(), sectionStartPoint.y(), EndPoint.x(), EndPoint.y());
                }
                Drawing = false;
            }
        }

        if (Circle_) {
            if (!Drawing) {
                Drawing = true;
                perimeterPoint.setX(logicalX);
                perimeterPoint.setY(logicalY);
            } else {
                QPoint cursorPoint(logicalX, logicalY);
                int centerX = (perimeterPoint.x() + cursorPoint.x()) / 2;
                int centerY = (perimeterPoint.y() + cursorPoint.y()) / 2;
                int radius = static_cast<int>(std::hypot(perimeterPoint.x() - centerX, perimeterPoint.y() - centerY));
                QPoint centerPoint(centerX, centerY);
                emit SigCircle(centerPoint, radius);
                Drawing = false;
            }
        }
    }

    tab = 0;
}

void QTPainter::mouseMoveEvent(QMouseEvent *event) {
    if (!Shift) {
        currentCursorX = cursorX;
        currentCursorY = cursorY;
    }
    scaling.mouseMove(event->pos());


    cursorX = event->pos().x();
    cursorY = event->pos().y();


    if (scaling.isRightMousePressed()) {
        emit RightPress();
    }

    if (!ReleaseLeftClick && leftClick) {
        if (Moving && id != 0) {
            emit MovingFigures();
        }
    } else {
        id = 0;
        leftClick = false;
    }


    leftDoubleClick = false;
    update();
}

void QTPainter::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        scaling.endMousePress();
    }
    if (event->button() == Qt::LeftButton) {
        leftDoubleClick = false;
        ReleaseLeftClick = true;
    }
}

void QTPainter::onWorkWindowResized() {
    // При изменении размера окна родителя меняем размер
    resize(ui->workWindow->size());
}

void QTPainter::mouseDoubleClickEvent(QMouseEvent *event) {

    if (event->button() == Qt::LeftButton) {
        leftDoubleClick = true;
    }

    QWidget::mouseDoubleClickEvent(event);
}

void QTPainter::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        Shift = true;
        currentCursorX = cursorX;
        currentCursorY = cursorY;
    }

    if (event->key() == Qt::Key_Tab) {

        if (tab == 2) {
            tab = 0;
        } else {
            ++tab;
        }
        event->accept();
    }
}

void QTPainter::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        Shift = false;
    }
}

void QTPainter::setMode(DrawMode mode) {
    setEditor(mode == DrawMode::Editor);
    setPoint(mode == DrawMode::Point);
    setSection(mode == DrawMode::Section);
    setCircle(mode == DrawMode::Circle);
    bool movingMode = (mode == DrawMode::Move || mode == DrawMode::Resize || mode == DrawMode::Rotate);
    setMoving(movingMode);
}