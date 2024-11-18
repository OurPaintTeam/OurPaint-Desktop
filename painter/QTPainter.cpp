#include "QTPainter.h"

QTPainter::QTPainter(Ui::MainWindow *ui, QWidget *parent)
        : QFrame(parent), ui(ui), Scaling(ui->workWindow->width(), ui->workWindow->height()),
          minCellSize(10), maxCellSize(20), CellSize(8), CellView(true), currentCellSize(1), cursorX(0), cursorY(0),
          editor(false), Circle(false), Section(false), Point(false),Drawing(false) {

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
    Scaling.setZoomZero();
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
    double max_zoom = (ui->workWindow->width()) / 16.0 / Scaling.getScale();
    Scaling.setZoomPlus(max_zoom);
    update();
}

void QTPainter::setZoomMinus() {
    Scaling.setZoomMinus();
    update();
}

void QTPainter::setZoomZero() {
    Scaling.setZoomZero();
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

void QTPainter::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::lightGray);

    // Масштабируем
    Scaling.scaling(width(), height(), FindMaxMin());

    double scale = Scaling.getScale();
    double zoom = Scaling.getZoom();
    double _width = width() / 2.0;
    double _height = height() / 2.0;
    int deltaX = Scaling.getDeltaX();
    int deltaY = Scaling.getDeltaY();


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


    // Отрисовка фигур
    for (const auto &pt: points) {
        int X = static_cast<int>(Scaling.scaleCoordinateX(pt.x) + _width );
        int Y = static_cast<int>(Scaling.scaleCoordinateY(-pt.y) + _height );
        painter.drawPoint(X, Y);
    }

    for (const auto &c: circles) {
        int X = static_cast<int>(Scaling.scaleCoordinateX(c.center->x) + _width );
        int Y = static_cast<int>(Scaling.scaleCoordinateY(-c.center->y) + _height );
        int Radius = 2 * static_cast<int>(Scaling.scaleCoordinate(c.R));
        painter.drawEllipse(X - Radius / 2, Y - Radius / 2, Radius, Radius);
    }

    for (const auto &sec: sections) {
        int BegX = static_cast<int>(Scaling.scaleCoordinateX(sec.beg->x) + _width );
        int BegY = static_cast<int>(Scaling.scaleCoordinateY(-sec.beg->y) + _height );
        int EndX = static_cast<int>(Scaling.scaleCoordinateX(sec.end->x) + _width );
        int EndY = static_cast<int>(Scaling.scaleCoordinateY(-sec.end->y) + _height );
        painter.drawLine(BegX, BegY, EndX, EndY);
    }

    if (editor) {
        painter.setPen(Qt::black);
        int logicalX = static_cast<int>((cursorX - _width) / (scale * zoom));
        int logicalY = static_cast<int>((_height - cursorY) / (scale * zoom));
        painter.drawText(cursorX + 10, cursorY - 10, QString("X: %1, Y: %2").arg(logicalX).arg(logicalY));

        if (Circle && Drawing) {
            int logicalPerimeterX = perimeterPoint.x();
            int logicalPerimeterY = perimeterPoint.y();
            int logicalCursorX = static_cast<int>((cursorX - width() / 2) / (Scaling.getScale() * Scaling.getZoom()));
            int logicalCursorY = static_cast<int>((height() / 2 - cursorY) / (Scaling.getScale() * Scaling.getZoom()));
            int centerX = (logicalPerimeterX + logicalCursorX) / 2;
            int centerY = (logicalPerimeterY + logicalCursorY) / 2;
            int radius = static_cast<int>(std::hypot(logicalPerimeterX - centerX, logicalPerimeterY - centerY));
            int screenCenterX = static_cast<int>(Scaling.scaleCoordinateX(centerX) + width() / 2);
            int screenCenterY = static_cast<int>(Scaling.scaleCoordinateY(-centerY) + height() / 2);
            int scaledRadius = static_cast<int>(radius * Scaling.getScale() * Scaling.getZoom());
            painter.drawEllipse(screenCenterX - scaledRadius, screenCenterY - scaledRadius, 2 * scaledRadius, 2 * scaledRadius);
        }
        else if (Section && Drawing) {
            int screenStartX = static_cast<int>(Scaling.scaleCoordinateX(sectionStartPoint.x()) + _width);
            int screenStartY = static_cast<int>(Scaling.scaleCoordinateY(-sectionStartPoint.y()) + _height);
            painter.drawLine(screenStartX, screenStartY, cursorX, cursorY);
        }
    }
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
        Scaling.startMousePress(event->pos());
        emit RightPress();

        if (Drawing) {
            Drawing = false;
            update();
        }

    }

    if (editor && event->button() == Qt::LeftButton) {
        if (Point) {
            QPoint Position = event->pos();
            int logicalX = static_cast<int>((Position.x() - width() / 2) / Scaling.getScale());
            int logicalY = static_cast<int>((height() / 2 - Position.y()) / Scaling.getScale());
            Position.setX(logicalX);
            Position.setY(logicalY);
            emit SigPoint(Position);
        }

        if (Section) {
            if (!Drawing) {
                Drawing = true;
                QPoint Position = event->pos();
                int logicalX = static_cast<int>((Position.x() - width() / 2) / (Scaling.getScale() * Scaling.getZoom()));
                int logicalY = static_cast<int>((height() / 2 - Position.y()) / (Scaling.getScale() * Scaling.getZoom()));
                sectionStartPoint.setX(logicalX);
                sectionStartPoint.setY(logicalY);
            } else {
                QPoint Position = event->pos();
                int logicalX = static_cast<int>((Position.x() - width() / 2) / (Scaling.getScale() * Scaling.getZoom()));
                int logicalY = static_cast<int>((height() / 2 - Position.y()) / (Scaling.getScale() * Scaling.getZoom()));
                QPoint EndPoint(logicalX, logicalY);
                emit SigSection(sectionStartPoint.x(), sectionStartPoint.y(), EndPoint.x(), EndPoint.y());
                Drawing = false;
            }
        }

        if (Circle) {
            if (!Drawing) {
                Drawing = true;
                QPoint Position = event->pos();
                int logicalX = static_cast<int>((Position.x() - width() / 2) / (Scaling.getScale() * Scaling.getZoom()));
                int logicalY = static_cast<int>((height() / 2 - Position.y()) / (Scaling.getScale() * Scaling.getZoom()));
                perimeterPoint.setX(logicalX);
                perimeterPoint.setY(logicalY);
            } else {
                QPoint Position = event->pos();
                int logicalX = static_cast<int>((Position.x() - width() / 2) / (Scaling.getScale() * Scaling.getZoom()));
                int logicalY = static_cast<int>((height() / 2 - Position.y()) / (Scaling.getScale() * Scaling.getZoom()));
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
    Scaling.mouseMove(event->pos());

    if (editor ) {
        cursorX = event->pos().x();
        cursorY = event->pos().y();
        if (Drawing) {
            update();
        }
    }

    if (Scaling.isRightMousePressed()) {
        emit RightPress();
    }
    update();
}

void QTPainter::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        Scaling.endMousePress();
    }
}

void QTPainter::onWorkWindowResized() {
    // При изменении размера окна родителя меняем размер
    resize(ui->workWindow->size());
}