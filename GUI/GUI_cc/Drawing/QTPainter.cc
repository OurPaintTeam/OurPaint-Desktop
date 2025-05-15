#include "QTPainter.h"

QTPainter::QTPainter(QWidget *parent) : QFrame(parent), circleStorage(nullptr),
                                        sectionStorage(nullptr), pointStorage(nullptr),
                                        arcStorage(nullptr),drawing(false) {

    if (parent) {
        // We take the size from the father
        resize(parentWidget()->size());
        connect(parent->window(), SIGNAL(resize()), this, SLOT(onWorkWindowResized()));

        connect(&drawingWithMouse, &DrawMouse::SigPoint, this, &QTPainter::onSigPoint);
        connect(&drawingWithMouse, &DrawMouse::SigSection, this, &QTPainter::onSigSection);
        connect(&drawingWithMouse, &DrawMouse::SigCircle, this, &QTPainter::onSigCircle);
        connect(&drawingWithMouse, &DrawMouse::SigArc, this, &QTPainter::onSigArc);
    }

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: \"#ffffff\"");
    setAttribute(Qt::WA_AcceptTouchEvents);

    Scaling::updateScaling();
    Scaling::setStartMonitorSize((short int) width(), (short int) height());
}


std::vector<ID> QTPainter::getVecSelectedIDPoints() {
    std::vector<ID> vec_id;
    vec_id.reserve(selectedIDPoint.size());
    for (const auto& pair : selectedIDPoint) {
        vec_id.push_back(pair.first);
    }
    return vec_id;
}


std::vector<ID> QTPainter::getVecSelectedIDSections() {
    std::vector<ID> vec_id;
    vec_id.reserve(selectedIDSection.size());
    for (const auto& pair : selectedIDSection) {
        vec_id.push_back(pair.first);
    }
    return vec_id;
}


std::vector<ID> QTPainter::getVecSelectedIDCircles() {
    std::vector<ID> vec_id;
    vec_id.reserve(selectedIDCircle.size());
    for (const auto& pair : selectedIDCircle) {
        vec_id.push_back(pair.first);
    }
    return vec_id;
}


std::vector<ID> QTPainter::getVecSelectedIDArcs() {
    std::vector<ID> vec_id;
    vec_id.reserve(selectedIDArc.size());
    for (const auto& pair : selectedIDArc) {
        vec_id.push_back(pair.first);
    }
    return vec_id;
}


std::optional<std::pair<ID, ID>> QTPainter::getPairSelectedID() {
    std::vector<ID> IDs;

    // It will return only if two

    for (const auto& pair : selectedIDPoint) {
        IDs.push_back(pair.first);
        if (IDs.size() > 2) { return std::nullopt; }
    }

    for (const auto& pair : selectedIDSection) {
        IDs.push_back(pair.first);
        if (IDs.size() > 2) { return std::nullopt; }
    }

    for (const auto& pair : selectedIDCircle) {
        IDs.push_back(pair.first);
        if (IDs.size() > 2) { return std::nullopt; }
    }

    for (const auto& pair : selectedIDArc) {
        IDs.push_back(pair.first);
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
    drawingWithMouse.clear();
    selectedRectangle.clear();
}


void QTPainter::selectedClear() {
    selectedIDPoint.clear();
    selectedIDSection.clear();
    selectedIDCircle.clear();
    selectedIDArc.clear();
    selectedRectangle.clear();
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
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastClickTime);

    if (duration < std::chrono::milliseconds(300)) {
        return false;
    }

    lastClickTime = currentTime;

    if(!shiftPress){
        selectedClear();
    }

    for (const auto &pt : *pointStorage) {
        const Point *point = pt.second;
        if (ClosesPoint::checkFigure(point->x, point->y,Scaling::logicCursorX(),Scaling::logicCursorY(),Scaling::getZoom())) {
            bool found = false;
            std::unordered_map<ID,Color> updated;

            for (const auto &pair : selectedIDPoint) {
                if (pair.first == pt.first) {
                    found = true; // не добавляем
                } else {
                    updated.emplace(pair.first,pair.second);
                }
            }

            if (!found) {
                updated.emplace(pt.first,Color::Blue);
            }

            selectedIDPoint = std::move(updated);
            return true;
        }
    }

    for (const auto &sec : *sectionStorage) {
        const Section *section = sec.second;
        if (ClosesPoint::checkFigure(section->beg->x, section->beg->y,
                                     section->end->x, section->end->y,Scaling::logicCursorX(),Scaling::logicCursorY(),Scaling::getZoom())) {
            bool found = false;
            std::unordered_map<ID,Color> updated;

            for (const auto &pair : selectedIDSection) {
                if (pair.first == sec.first) {
                    found = true;
                } else {
                    updated.emplace(pair.first,pair.second);
                }
            }

            if (!found) {
                updated.emplace(sec.first,Color::Blue);
            }

            selectedIDSection = std::move(updated);
            return true;
        }
    }

    for (const auto &cir : *circleStorage) {
        const Circle *circle = cir.second;
        if (ClosesPoint::checkFigure(circle->center->x, circle->center->y, circle->r,Scaling::logicCursorX(),Scaling::logicCursorY(),Scaling::getZoom())) {
            bool found = false;
            std::unordered_map<ID,Color> updated;

            for (const auto &pair : selectedIDCircle) {
                if (pair.first == cir.first) {
                    found = true; // не добавляем
                } else {
                    updated.emplace(pair.first,pair.second);
                }
            }

            if (!found) {
                updated.emplace(cir.first,Color::Blue);
            }

            selectedIDCircle = std::move(updated);
            return true;
        }
    }

    for (const auto &arcs : *arcStorage) {
        const Arc *arc = arcs.second;
        if (ClosesPoint::checkFigure(arc->beg->x,arc->beg->y,arc->end->x,arc->end->y,arc->center->x,arc->center->y)) {
            bool found = false;
            std::unordered_map<ID,Color> updated;

            for (const auto &pair : selectedIDArc) {
                if (pair.first == arcs.first) {
                    found = true; // не добавляем
                } else {
                    updated.emplace(pair.first,pair.second);
                }
            }

            if (!found) {
                updated.emplace(arcs.first,Color::Blue);
            }

            selectedIDArc = std::move(updated);
            return true;
        }
    }

    selectedClear();
    return false;
}


void QTPainter::drawingFigures(QPainter &painter) {

    if (pointStorage != nullptr && !pointStorage->empty()) {
        DrawFigures::drawPoint(painter, *pointStorage, selectedIDPoint);
    }

    if (sectionStorage != nullptr && !sectionStorage->empty()) {
        DrawFigures::drawSection(painter, *sectionStorage, selectedIDSection);
    }

    if (circleStorage != nullptr && !circleStorage->empty()) {
        DrawFigures::drawCircle(painter, *circleStorage, selectedIDCircle);
    }

    if (arcStorage != nullptr && !arcStorage->empty()) {
        DrawFigures::drawArc(painter, *arcStorage, selectedIDArc);
    }
}

void QTPainter::saveToImage(const QString &fileName, QString &format) {

    resizeRectangle();

    if (format.startsWith('.'))
        format = format.mid(1);

    QString originalFormat = format.toLower();
    QString chosenFormat;

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
        QMessageBox::warning(this, tr("Error"), tr("Unsupported format: %1").arg(format));
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
        QPixmap pixmapCopy = grab();

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
                QMessageBox::warning(this, tr("Error"), tr("The PDF file could not be opened for writing."));
            }
        } else {
            if (!pixmapCopy.save(filePath, chosenFormat.toUtf8().constData())) {
                QMessageBox::warning(this, tr("Error"),
                                     tr("Couldn't save the image in the format %1").arg(chosenFormat));
            }
        }
    }
}


void QTPainter::selectedElemByID(ID id,const std::string &type) {
    if(type=="Point"){
        selectedIDPoint.emplace(id,Color::Purple);
    }else  if(type=="Circle"){
        selectedIDCircle.emplace(id,Color::Purple);
    }else  if(type=="Section"){
        selectedIDSection.emplace(id,Color::Purple);
    }else  if(type=="Arc"){
        selectedIDArc.emplace(id,Color::Purple);
    }
}


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


void QTPainter::pointInRect(QRectF &rect){
    std::vector<ID> vecPointID=ClosesPoint::enteringInRect(*pointStorage, rect);
    selectedIDPoint.clear();
    for (const ID &id : vecPointID) {
        selectedIDPoint[id] = Color::Blue;
    }
}

void QTPainter::sectionInRect(QRectF &rect){
    std::vector<ID> vecSectionID=ClosesPoint::enteringInRect(*sectionStorage, rect );
    selectedIDSection.clear();
    for (const ID &id : vecSectionID) {
        selectedIDSection[id] = Color::Blue;
    }
}
void QTPainter::circleInRect(QRectF &rect){
    std::vector<ID> vecCircleID=ClosesPoint::enteringInRect(*circleStorage, rect);
    selectedIDCircle.clear();
    for (const ID &id : vecCircleID) {
        selectedIDCircle[id] = Color::Blue;
    }
}
void QTPainter::arcsInRect(QRectF &rect){
    std::vector<ID> vecArcID=ClosesPoint::enteringInRect(*arcStorage, rect);
    selectedIDArc.clear();
    for (const ID &id : vecArcID) {
        selectedIDArc[id] = Color::Blue;
    }
}


[[maybe_unused]] void QTPainter::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    Scaling::setActualMonitorSize(width(), height());
    update();
}


void QTPainter::paintEvent(QPaintEvent *event) {

    QPainter painter(this);

    if (!painter.isActive()) {
        throw std::runtime_error("Painter not active!");
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    // Moving to the center of the screen
    painter.translate((int) (width() / 2 + Scaling::getDeltaX()), (int) (height() / 2 + Scaling::getDeltaY()));

    DrawBackground::drawFon(painter);

    // If the cursor is in the area, then draw its values
    if (ModeManager::getCursor()) {
        DrawAdditionalInf::drawCursor(painter);
    }


    /*************** Drawing shapes with the mouse ********************/

    if (ModeManager::getCursor()) {

        QPointF cursor={Scaling::logicCursorX(),Scaling::logicCursorY()};

         if ( ModeManager::getActiveMode(WorkModes::Point)||
              ModeManager::getActiveMode(WorkModes::Circle) ||
              ModeManager::getActiveMode(WorkModes::Arc)) {

            drawingWithMouse.DrawFiguresMouse(painter,cursor);

        } else if (ModeManager::getActiveMode(WorkModes::Section)) {

             drawingWithMouse.DrawFiguresMouse(painter,cursor);

            if (pointStorage != nullptr && pointStorage->size() > 0) {
                QPointF closes = ClosesPoint::findClosestPoint(*pointStorage,cursor.x(),cursor.y()); // Ищем ближайшие точки
                drawingWithMouse.drawHints(painter, closes,cursor);
            }
        }
    }


    /****************** Highlight and move functions ********************/


    if (ModeManager::getActiveMode(WorkModes::Editor)) {

        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {

            if (arcStorage != nullptr && arcStorage->size() > 0 ||
                circleStorage != nullptr && circleStorage->size() > 0 ||
                sectionStorage != nullptr && sectionStorage->size() > 0 ||
                pointStorage != nullptr && pointStorage->size() > 0) {

                findClosesObject();
            }

        } else if (ModeManager::getActiveMode(MouseMode::DoubleClickLeft)) {
            if (circleStorage != nullptr && circleStorage->size() == 0 ||
                sectionStorage != nullptr && sectionStorage->size() == 0 ||
                pointStorage != nullptr && pointStorage->size() == 0 ||
                arcStorage != nullptr && arcStorage->size() == 0){
                if(findClosesObject()){
                    if (selectedIDSection.size() == 1) {
                        auto it = selectedIDSection.begin();
                        ID key = it->first;
                        emit DoubleClickOnObject(key);
                    }
                    if (selectedIDPoint.size()==1) {
                        auto it = selectedIDPoint.begin();
                        ID key = it->first;
                        emit DoubleClickOnObject(key);
                    }
                    if (selectedIDCircle.empty()==1) {
                        auto it = selectedIDCircle.begin();
                        ID key = it->first;
                        emit DoubleClickOnObject(key);
                    }
                    if (selectedIDArc.empty()==1) {
                        auto it = selectedIDArc.begin();
                        ID key = it->first;
                        emit DoubleClickOnObject(key);
                    }
                }
            }
        }

    } else if (ModeManager::getActiveMode(WorkModes::Selected)) {

        QRectF rect = selectedRectangle.selected(painter);
        pointInRect(rect);
        sectionInRect(rect);
        circleInRect(rect);
        arcsInRect(rect);

    } else if (ModeManager::getActiveMode(WorkModes::Move)) {

        if (ModeManager::getActiveMode(MouseMode::RightClick)) {
            drawing = false;
            selectedClear();
        }

        // The mouse button is clamped
        if (!drawing) {
            if (ModeManager::getActiveMode(MouseMode::LeftClick) && findClosesObject()) {
                drawing = true;
                QPointF cursorPressPos = QPointF(Scaling::logicCursorX(), Scaling::logicCursorY());

                if (!selectedIDSection.empty()) {
                    ID id = selectedIDSection.begin()->first;

                    if (sectionStorage->contains(id)) {
                        Section* s = (*sectionStorage)[id];
                        LineVecBeg = QPointF(s->beg->x, s->beg->y) - cursorPressPos;
                        LineVecEnd = QPointF(s->end->x, s->end->y) - cursorPressPos;
                    }
                }

                if (!selectedIDCircle.empty()) {
                    ID id = selectedIDCircle.begin()->first;

                    if (sectionStorage->contains(id)) {
                        Circle *c = (*circleStorage)[id];
                        QPointF center(c->center->x, c->center->y);
                        VecCircle = center - cursorPressPos;
                    }
                }
            } else {
                selectedClear();
            }

        } else {
            if (!ModeManager::getActiveMode(MouseMode::ReleasingLeft)) {
                if (!selectedIDSection.empty()) {
                    emit MovingSection(getVecSelectedIDSections(), LineVecBeg, LineVecEnd);
                }
                if (!selectedIDPoint.empty()) {
                    emit MovingPoint(getVecSelectedIDPoints());
                }
                if (!selectedIDCircle.empty()) {
                    emit MovingCircle(getVecSelectedIDCircles(),VecCircle);
                }
                if (!selectedIDArc.empty()) {
                    emit MovingArc(getVecSelectedIDArcs());
                }
            } else {
                emit EndMoving();
                drawing = false;
            }
        }
    }



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
    // When changing the size of the parent window, we change the size
    Scaling::setActualMonitorSize(parentWidget()->width(), parentWidget()->height());
    resize(parentWidget()->size());
}








