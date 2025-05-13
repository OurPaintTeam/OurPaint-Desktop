#include "QTPainter.h"

QTPainter::QTPainter(QWidget *parent) : QFrame(parent), pointStorage(nullptr),
                                        sectionStorage(nullptr), circleStorage(nullptr), arcStorage(nullptr) {

    if (parent) {
        resize(parentWidget()->size()); // Берем размер от отца

        // Ловим сигнал изменения размера окна
        connect(parent->window(), SIGNAL(resize()), this, SLOT(onWorkWindowResized()));

        // Ловим сигналы от рисования мышкой
        connect(&drawFigM, &DrawMouse::SigPoint, this, &QTPainter::onSigPoint);
        connect(&drawFigM, &DrawMouse::SigSection, this, &QTPainter::onSigSection);
        connect(&drawFigM, &DrawMouse::SigCircle, this, &QTPainter::onSigCircle);
        connect(&drawFigM, &DrawMouse::SigArc, this, &QTPainter::onSigArc);
    }

    // Установим стили
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: \"#ffffff\"");
    setAttribute(Qt::WA_AcceptTouchEvents);

    // Базовые параметры
    Scaling::updateScaling();
    Scaling::setStartMonitorSize((short int) width(), (short int) height());
}

std::vector<ID> &QTPainter::getVecSelectedIDPoints() {
    return selectedIDPoint;
}

std::vector<ID> &QTPainter::getVecSelectedIDSections() {
    return selectedIDSection;
}

std::vector<ID> &QTPainter::getVecSelectedIDCircles() {
    return selectedIDCircle;
}

std::optional<std::pair<ID, ID>> QTPainter::getPairSelectedID() {
    std::vector<ID> IDs;

    for (const ID &id: selectedIDPoint) {
        IDs.push_back(id);
        if (IDs.size() > 2) { return std::nullopt; }
    }

    for (const ID &id: selectedIDSection) {
        IDs.push_back(id);
        if (IDs.size() > 2) { return std::nullopt; }
    }

    for (const ID &id: selectedIDCircle) {
        IDs.push_back(id);
        if (IDs.size() > 2) { return std::nullopt; }
    }

    if (IDs.size() == 2) {
        return std::make_pair(IDs[0], IDs[1]);
    }

    return std::nullopt;
}

void QTPainter::draw() {
    update();
}

void QTPainter::clear() {

    selectedClear();
    Scaling::setZoomZero();

    selectedRectangle.clear();
}

void QTPainter::selectedClear() {
    selectedIDPoint.clear();
    selectedIDSection.clear();
    selectedIDCircle.clear();
    selectedIDArc.clear();
}

void QTPainter::resizeRectangle() {}

bool QTPainter::findClosesObject() {
    bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    bool doubleClick = ModeManager::getActiveMode(MouseMode::DoubleClickLeft);
    bool shiftPress=ModeManager::getActiveMode(KeyMode::Shift);

    if (!leftClick && !doubleClick) {
        return false;
    }

    auto currentTime = std::chrono::steady_clock::now();
    if (currentTime - lastClickTime < std::chrono::milliseconds(300)) {
        return false;
    }

    lastClickTime = currentTime;

    if(!shiftPress){
        selectedClear();
    }

    for (const auto &pt : *pointStorage) {
        const Point *point = pt.second;
        if (ClosesPoint::checkFigure(point->x, point->y)) {
            bool found = false;
            std::vector<ID> updated;

            for (const auto &id : selectedIDPoint) {
                if (id == pt.first) {
                    found = true; // не добавляем
                } else {
                    updated.push_back(id);
                }
            }

            if (!found) {
                updated.push_back(pt.first); // добавляем
            }

            selectedIDPoint = std::move(updated); // заменяем старый вектор
            return true;
        }
    }

    for (const auto &sec : *sectionStorage) {
        const Section *section = sec.second;
        if (ClosesPoint::checkFigure(section->beg->x, section->beg->y,
                                     section->end->x, section->end->y)) {
            bool found = false;
            std::vector<ID> updated;

            for (const auto &id : selectedIDSection) {
                if (id == sec.first) {
                    found = true; // не добавляем
                } else {
                    updated.push_back(id);
                }
            }

            if (!found) {
                updated.push_back(sec.first); // добавляем
            }

            selectedIDSection = std::move(updated); // заменяем старый вектор
            return true;
        }
    }

    for (const auto &cir : *circleStorage) {
        const Circle *circle = cir.second;
        if (ClosesPoint::checkFigure(circle->center->x, circle->center->y, circle->r)) {
            bool found = false;
            std::vector<ID> updated;

            for (const auto &id : selectedIDCircle) {
                if (id == cir.first) {
                    found = true; // не добавляем
                } else {
                    updated.push_back(id);
                }
            }

            if (!found) {
                updated.push_back(cir.first); // добавляем
            }

            selectedIDCircle = std::move(updated); // заменяем старый вектор
            return true;
        }
    }

    for (const auto &arcs : *arcStorage) {
        const Arc *arc = arcs.second;
        if (ClosesPoint::checkFigure(arc->beg->x,arc->beg->y,arc->end->x,arc->end->y,arc->center->x,arc->center->y)) {
            bool found = false;
            std::vector<ID> updated;

            for (const auto &id : selectedIDArc) {
                if (id == arcs.first) {
                    found = true; // не добавляем
                } else {
                    updated.push_back(id);
                }
            }

            if (!found) {
                updated.push_back(arcs.first); // добавляем
            }

            selectedIDArc = std::move(updated); // заменяем старый вектор
            return true;
        }
    }

    return false;
}

void QTPainter::drawingFigures(QPainter &painter) {

    // Обработка точек
    if (pointStorage != nullptr && !pointStorage->empty()) {
        DrawFigures::drawPoint(painter, *pointStorage, selectedIDPoint);
    }

    // Обработка отрезков
    if (sectionStorage != nullptr && !sectionStorage->empty()) {
        DrawFigures::drawSection(painter, *sectionStorage, selectedIDSection);
    }

    // Обработка кругов
    if (circleStorage != nullptr && !circleStorage->empty()) {
        DrawFigures::drawCircle(painter, *circleStorage, selectedIDCircle);
    }

    // Обработка дуг
    if (arcStorage != nullptr && !arcStorage->empty()) {
        DrawFigures::drawArc(painter, *arcStorage, selectedIDArc);
    }


}

void QTPainter::saveToImage(const QString &fileName, QString &format) {

    resizeRectangle();

    if (format.startsWith('.'))
        format = format.mid(1);

    QString originalFormat = format.toLower();
    QString chosenFormat = originalFormat;

    if (originalFormat == "jpg" || originalFormat == "jpeg") {
        chosenFormat = "JPEG";
    } else if (originalFormat == "png") {
        chosenFormat = "PNG";
    } else if (originalFormat == "bmp") {
        chosenFormat = "BMP";
    } else if (originalFormat == "tiff" || originalFormat == "tif") {
        chosenFormat = "TIFF";
    } else if (originalFormat == "pdf") {
        chosenFormat = "PDF";
    } else if (originalFormat == "svg") {
        chosenFormat = "SVG";
    } else {
        QMessageBox::warning(this, tr("Ошибка"), tr("Неподдерживаемый формат: %1").arg(format));
        return;
    }

    QString filePath = fileName;

    if (!filePath.endsWith("." + originalFormat, Qt::CaseInsensitive)) {
        filePath += "." + originalFormat;
    }

    if (chosenFormat == "SVG") {
        QSvgGenerator generator;
        generator.setFileName(filePath);
        generator.setSize(this->size());
        generator.setViewBox(QRect(0, 0, this->width(), this->height()));
        generator.setTitle("Exported SVG");
        generator.setDescription("Generated by QTPainter");

        QPainter painter(&generator);

        this->render(&painter);

        painter.end();
    } else {
        QPixmap pixmap = grab();

        if (chosenFormat == "PDF") {
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                QPdfWriter writer(&file);
                writer.setPageSize(QPageSize(QPageSize::A4));
                writer.setResolution(300);

                QPainter pdfPainter(&writer);

                QPixmap pixmap = this->grab(this->rect());
                if (!pixmap.isNull()) {
                    QSize targetSize(writer.width(), writer.height());
                    QPixmap scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    pdfPainter.drawPixmap(0, 0, scaledPixmap);
                } else {
                    qDebug() << "Pixmap is null!";
                }

                pdfPainter.end();
                file.close();
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось открыть PDF-файл для записи."));
            }
        } else {
            if (!pixmap.save(filePath, chosenFormat.toUtf8().constData())) {
                QMessageBox::warning(this, tr("Ошибка"),
                                     tr("Не удалось сохранить изображение в формате %1").arg(chosenFormat));
            }
        }
    }
}

void QTPainter::selectedElemByID() {}

void QTPainter::onSigPoint(double x, double y) {
    emit SigPoint(x, y);
}

void QTPainter::onSigCircle(double x, double y, double r) {
    emit SigCircle(x, y, r);
}

void QTPainter::onSigSection(double x, double y, double x1, double y1) {
    emit SigSection(x, y, x1, y1);
}

void QTPainter::onSigArc(double x, double y, double x1, double y1, double xc, double yc) {
    emit SigArc(x, y, x1, y1,xc,yc);
}

void QTPainter::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    Scaling::setActualMonitorSize(width(), height());
    update();
}

void QTPainter::paintEvent(QPaintEvent *event) {

    QPainter painter(this);

    // Если не активен то не рисуем
    if (!painter.isActive()) {
        throw std::runtime_error("Painter not active!");
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    // Переносим на центр экрана
    painter.translate((int) (width() / 2 + Scaling::getDeltaX()), (int) (height() / 2 + Scaling::getDeltaY()));

    // Отрисовка фона
    DrawBackground::drawFon(painter);

    // Если курсор в области то рисуем его значения
    if (ModeManager::getCursor()) {
        DrawAdditionalInf::drawCursor(painter);
    }


    /*************** Отрисовка фигур мышкой ********************/

    if (ModeManager::getCursor()) {
        if (!(ModeManager::getActiveMode(MouseMode::MouseMove)) &&
            ModeManager::getActiveMode(WorkModes::Point)) {

            drawFigM.DrawFiguresMouse(painter);

        } else if (ModeManager::getActiveMode(WorkModes::Circle) ||
                (ModeManager::getActiveMode(WorkModes::Arc))) {
            drawFigM.DrawFiguresMouse(painter);

        } else if (ModeManager::getActiveMode(WorkModes::Section)) {

            drawFigM.DrawFiguresMouse(painter);

            if (pointStorage != nullptr && pointStorage->size() > 0) {
                QPointF closes = ClosesPoint::findClosestPoint(*pointStorage); // Ищем ближайшие точки
                drawFigM.drawHints(painter, closes);
            }
        }

    }



    /****************** Функции выделения и перемещения  ********************/


    // Выделение
    if (ModeManager::getActiveMode(WorkModes::Editor)) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if ((circleStorage != nullptr && circleStorage->size() > 0) ||
                (sectionStorage != nullptr && sectionStorage->size() > 0) ||
                (pointStorage != nullptr && pointStorage->size() > 0))

                if(!findClosesObject()){
                    selectedClear();
                }

        } else if (ModeManager::getActiveMode(MouseMode::DoubleClickLeft)) {
            if ((circleStorage != nullptr && circleStorage->size() > 0) ||
                (sectionStorage != nullptr && sectionStorage->size() > 0) ||
                (pointStorage != nullptr && pointStorage->size() > 0)){}
                //findClosesObject();
        }
    } else if (ModeManager::getActiveMode(WorkModes::Selected)) {
        QRectF rect = selectedRectangle.selected(painter);
        ClosesPoint::enteringInRect(*pointStorage, rect, selectedIDPoint);
        ClosesPoint::enteringInRect(*sectionStorage, rect, selectedIDSection);
        ClosesPoint::enteringInRect(*circleStorage, rect, selectedIDCircle);
    } else if (ModeManager::getActiveMode(WorkModes::Move)) {

        if (ModeManager::getActiveMode(MouseMode::RightClick)) {
            drawing = false;
            selectedClear();
        }

        //  Кнопка мыши зажата
        if (!drawing) {
            if (ModeManager::getActiveMode(MouseMode::LeftClick) && findClosesObject()) {
                drawing = true;
                QPointF cursorPressPos = QPointF(Scaling::logicCursorX(), Scaling::logicCursorY());

              if(!selectedIDSection.empty() && sectionStorage->contains(selectedIDSection[0])){
                  Section* s = (*sectionStorage)[selectedIDSection[0]];
                  LineVecBeg = QPointF(s->beg->x,s->beg->y)  - cursorPressPos;
                  LineVecEnd = QPointF(s->end->x,s->end->y)  - cursorPressPos;
              }
              if(!selectedIDCircle.empty() && circleStorage->contains(selectedIDCircle[0])){
                  Circle* c = (*circleStorage)[selectedIDCircle[0]];
                  QPointF center(c->center->x, c->center->y);
                  VecCircle = center - cursorPressPos;
              }
            } else {
                selectedClear();
            }
        } else {
            if (!ModeManager::getActiveMode(MouseMode::ReleasingLeft)) {
                if (!selectedIDSection.empty()) {
                    emit MovingSection(selectedIDSection, LineVecBeg, LineVecEnd);
                }
                if (!selectedIDPoint.empty()) {
                    emit MovingPoint(selectedIDPoint);
                }
                if (!selectedIDCircle.empty()) {
                    emit MovingCircle(selectedIDCircle,VecCircle);
                }
                if (!selectedIDArc.empty()) {
                    emit MovingArc(selectedIDArc);
                }
            } else {
                emit EndMoving();
                drawing = false;
            }
        }
    }


    // Вызов функции рисования
    drawingFigures(painter);

    QFrame::paintEvent(event);
}

unsigned long long QTPainter::getWeight() {
    return Scaling::getActualMonitorWidth();
}

unsigned long long QTPainter::getHeight() {
    return Scaling::getActualMonitorHeight();
}

void QTPainter::getBoundBox(const BoundBox2D &allObjects) {
    Rectangle = &allObjects;
}

void QTPainter::initPoint(std::unordered_map<ID, Point *> &points) {
    pointStorage = &points;
}

void QTPainter::initSection(std::unordered_map<ID, Section *> &sections) {
    sectionStorage = &sections;
}

void QTPainter::initCircle(std::unordered_map<ID, Circle *> &circles) {
    circleStorage = &circles;
}

void QTPainter::initArc(std::unordered_map<ID, Arc *> &arcs) {
    arcStorage = &arcs;
}

void QTPainter::onWorkWindowResized() {
    // При изменении размера окна родителя меняем размер
    Scaling::setActualMonitorSize(parentWidget()->width(), parentWidget()->height());
    resize(parentWidget()->size());
}








