#include "QTPainter.h"

QTPainter::QTPainter(Ui::MainWindow *ui, QWidget *parent)
        : QFrame(parent), ui(ui), scaling(ui->workWindow->width(), ui->workWindow->height()),
          minCellSize(10), maxCellSize(20), CellSize(8), CellView(true), currentCellSize(1), cursorX(0), cursorY(0),
          editor(false), Circle(false), Section(false), Point(false), Drawing(false),leftClick(false),leftDoubleClick(false),Shift(false)
          ,tab(0){

    setMouseTracking(true);

    resize(parent->size());
    setStyleSheet("background: \"#ffffff\"");

    // Отслеживаем размер окна родителя
    connect(ui->workWindow->window(), SIGNAL(resized()), this, SLOT(onWorkWindowResized()));
    setAttribute(Qt::WA_AcceptTouchEvents);
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

void QTPainter::drawPoint(point pt, bool isWhite) {
    points.push_back(pt);
}

void QTPainter::drawCircle(circle c, bool isWhite) {
    circles.push_back(c);
}

void QTPainter::drawSection(section sec, bool isWhite) {
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

bool QTPainter::moving(int x, int y) {
    if (cursorX >= x - 1 && cursorX <= x + 1 && cursorY >= y - 1 && cursorY <= y + 1) {
        emit Move(ET_POINT,x, y);
        return true;
    }
return false;
}

bool QTPainter::moving(int x0, int y0, int r) {
    int x = 0;
    int y = r;
    int p = 1 - r;

    while (x <= y) {
        // Проверка с учетом погрешности ±1 пиксель
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if ((x0 + x + dx == cursorX && y0 + y + dy == cursorY) ||
                    (x0 - x + dx == cursorX && y0 + y + dy == cursorY) ||
                    (x0 + x + dx == cursorX && y0 - y + dy == cursorY) ||
                    (x0 - x + dx == cursorX && y0 - y + dy == cursorY) ||
                    (x0 + y + dx == cursorX && y0 + x + dy == cursorY) ||
                    (x0 - y + dx == cursorX && y0 + x + dy == cursorY) ||
                    (x0 + y + dx == cursorX && y0 - x + dy == cursorY) ||
                    (x0 - y + dx == cursorX && y0 - x + dy == cursorY)) {
                    emit Move(ET_CIRCLE, x0, y0, r);
                    return true;
                }
            }
        }

        x++;

        if (p < 0) {
            p = p + 2 * x + 1;
        } else {
            y--;
            p = p + 2 * x - 2 * y + 1;
        }
    }

    return false;
}

bool QTPainter::moving(int x0, int y0, int x1, int y1) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    int stepX = (dx > 0) ? 1 : -1;
    int stepY = (dy > 0) ? 1 : -1;

    dx = abs(dx);
    dy = abs(dy);

    if (dx > dy) {
        int p = 2 * dy - dx;
        for (int x = x0, y = y0; x != x1; x += stepX) {
            // Проверка с учетом погрешности ±1 пиксель
            for (int dyOffset = -1; dyOffset <= 1; ++dyOffset) {
                if (x == cursorX && (y + dyOffset) == cursorY) {
                    emit Move(ET_SECTION, x0, y0, x1, y1);
                    return true;
                }
            }
            if (p > 0) {
                y += stepY;
                p -= 2 * dx;
            }
            p += 2 * dy;
        }
    } else {
        int p = 2 * dx - dy;
        for (int y = y0, x = x0; y != y1; y += stepY) {
            // Проверка с учетом погрешности ±1 пиксель
            for (int dxOffset = -1; dxOffset <= 1; ++dxOffset) {
                if (y == cursorY && (x + dxOffset) == cursorX) {
                    emit Move(ET_SECTION, x0, y0, x1, y1);
                    return true;
                }
            }
            if (p > 0) {
                x += stepX;
                p -= 2 * dy;
            }
            p += 2 * dx;
        }
    }
    return false;
}

QPoint QTPainter::findPoint() {
    QPoint closest;
    float minDistance = std::numeric_limits<float>::max(); // Инициализация максимальным значением
    double _width = width() / 2.0;
    double _height = height() / 2.0;
    for (const auto &pt : points) {
        int X = static_cast<int>(scaling.scaleCoordinateX(pt.x) + _width);
        int Y = static_cast<int>(scaling.scaleCoordinateY(-pt.y) + _height);

        // Вычисление расстояния до курсора
        float distance = std::sqrt(std::pow(X - cursorX, 2) + std::pow(Y - cursorY, 2));

        // Проверка, является ли текущая точка ближайшей
        if (distance < minDistance) {
            minDistance = distance;
            closest.setX(X);
            closest.setY(Y);
        }
    }

    return closest;
}

void QTPainter::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::lightGray);

    // Масштабируем
    //   Scaling.scaling(width(), height(), FindMaxMin());


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
        // Изменяем размер клетки
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
        for (int x = _width - currentCellSize, x1 = _width + currentCellSize;
             x + abs(deltaX) >= 0; x -= currentCellSize, x1 += currentCellSize) {
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


    // Отрисовка координат
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

   /* if(Section) {
        if(editor ){
        QPoint closestPoint = findPoint();
            if(tab==0){
             painter.setPen(QPen(QColor(169, 169, 169, 128), 2)); // Тускло-серая линия
             // Рисуем линию от ближайшей точки до курсора
             painter.drawLine(closestPoint, QPoint(cursorX, cursorY));
             }
            /*else if(tab==1){
              painter.setPen(QPen(QColor(169, 169, 169, 128), 2)); // Тускло-серая линия
                QPoint closestPointNext = findPoint();

             // Рисуем линию от ближайшей точки до курсора
          if(!Drawing)    painter.drawLine(closestPoint, closestPointNext);
          else painter.drawLine(sectionStartPoint.x(),sectionStartPoint.y(), closestPointNext);
             }
             else if(tab==2){
              emit SigSection(closestPoint.x(), closestPoint.y(), closestPointNext.x(), closestPointNext.y());
             }

         }
    }*/

    // Отрисовка фигур
    for (const auto &pt: points) {
        int X = static_cast<int>(scaling.scaleCoordinateX(pt.x) + _width );
        int Y = static_cast<int>(scaling.scaleCoordinateY(-pt.y) + _height );


        if (leftClick && moving(X, Y)) {
            qreal glowRadius = 5.0;

            // Создаем радиальный градиент для свечения
            QRadialGradient radialGradient(X, Y, glowRadius, X, Y);
            radialGradient.setColorAt(0.0, QColor(0, 255, 255, 150)); // Циановый цвет в центре (полупрозрачный)
            radialGradient.setColorAt(1.0, QColor(0, 255, 255, 0));   // Прозрачный цвет на краю градиента

            // Настраиваем кисть с градиентом
            QBrush glowBrush(radialGradient);
            painter.setBrush(glowBrush);
            painter.setPen(Qt::NoPen); // Без обводки для свечения
            painter.drawEllipse(X - glowRadius, Y - glowRadius, glowRadius * 2, glowRadius * 2); // Рисуем свечение

            // Рисуем основную черную точку поверх свечения
            qreal pointRadius = 2.0; // Радиус точки
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen); // Без обводки для точки
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2); // Рисуем точку
        }
        else {
            // Рисуем обычную черную точку без свечения
            qreal pointRadius = 1.0; // Радиус точки
            QBrush pointBrush(Qt::black);
            painter.setBrush(pointBrush);
            painter.setPen(Qt::NoPen); // Без обводки для точки
            painter.drawEllipse(X - pointRadius, Y - pointRadius, pointRadius * 2, pointRadius * 2); // Рисуем точку
        }
    }


    for (const auto &c: circles) {
        int X = static_cast<int>(scaling.scaleCoordinateX(c.center->x) + _width );
        int Y = static_cast<int>(scaling.scaleCoordinateY(-c.center->y) + _height );
        int Radius = 2 * static_cast<int>(scaling.scaleCoordinate(c.R));

        if (leftClick && moving(X, Y, Radius/ 2)) {
            // Параметры свечения
            qreal glowDistance = 20.0; // Расстояние свечения от обводки
            qreal gradientRadius = (Radius / 2.0) + glowDistance; // Общий радиус градиента

            // Вычисляем относительные позиции для цветовых остановок
            qreal stopTransparent1 = (Radius / 2.0 - glowDistance) / gradientRadius;
            qreal stopCyan = (Radius / 2.0) / gradientRadius;
            qreal stopTransparent2 = (Radius / 2.0 + glowDistance) / gradientRadius;

            // Создаем радиальный градиент
            QRadialGradient radialGradient(X, Y, gradientRadius, X, Y);
            radialGradient.setColorAt(0.0, QColor(0, 255, 255, 0));         // Прозрачный в центре
            radialGradient.setColorAt(stopTransparent1, QColor(0, 255, 255, 0));   // Прозрачный до внутренней границы свечения
            radialGradient.setColorAt(stopCyan, QColor(0, 255, 255, 150));   // Насыщенный циан на обводке
            radialGradient.setColorAt(stopTransparent2, QColor(0, 255, 255, 0));   // Прозрачный после внешней границы свечения
            radialGradient.setColorAt(1.0, QColor(0, 255, 255, 0));         // Прозрачный за пределами свечения

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

    for (const auto &sec: sections) {
        int BegX = static_cast<int>(scaling.scaleCoordinateX(sec.beg->x) + _width );
        int BegY = static_cast<int>(scaling.scaleCoordinateY(-sec.beg->y) + _height );
        int EndX = static_cast<int>(scaling.scaleCoordinateX(sec.end->x) + _width );
        int EndY = static_cast<int>(scaling.scaleCoordinateY(-sec.end->y) + _height );


        if (leftClick && moving(BegX, BegY, EndX, EndY)) {
            // Вычисляем направление линии
            qreal dx = EndX - BegX;
            qreal dy = EndY - BegY;
            qreal length = qSqrt(dx * dx + dy * dy);

            if (length == 0) length = 1; // Избегаем деления на ноль

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
        }
        else {
            QPen blackPen(Qt::black);
            blackPen.setWidth(1);
            blackPen.setCapStyle(Qt::RoundCap); // Устанавливаем закругленные концы
            painter.setPen(blackPen);
            painter.drawLine(BegX, BegY, EndX, EndY);
        }

    }

    if (editor) {
        painter.setPen(Qt::black);
        int logicalX = static_cast<int>(scaling.logicX(cursorX - _width));
        int logicalY = static_cast<int>(scaling.logicY(_height - cursorY));
        painter.
                drawText(cursorX+ 10, cursorY - 10, QString("X: %1, Y: %2").arg(logicalX).arg(logicalY)
        );

        if (Circle && Drawing) {


            int logicalPerimeterX = perimeterPoint.x();
            int logicalPerimeterY = perimeterPoint.y();
            int centerX = (logicalPerimeterX + logicalX) / 2;
            int centerY = (logicalPerimeterY + logicalY) / 2;

            int radius = static_cast<int>(std::hypot(logicalPerimeterX - centerX, logicalPerimeterY - centerY));
            int screenCenterX = static_cast<int>(scaling.scaleCoordinateX(centerX) + _width);
            int screenCenterY = static_cast<int>(scaling.scaleCoordinateY(-centerY) + _height);
            int scaledRadius = static_cast<int>(scaling.scaleCoordinate(radius));
            painter.
                    drawEllipse(screenCenterX
                                - scaledRadius, screenCenterY - scaledRadius, 2 * scaledRadius,
                                2 * scaledRadius);
        } else if (
                Section && Drawing
                ) {
            int screenStartX = static_cast<int>(scaling.scaleCoordinateX(sectionStartPoint.x()) + _width);
            int screenStartY = static_cast<int>(scaling.scaleCoordinateY(-sectionStartPoint.y()) + _height);
            if(Shift && abs(cursorX-currentCurcsorX)>abs(cursorY-currentCurcsorY)){
  painter.drawLine(screenStartX, screenStartY, screenStartX, cursorY);
            }else if(Shift){
                painter.drawLine(screenStartX, screenStartY, screenStartX, cursorY);
            }else {
                painter.drawLine(screenStartX, screenStartY, cursorX, cursorY);
            }
        }
    }
    points.

            clear();

    circles.

            clear();

    sections.

            clear();

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

        if ((Circle || Section || Point) && Drawing) {
            scaling.resetUsersResize();
            Drawing = false;
        }
        update();
    }

if(!editor && !leftClick &&  event->button() == Qt::LeftButton){
    leftClick=true;

}


    if (editor && event->button() == Qt::LeftButton) {

        QPoint Position = event->pos();
        double _width = width() / 2.0;
        double _height = height() / 2.0;
        int logicalX = static_cast<int>(scaling.logicX(Position.x() - _width));
        int logicalY = static_cast<int>(scaling.logicY(_height - Position.y()));

        if (Point) {
            Position.setX(logicalX);
            Position.setY(logicalY);
            emit SigPoint(Position);
        }

        if (Section) {
            if (!Drawing) {
                Drawing = true;
                sectionStartPoint.setX(logicalX);
                sectionStartPoint.setY(logicalY);
            } else {
                QPoint EndPoint(logicalX, logicalY);
                emit SigSection(sectionStartPoint.x(), sectionStartPoint.y(), EndPoint.x(), EndPoint.y());
                Drawing = false;
            }
        }

        if (Circle) {
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
}

void QTPainter::mouseMoveEvent(QMouseEvent *event) {
    scaling.mouseMove(event->pos());


        cursorX = event->pos().x();
        cursorY = event->pos().y();


    if (scaling.isRightMousePressed()) {
        emit RightPress();
    }

    leftClick=false;
    leftDoubleClick=false;
    update();
}

void QTPainter::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        scaling.endMousePress();
    }
   // if(event->button() == Qt::LeftButton){leftClick=false;}
}

void QTPainter::onWorkWindowResized() {
    // При изменении размера окна родителя меняем размер
    resize(ui->workWindow->size());
}

void QTPainter::mouseDoubleClickEvent(QMouseEvent *event) {

    if (event->button() == Qt::LeftButton) {
        leftDoubleClick=true;
    }


    QWidget::mouseDoubleClickEvent(event);
}

void QTPainter::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        Shift = true;
        currentCurcsorX=cursorX;
        currentCurcsorY=cursorY;
    }

    if (event->key() == Qt::Key_Tab) {
        if(tab<3){
          ++tab;
        }
        else{
            tab=0;
        }
    }
}

void QTPainter::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        Shift = false;
    }
}