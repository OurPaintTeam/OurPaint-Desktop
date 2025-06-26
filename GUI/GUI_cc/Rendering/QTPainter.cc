#include "QTPainter.h"

QTPainter::QTPainter(QWidget* parent) : QFrame(parent),
                                        mouseManager(std::make_unique<MouseDrawingManager>()),
                                        rectTool(std::make_unique<DrawRectangleTool>()),
                                        leftClickFlag(true), drawing(false){

    if (parent) {
        // We take the size from the father
        resize(parentWidget()->size());
        connect(parent->window(), SIGNAL(resize()), this, SLOT(onWorkWindowResized()));


    }

    connect(mouseManager.get(), &MouseDrawingManager::SigPoint, this, &QTPainter::onSigPoint);
    connect(mouseManager.get(), &MouseDrawingManager::SigSection, this, &QTPainter::onSigSection);
    connect(mouseManager.get(), &MouseDrawingManager::SigCircle, this, &QTPainter::onSigCircle);
    connect(mouseManager.get(), &MouseDrawingManager::SigArc, this, &QTPainter::onSigArc);


    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: \"#ffffff\"");
    setAttribute(Qt::WA_AcceptTouchEvents);

    Scaling::updateScaling();
    Scaling::setStartMonitorSize(size());
}


QVector<ID> QTPainter::getVecSelectedIDPoints() {
    QVector<ID> vec_id;
    vec_id.reserve(selectedIDPoint.size());

    for (const auto& id: selectedIDPoint) {
        vec_id.push_back(id);
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDSections() {
    QVector<ID> vec_id;
    vec_id.reserve(selectedIDSection.size());

    for (const auto& id: selectedIDSection) {
        vec_id.push_back(id);
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDCircles() {
    QVector<ID> vec_id;
    vec_id.reserve(selectedIDCircle.size());

    for (const auto& id: selectedIDCircle) {
        vec_id.push_back(id);
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDArcs() {
    QVector<ID> vec_id;
    vec_id.reserve(selectedIDArc.size());

    for (const auto& id: selectedIDArc) {
        vec_id.push_back(id);
    }

    return vec_id;
}


std::optional<QPair<ID, ID>> QTPainter::getPairSelectedID() const {
    QVector<ID> IDs;

    auto collectIDs = [&](const auto& container) -> bool {
        for (const auto& id: container) {
            IDs.push_back(id);
            if (IDs.size() > 2) {
                return false;
            }
        }
        return true;
    };

    if (!collectIDs(selectedIDPoint)) { return std::nullopt; }
    if (!collectIDs(selectedIDSection)) { return std::nullopt; }
    if (!collectIDs(selectedIDCircle)) { return std::nullopt; }
    if (!collectIDs(selectedIDArc)) { return std::nullopt; }

    return (IDs.size() == 2)
           ? std::optional<QPair<ID, ID>>(QPair<ID, ID>(IDs[0], IDs[1]))
           : std::nullopt;
}



void QTPainter::draw() {
    inArea();
    update();
}


void QTPainter::clear() {
    selectedClear();
    Scaling::setZoomZero();
    mouseManager->clear();
}


void QTPainter::selectedClear() {
    for (const ID& id: selectedIDPoint) {
        visiblePoints[id].style = createNormalPointStyle();
    }
    for (const ID& id: selectedIDCircle) {
        visibleCircles[id].style = createNormalCircleStyle();
    }
    for (const ID& id: selectedIDSection) {
        visibleLines[id].style = createNormalLineStyle();
    }
    for (const ID& id: selectedIDArc) {
        visibleArcs[id].style = createNormalArcStyle();
    }
    selectedIDPoint.clear();
    selectedIDSection.clear();
    selectedIDCircle.clear();
    selectedIDArc.clear();
    rectTool->clear();
}


bool QTPainter::leftClickTimer() {
    if (lastClickTime.isValid() && lastClickTime.elapsed() < 300) {
        return false;
    }

    lastClickTime.restart();
    return true;
}


bool QTPainter::findClosestObject() {
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool doubleClick = ModeManager::getActiveMode(MouseMode::DoubleClickLeft);
    const bool shiftPress = ModeManager::getActiveMode(KeyMode::Shift);

    if (!leftClick && !doubleClick) {
        return false;
    }

    if (casePoints != nullptr) {
        for (auto it = casePoints->cbegin(); it != casePoints->cend(); ++it) {
            const Point* point = it->second;
            const QPointF pos(point->x, point->y);

            if (ClosestPoint::checkFigure(pos, Scaling::logicCursor(), Scaling::getZoom())) {
                const ID& id = it->first;

                if (selectedIDPoint.contains(id)) {
                    selectedIDPoint.removeOne(id);
                    visiblePoints[id].style=createNormalPointStyle();
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    visiblePoints[id].style=createSelectedPointStyle();
                    selectedIDPoint.append(id);
                }

                return true;
            }
        }
    }

    if (caseSections != nullptr) {
        for (auto it = caseSections->cbegin(); it != caseSections->cend(); ++it) {
            const Section* section = it->second;
            const QPointF startPoint(section->beg->x, section->beg->y);
            const QPointF endPoint(section->end->x, section->end->y);

            if (ClosestPoint::checkFigure(startPoint, endPoint, Scaling::logicCursor(), Scaling::getZoom())) {
                const ID& id = it->first;

                if (selectedIDSection.contains(id)) {
                    selectedIDSection.removeOne(id);
                    visibleLines[id].style=createNormalLineStyle();
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    visibleLines[id].style=createSelectedLineStyle();
                    selectedIDSection.append(id);
                }

                return true;
            }
        }
    }

    if (caseCircles != nullptr) {
        for (auto it = caseCircles->cbegin(); it != caseCircles->cend(); ++it) {
            const Circle* circle = it->second;
            const QPointF center(circle->center->x, circle->center->y);
            if (ClosestPoint::checkFigure(center, circle->r,
                                          Scaling::logicCursor(), Scaling::getZoom())) {
                const ID& id = it->first;

                if (selectedIDCircle.contains(id)) {
                    selectedIDCircle.removeOne(id);
                    visibleCircles[id].style=createNormalCircleStyle();
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    visibleCircles[id].style=createSelectedCircleStyle();
                    selectedIDCircle.append(id);
                }

                return true;
            }
        }
    }

    if (caseArcs != nullptr) {
        for (auto it = caseArcs->cbegin(); it != caseArcs->cend(); ++it) {
            const Arc* arc = it->second;
            const QPointF startPoint(arc->beg->x, arc->beg->y);
            const QPointF endPoint(arc->end->x, arc->end->y);
            const QPointF centerPoint(arc->center->x, arc->center->y);

            if (ClosestPoint::checkFigure(startPoint, endPoint, centerPoint,
                                          Scaling::logicCursor(), Scaling::getZoom())) {
                const ID& id = it->first;

                if (selectedIDArc.contains(id)) {
                    selectedIDArc.removeOne(id);
                    visibleArcs[id].style=createNormalArcStyle();
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    visibleArcs[id].style=createSelectedArcStyle();
                    selectedIDArc.append(id);
                }

                return true;
            }
        }
    }

    selectedClear();
    return false;
}


void QTPainter::drawingFigures(QPainter& painter) {

    if (casePoints != nullptr && !casePoints->empty()) {
        render::drawFigures(painter, visiblePoints);
    }

    if (caseSections != nullptr && !caseSections->empty()) {
        render::drawFigures(painter, visibleLines);
    }

    if (caseCircles != nullptr && !caseCircles->empty()) {
        render::drawFigures(painter, visibleCircles);
    }

    if (caseArcs != nullptr && !caseArcs->empty()) {
        render::drawFigures(painter, visibleArcs);
    }
}


void QTPainter::saveToImage(const QString& fileName, QString& format) {

    if (format.startsWith('.')) {
        format = format.mid(1);
    }

    const QString originalFormat = format.toLower();
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


void QTPainter::selectedElemByID(ID id, const std::string& type) {
    if (type == "Point") {
        selectedIDPoint.push_back(id);
        visiblePoints[id].style=createSelectedPointStyle();
        visiblePoints[id].style.figure.glow.color = Color::Purple;
    } else if (type == "Circle") {
        selectedIDCircle.push_back(id);
        visibleCircles[id].style=createSelectedCircleStyle();
        visibleCircles[id].style.figure.glow.color = Color::Purple;
    } else if (type == "Section") {
        selectedIDSection.push_back(id);
        visibleLines[id].style=createSelectedLineStyle();
        visibleLines[id].style.figure.glow.color = Color::Purple;
    } else if (type == "Arc") {
        selectedIDArc.push_back(id);
        visibleArcs[id].style=createSelectedArcStyle();
        visibleArcs[id].style.figure.glow.color = Color::Purple;
    }
}


void QTPainter::managerMoving() {
    if (ModeManager::getActiveMode(MouseMode::RightClick)) {
        drawing = false;
        selectedClear();
    }

    // The mouse button is clamped
    if (!drawing) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if (findClosestObject()) {
                drawing = true;
                poseMovingFigures();

            }
        } else {
            selectedClear();
        }
    } else {
        if (!ModeManager::getActiveMode(MouseMode::ReleasingLeft)) {
            emitMoveFigures();
        } else {
            emit EndMoving();
            drawing = false;
        }
    }
}


void QTPainter::doubleClickEvent() {
    constexpr qreal SIZE = 1;
    if (selectedIDSection.size() == SIZE) {
        auto it = selectedIDSection.begin();
        ID key = *it;
        emit DoubleClickOnObject(key);
    } else if (selectedIDPoint.size() == SIZE) {
        auto it = selectedIDPoint.begin();
        ID key = *it;
        emit DoubleClickOnObject(key);
    } else if (selectedIDCircle.size() == SIZE) {
        auto it = selectedIDCircle.begin();
        ID key = *it;
        emit DoubleClickOnObject(key);
    } else if (selectedIDArc.size() == SIZE) {
        auto it = selectedIDArc.begin();
        ID key = *it;
        emit DoubleClickOnObject(key);
    }
}


void QTPainter::emitMoveFigures() {
    if (!selectedIDSection.empty()) {
        emit MovingSection(getVecSelectedIDSections(), pressLineVecBeg, pressLineVecEnd);
    }
    if (!selectedIDPoint.empty()) {
        emit MovingPoint(getVecSelectedIDPoints());
    }
    if (!selectedIDCircle.empty()) {
        emit MovingCircle(getVecSelectedIDCircles(), pressPointCircle);
    }
    if (!selectedIDArc.empty()) {
        emit MovingArc(getVecSelectedIDArcs());
    }
}


void QTPainter::poseMovingFigures() {
    const QPointF cursorPressPos = Scaling::logicCursor();

    if (!selectedIDSection.empty()) {
        const ID id = *selectedIDSection.begin();

        if (caseSections->contains(id)) {
            const Section* s = (*caseSections)[id];
            pressLineVecBeg = QPointF(s->beg->x, s->beg->y) - cursorPressPos;
            pressLineVecEnd = QPointF(s->end->x, s->end->y) - cursorPressPos;
        }
    }

    if (!selectedIDCircle.empty()) {
        const ID id = *selectedIDCircle.begin();

        if (caseCircles->contains(id)) {
            const Circle* c = (*caseCircles)[id];
            const QPointF center(c->center->x, c->center->y);
            pressPointCircle = center - cursorPressPos;
        }
    }
}


void QTPainter::drawRectangle(QPainter& painter) {
    const bool leftClick=ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool releasingClick=ModeManager::getActiveMode(MouseMode::ReleasingLeft);
    const QPointF cursor = Scaling::logicCursor();

    if(leftClick){
        rectTool->pressButton(cursor);
    }

    if(releasingClick){
        rectTool->releasingButton();
    }

    rectTool->draw(painter,cursor);

    QRectF rect =  rectTool->getRect();
    pointInRect(rect);
    sectionInRect(rect);
    circleInRect(rect);
    arcsInRect(rect);
}


void QTPainter::onSigPoint(const QPointF& point) {
    emit SigPoint(point);
}


void QTPainter::onSigCircle(const QPointF& center, qreal radius) {
    emit SigCircle(center, radius);
}


void QTPainter::onSigSection(const QPointF& startPoint, const QPointF& endPoint) {
    emit SigSection(startPoint, endPoint);
}


void QTPainter::onSigArc(const QPointF& startPoint, const QPointF& endPoint, const QPointF& center) {
    emit SigArc(startPoint, endPoint, center);
}


void QTPainter::pointInRect(QRectF& rect) {
    if (casePoints == nullptr) {
        return;
    }

    QVector<ID> vecPointID = ClosestPoint::enteringInRect(*casePoints, rect);
    for (const ID& id: selectedIDPoint) {
        visiblePoints[id].style=createNormalPointStyle();
    }

    selectedIDPoint.clear();
    for (const ID& id: vecPointID) {
        visiblePoints[id].style = createSelectedPointStyle();
    }
}


void QTPainter::sectionInRect(QRectF& rect) {
    if (caseSections == nullptr) {
        return;
    }

    QVector<ID> vecSectionID = ClosestPoint::enteringInRect(*caseSections, rect);
    for (const ID& id: selectedIDSection) {
        visibleLines[id].style = createNormalLineStyle();
    }
    selectedIDSection.clear();
    for (const ID& id: vecSectionID) {
        visibleLines[id].style = createSelectedLineStyle();
    }
}


void QTPainter::circleInRect(QRectF& rect) {
    if (caseCircles == nullptr) {
        return;
    }

    QVector<ID> pressPointCircleID = ClosestPoint::enteringInRect(*caseCircles, rect);
    for (const ID& id: selectedIDCircle) {
        visibleCircles[id].style = createNormalCircleStyle();
    }
    selectedIDCircle.clear();

    for (const ID& id: pressPointCircleID) {
        visibleCircles[id].style = createSelectedCircleStyle();
    }
}


void QTPainter::arcsInRect(QRectF& rect) {
    if (caseArcs == nullptr) {
        return;
    }

    QVector<ID> vecArcID = ClosestPoint::enteringInRect(*caseArcs, rect);
    for (const ID& id: selectedIDArc) {
        visibleArcs[id].style = createNormalArcStyle();
    }
    selectedIDArc.clear();
    for (const ID& id: vecArcID) {
        visibleArcs[id].style = createSelectedArcStyle();
    }
}


[[maybe_unused]] void QTPainter::resizeEvent(QResizeEvent* event) {
    QFrame::resizeEvent(event);
    Scaling::setActualMonitorSize(size());
    update();
}


void QTPainter::paintEvent(QPaintEvent* event) {

    QPainter painter(this);

    if (!painter.isActive()) {
        throw std::runtime_error("Painter not active!");
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    // Moving to the center of the screen
    painter.translate((qint32) (width() / 2 + Scaling::getDeltaX()), (qint32) (height() / 2 + Scaling::getDeltaY()));

    DrawBackground::backgroundRender(painter);

    // If the cursor is in the area, then draw its values
    if (ModeManager::getCursor()) {
        DrawAdditionalInf::drawCursor(painter);
    }


    /*************** Drawing shapes with the mouse ********************/
    if (ModeManager::getCursor()) {
        if (ModeManager::getActiveMode(WorkModes::Point) ||
            ModeManager::getActiveMode(WorkModes::Section) ||
            ModeManager::getActiveMode(WorkModes::Circle) ||
            ModeManager::getActiveMode(WorkModes::Arc))
            if (ModeManager::getActiveMode(WorkModes::Section)) {
                if (casePoints != nullptr) {
                    QPointF cursor = Scaling::logicCursor();
                    QPointF closest = ClosestPoint::findClosestPoint(*casePoints,
                                                                     cursor); // Finding the closest points
                    mouseManager->setClosestPoint(closest);
                }
            }

        mouseManager->managerMouseDrawing(painter);
    }


    /****************** Highlight and move functions ********************/
    if (ModeManager::getActiveMode(WorkModes::Editor)) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if (leftClickFlag) {
                findClosestObject();
            }
        } else if (ModeManager::getActiveMode(MouseMode::DoubleClickLeft)) {
            if (leftClickFlag && findClosestObject()) {
                doubleClickEvent();
            }
        }
    } else if (ModeManager::getActiveMode(WorkModes::Selected)) {
        drawRectangle(painter);
    } else if (ModeManager::getActiveMode(WorkModes::Move)) {
        managerMoving();
    }

    if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
        leftClickFlag = leftClickTimer();
    }

    drawingFigures(painter);

    QFrame::paintEvent(event);
}


unsigned long long QTPainter::getWeight() {
    const QSize size = Scaling::getActualMonitorSize();
    return size.width();
}


unsigned long long QTPainter::getHeight() {
    const QSize size = Scaling::getActualMonitorSize();
    return size.height();
}


void QTPainter::getBoundBox(const BoundBox2D& allObjects) {
    rectangle = &allObjects;
}


void QTPainter::initPointCase(std::unordered_map<ID, Point*>& points) {
    casePoints = &points;
}


void QTPainter::initSectionCase(std::unordered_map<ID, Section*>& sections) {
    caseSections = &sections;
}


void QTPainter::initCircleCase(std::unordered_map<ID, Circle*>& circles) {
    caseCircles = &circles;
}


void QTPainter::initArcCase(std::unordered_map<ID, Arc*>& arcs) {
    caseArcs = &arcs;
}


void QTPainter::onWorkWindowResized() {
    // When changing the size of the parent window, we change the size
    Scaling::setActualMonitorSize(size());
    resize(parentWidget()->size());
}