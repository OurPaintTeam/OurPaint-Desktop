#include "QTPainter.h"

#include "Scaling.h"
#include "DrawFigures.h"
#include "ClosestPoint.h"
#include "DrawBackground.h"
#include "GeometricObjects.h"
#include "ID.h"
#include "BoundBox.h"
#include "Colors.h"
#include "MouseDrawingManager.h"
#include "DrawRectangleTool.h"
#include "RenderStyle.h"
#include "RenderPoints.h"
#include "RenderLines.h"
#include "RenderCircles.h"
#include "RenderArcs.h"
#include "RenderDistance.h"
#include "MouseEventWorkWindow.h"
#include "KeyWorkWindow.h"


QTPainter::QTPainter(QWidget* parent) : QFrame(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: \"#ffffff\"");

    Scaling::updateScaling();
    Scaling::setStartMonitorSize(this->size());
    Scaling::setStartMonitorSize(this->size());

    createNewContainer("Default");
}

bool QTPainter::createNewContainer(const QString& name) {
    if (name.isEmpty()) {
        qWarning() << "Container name cannot be empty";
        return false;
    }

    if (namedContainers.find(name) != namedContainers.end()) {
        qWarning() << "Container with name" << name << "already exists";
        return false;
    }

    auto container = std::make_unique<Container>();
    Container* rawPtr = container.get();
    namedContainers[name] = std::move(container);

    activeContainer = rawPtr;

    qDebug() << "Container created:" << name;

    return true;
}


bool QTPainter::setActiveContainer(const QString& name) {

    auto it = namedContainers.find(name);
    if (it == namedContainers.end()) {
        qWarning() << "Container" << name << "not found";
        return false;
    }

    activeContainer = it->second.get();

    qDebug() << "Active container changed to:" << name;
    update();

    return true;
}


bool QTPainter::deleteContainer(const QString& name) {

    auto it = namedContainers.find(name);
    if (it == namedContainers.end()) {
        qWarning() << "Container" << name << "not found";
        return false;
    }

    if (it->second.get() == activeContainer) {
        qWarning() << "Cannot delete active container:" << name;
        return false;
    }

    namedContainers.erase(it);

    qDebug() << "Container deleted:" << name;

    return true;
}


MouseDrawingManager* QTPainter::getMouseManager() const {
    return activeContainer->mouseManager.get();
}


KeyWorkWindow* QTPainter::getKeyWW() const {
    return keyWW.get();
}


void QTPainter::inArea() {
    QSize size = Scaling::getActualMonitorSize();
    QRectF visibleRect(QPointF(0, 0), size);

    activeContainer->visiblePoints.clear();
    activeContainer->visibleLines.clear();
    activeContainer->visibleCircles.clear();
    activeContainer->visibleArcs.clear();

    // Points
    for (const auto& [id, point]: *objectContainer->casePoints) {
        QPointF screenPos(
                Scaling::scaleCoordinate(point->x) + Scaling::getDelta().x() +
                Scaling::getCenteredCoordinates().width(),
                Scaling::scaleCoordinate(point->y) + Scaling::getDelta().y() +
                Scaling::getCenteredCoordinates().height()
        );

        if (visibleRect.contains(screenPos)) {
            if (activeContainer->bufferPointStyle.contains(id)) {
                PointStyle* style = &activeContainer->bufferPointStyle[id];
                activeContainer->visiblePoints.emplace(id, render::pointShell{point, style});
            } else {
                PointStyle style;
                style.createNormalPointStyle();
                auto [it, inserted] = activeContainer->bufferPointStyle.emplace(id, style);
                PointStyle* stylePtr = &it->second;
                activeContainer->visiblePoints.emplace(id, render::pointShell{point, stylePtr});
            }

        }
    }

    // Lines
    for (const auto& [id, line]: *objectContainer->caseSections) {
        QPointF p1(
                Scaling::scaleCoordinate(line->beg->x) + Scaling::getDelta().x() +
                Scaling::getCenteredCoordinates().width(),
                Scaling::scaleCoordinate(line->beg->y) + Scaling::getDelta().y() +
                Scaling::getCenteredCoordinates().height()
        );
        QPointF p2(
                Scaling::scaleCoordinate(line->end->x) + Scaling::getDelta().x() +
                Scaling::getCenteredCoordinates().width(),
                Scaling::scaleCoordinate(line->end->y) + Scaling::getDelta().y() +
                Scaling::getCenteredCoordinates().height()
        );

        if (QRectF lineBounding = QRectF(p1, p2).normalized(); visibleRect.contains(p1) || visibleRect.contains(p2) || visibleRect.intersects(lineBounding)) {

            if (activeContainer->bufferLineStyle.contains(id)) {
                LineStyle* style = &activeContainer->bufferLineStyle[id];
                activeContainer->visibleLines.emplace(id, render::lineShell{line, style});
            } else {
                LineStyle style;
                style.createNormalLineStyle();
                auto [it, inserted] = activeContainer->bufferLineStyle.emplace(id, style);
                LineStyle* stylePtr = &it->second;
                activeContainer->visibleLines.emplace(id, render::lineShell{line, stylePtr});
            }


        }
    }

    // Circles
    for (const auto& [id, circle]: *objectContainer->caseCircles) {
        QPointF center(
                Scaling::scaleCoordinate(circle->center->x) + Scaling::getDelta().x() +
                Scaling::getCenteredCoordinates().width(),
                Scaling::scaleCoordinate(circle->center->y) + Scaling::getDelta().y() +
                Scaling::getCenteredCoordinates().height()
        );
        qreal r = circle->r * Scaling::getZoom();

        if (QRectF bounding(center - QPointF(r, r), QSizeF(2 * r, 2 * r)); visibleRect.intersects(bounding)) {

            if (activeContainer->bufferCircleStyle.contains(id)) {
                CircleStyle* style = &activeContainer->bufferCircleStyle[id];
                activeContainer->visibleCircles.emplace(id, render::circleShell{circle, style});
            } else {
                CircleStyle style;
                style.createNormalCircleStyle();
                auto [it, inserted] = activeContainer->bufferCircleStyle.emplace(id, style);
                CircleStyle* stylePtr = &it->second;
                activeContainer->visibleCircles.emplace(id, render::circleShell{circle, stylePtr});
            }

        }
    }

    // Arcs
    for (const auto& [id, arc]: *objectContainer->caseArcs) {
        QPointF center(
                Scaling::scaleCoordinate(arc->center->x) + Scaling::getDelta().x() +
                Scaling::getCenteredCoordinates().width(),
                Scaling::scaleCoordinate(arc->center->y) + Scaling::getDelta().y() +
                Scaling::getCenteredCoordinates().height()
        );

        qreal dist = QLineF({arc->beg->x,arc->beg->y}, {arc->center->x,arc->center->y}).length();
        qreal r = dist * Scaling::getZoom();

        QRectF bounding(center - QPointF(r, r), QSizeF(2 * r, 2 * r));
        if (visibleRect.intersects(bounding)) {

            if (activeContainer->bufferArcStyle.contains(id)) {
                ArcStyle* style = &activeContainer->bufferArcStyle[id];
                activeContainer->visibleArcs.emplace(id, render::arcShell{arc, style});
            } else {
                ArcStyle style;
                style.createNormalArcStyle();
                auto [it, inserted] = activeContainer->bufferArcStyle.emplace(id, style);
                ArcStyle* stylePtr = &it->second;
                activeContainer->visibleArcs.emplace(id, render::arcShell{arc, stylePtr});
            }

        }
    }
}


QVector<ID> QTPainter::getVecSelectedIDPoints() const {
    QVector<ID> vec_id;
    vec_id.reserve(activeContainer->visiblePoints.size());

    for (const auto& elem: activeContainer->visiblePoints) {
        const PointStyle* style = elem.second.style;
        if (style->figure.glow.activity) {
            const ID id = elem.first;
            vec_id.push_back(id);
        }
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDLines() const {
    QVector<ID> vec_id;
    vec_id.reserve(activeContainer->visibleLines.size());

    for (const auto& elem: activeContainer->visibleLines) {
        const LineStyle* style = elem.second.style;
        if (style->figure.glow.activity) {
            const ID id = elem.first;
            vec_id.push_back(id);
        }
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDCircles() const {
    QVector<ID> vec_id;
    vec_id.reserve(activeContainer->visibleCircles.size());

    for (const auto& elem: activeContainer->visibleCircles) {
        const CircleStyle* style = elem.second.style;
        if (style->figure.glow.activity) {
            const ID id = elem.first;
            vec_id.push_back(id);
        }
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDArcs() const {
    QVector<ID> vec_id;
    vec_id.reserve(activeContainer->visibleArcs.size());

    for (const auto& elem: activeContainer->visibleArcs) {
        const ArcStyle* style = elem.second.style;
        if (style->figure.glow.activity) {
            const ID id = elem.first;
            vec_id.push_back(id);
        }
    }

    return vec_id;
}


std::optional<QPair<ID, ID>> QTPainter::getPairSelectedID() {
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

    if (!collectIDs(getVecSelectedIDPoints())) { return std::nullopt; }
    if (!collectIDs(getVecSelectedIDLines())) { return std::nullopt; }
    if (!collectIDs(getVecSelectedIDCircles())) { return std::nullopt; }
    if (!collectIDs(getVecSelectedIDArcs())) { return std::nullopt; }

    return (IDs.size() == 2)
           ? std::optional<QPair<ID, ID>>(QPair<ID, ID>(IDs[0], IDs[1]))
           : std::nullopt;
}


void QTPainter::selectedClear() const {
    QVector<ID> selectedIDPoint = getVecSelectedIDPoints();
    QVector<ID> selectedIDLine = getVecSelectedIDLines();
    QVector<ID> selectedIDCircle = getVecSelectedIDCircles();
    QVector<ID> selectedIDArc = getVecSelectedIDArcs();

    for (const ID& id: selectedIDPoint) {
        activeContainer->visiblePoints[id].style->createNormalPointStyle();
    }
    for (const ID& id: selectedIDCircle) {
        activeContainer->visibleCircles[id].style->createNormalCircleStyle();
    }
    for (const ID& id: selectedIDLine) {
        activeContainer->visibleLines[id].style->createNormalLineStyle();
    }
    for (const ID& id: selectedIDArc) {
        activeContainer->visibleArcs[id].style->createNormalArcStyle();
    }

    activeContainer->rectTool->clear();
}


bool QTPainter::leftClickTimer() const {
    if (activeContainer->lastClickTime.isValid() && activeContainer->lastClickTime.elapsed() < 300) {
        return false;
    }

    activeContainer->lastClickTime.restart();
    return true;
}


bool QTPainter::findClosestObject() const {
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool doubleClick = ModeManager::getActiveMode(MouseMode::DoubleClickLeft);
    const bool shiftPress = ModeManager::getActiveMode(KeyMode::Shift);

    if (!leftClick && !doubleClick) {
        return false;
    }

    if (objectContainer && objectContainer->casePoints != nullptr) {
        for (auto it = objectContainer->casePoints->cbegin(); it != objectContainer->casePoints->cend(); ++it) {
            const Point* point = it->second;
            const QPointF pos(point->x, point->y);

            if (ClosestPoint::checkFigure(pos, Scaling::logicCursor(), Scaling::getZoom())) {
                const ID& id = it->first;

                if (!activeContainer->visiblePoints.contains(id)) {
                    break;
                }

                if (activeContainer->visiblePoints[id].style->figure.glow.activity) {
                    activeContainer->visiblePoints[id].style->createNormalPointStyle();
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    activeContainer->visiblePoints[id].style->createSelectedPointStyle();
                }

                return true;
            }
        }
    }

    if (objectContainer && objectContainer->caseSections != nullptr) {
        for (auto it = objectContainer->caseSections->cbegin(); it != objectContainer->caseSections->cend(); ++it) {
            const Section* section = it->second;
            const QPointF startPoint(section->beg->x, section->beg->y);
            const QPointF endPoint(section->end->x, section->end->y);
            const ID& id = it->first;

            if (ClosestPoint::checkFigure(startPoint, endPoint, Scaling::logicCursor(), Scaling::getZoom())) {

                if (!activeContainer->visibleLines.contains(id)) {
                    break;
                }


                if (ModeManager::getActiveMode(WorkModes::ShowSize)) {
                    if (activeContainer->visibleLines[id].style->serviceLine.active) {
                        activeContainer->visibleLines[id].style->serviceLine.active = false;
                    } else {
                        activeContainer->visibleLines[id].style->serviceLine.active = true;
                    }

                    if (!activeContainer->visibleLines[id].style->figure.glow.activity) {
                        activeContainer->visibleLines[id].style->createSelectedLineStyle();
                    }
                    return true;
                }

                if (activeContainer->visibleLines[id].style->figure.glow.activity) {
                    activeContainer->visibleLines[id].style->createNormalLineStyle();
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    activeContainer->visibleLines[id].style->createSelectedLineStyle();
                }


                return true;
            }

            if (ClosestPoint::checkFigure(activeContainer->visibleLines[id].style->serviceLine.position,
                                                 Scaling::logicCursor(), Scaling::getZoom())) {
                activeContainer->selectedIDLengthLine.push_back(id);
                return true;
            }
        }
    }

    if (objectContainer && objectContainer->caseCircles != nullptr) {
        for (auto it = objectContainer->caseCircles->cbegin(); it != objectContainer->caseCircles->cend(); ++it) {
            const Circle* circle = it->second;
            const QPointF center(circle->center->x, circle->center->y);
            if (ClosestPoint::checkFigure(center, circle->r,
                                          Scaling::logicCursor(), Scaling::getZoom())) {
                const ID& id = it->first;

                if (!activeContainer->visibleCircles.contains(id)) {
                    break;
                }
                if (activeContainer->visibleCircles[id].style->figure.glow.activity) {
                    activeContainer->visibleCircles[id].style->createNormalCircleStyle();
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                   activeContainer->visibleCircles[id].style->createSelectedCircleStyle();
                }

                return true;
            }
        }
    }

    if (objectContainer && objectContainer->caseArcs != nullptr) {
        for (auto it = objectContainer->caseArcs->cbegin(); it != objectContainer->caseArcs->cend(); ++it) {
            const Arc* arc = it->second;
            const QPointF startPoint(arc->beg->x, arc->beg->y);
            const QPointF endPoint(arc->end->x, arc->end->y);
            const QPointF centerPoint(arc->center->x, arc->center->y);

            if (ClosestPoint::checkFigure(startPoint, endPoint, centerPoint,
                                          Scaling::logicCursor(), Scaling::getZoom())) {
                const ID& id = it->first;

                if (!activeContainer->visibleArcs.contains(id)) {
                    break;
                }
                if (activeContainer->visibleArcs[id].style->figure.glow.activity) {
                    activeContainer->visibleArcs[id].style->createNormalArcStyle();
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    activeContainer->visibleArcs[id].style->createSelectedArcStyle();
                }

                return true;
            }
        }
    }

    selectedClear();
    return false;
}


void QTPainter::drawingFigures(QPainter& painter) const {
    if (!objectContainer) {
        qCritical() << "ObjectContainer is null";
        return;
    }

    if (objectContainer->casePoints != nullptr && !objectContainer->casePoints->empty()) {
        render::drawFigures(painter, activeContainer->visiblePoints);
    }

    if (objectContainer->caseSections != nullptr && !objectContainer->caseSections->empty()) {
        render::drawFigures(painter, activeContainer->visibleLines);
    }

    if (objectContainer->caseCircles != nullptr && !objectContainer->caseCircles->empty()) {
        render::drawFigures(painter, activeContainer->visibleCircles);
    }

    if (objectContainer->caseArcs != nullptr && !objectContainer->caseArcs->empty()) {
        render::drawFigures(painter, activeContainer->visibleArcs);
    }

}


void QTPainter::drawGostFrame(QPainter* painter, const QSize& size) {

    constexpr qint32 margin = 20;
    const qint32 frameWidth = size.width() - 2 * margin;
    const qint32 frameHeight = size.height() - 2 * margin;


    QRect frameRect(margin, margin, frameWidth, frameHeight);
    painter->setPen(QPen(Qt::black, 1));
    painter->drawRect(frameRect);

    constexpr qint32 stampHeight = 60;
    constexpr qint32 stampWidth = 180;


    qint32 stampTopY = frameRect.bottom() - stampHeight;
    painter->drawLine(frameRect.left(), stampTopY, frameRect.right(), stampTopY);

    QRect stampRect(frameRect.right() - stampWidth, stampTopY, stampWidth, stampHeight);
    painter->drawRect(stampRect);

    constexpr qint32 rows = 4;
    constexpr qint32 cols = 4;
    qint32 cellWidth = stampWidth / cols;
    qint32 cellHeight = stampHeight / rows;

    for (qint32 i = 1; i < rows; ++i) {
        qint32 y = stampRect.top() + i * cellHeight;
        painter->drawLine(stampRect.left(), y, stampRect.right(), y);
    }

    for (qint32 j = 1; j < cols; ++j) {
        qint32 x = stampRect.left() + j * cellWidth;
        painter->drawLine(x, stampRect.top(), x, stampRect.bottom());
    }
    painter->setFont(QFont("Arial", 8));
    painter->drawText(stampRect.adjusted(4, 4, -4, -4),
                      Qt::AlignLeft | Qt::AlignTop,
                      "Назв.\nАвтор\nДата");
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

    size_t width = objectContainer->rectangle->width() > 500 ? objectContainer->rectangle->width() : 500;
    size_t height = objectContainer->rectangle->height() > 500 ? objectContainer->rectangle->height() : 500;

    if (chosenFormat == "SVG") {
        QSvgGenerator generator;
        generator.setFileName(filePath);
        generator.setSize(QSize(width, objectContainer->rectangle->height()));
        generator.setViewBox(QRect(0, 0, width, objectContainer->rectangle->height()));
        generator.setTitle("Exported SVG");
        generator.setDescription("Generated by QTPainter");
        QPainter painter(&generator);
        drawGostFrame(&painter, QSize(width, objectContainer->rectangle->height()));
        painter.translate(static_cast<qint32>(width / 2), static_cast<qint32>(height / 2));
        this->drawingFigures(painter);
        painter.end();
    } else if (chosenFormat == "PDF") {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            qint32 dpi = 300;
            QPdfWriter writer(&file);
            writer.setPageSize(QPageSize(QPageSize::A3));
            writer.setResolution(dpi);

            QPainter pdfPainter(&writer);

            QSize pageSize(writer.width(), writer.height());

            drawGostFrame(&pdfPainter, pageSize);

            QPixmap pixmap = this->grab(this->rect());
            if (!pixmap.isNull()) {
                QPixmap scaledPixmap = pixmap.scaled(pageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
        QPixmap pixmapCopy(QSize(width + 40, height + 40));
        pixmapCopy.fill(Qt::white);
        QPainter painter(&pixmapCopy);
        drawGostFrame(&painter, QSize(width + 40, height + 40));

        painter.translate(static_cast<qint32>(width / 2) + 20, static_cast<qint32>(height / 2 + 20));
        drawingFigures(painter);
        painter.end();
        if (!pixmapCopy.save(filePath, chosenFormat.toUtf8().constData())) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to save the image: %1").arg(filePath));
        }
    }
}


void QTPainter::selectedElemByID(const ID id, const std::string& type) const {
    if (type == "Point") {
        activeContainer->visiblePoints[id].style->createSelectedPointStyle();
        activeContainer->visiblePoints[id].style->figure.glow.color = Color::Purple;
    } else if (type == "Circle") {
        activeContainer->visibleCircles[id].style->createSelectedCircleStyle();
        activeContainer->visibleCircles[id].style->figure.glow.color = Color::Purple;
    } else if (type == "Section") {
        activeContainer->visibleLines[id].style->createSelectedLineStyle();
        activeContainer->visibleLines[id].style->figure.glow.color = Color::Purple;
    } else if (type == "Arc") {
        activeContainer->visibleArcs[id].style->createSelectedArcStyle();
        activeContainer->visibleArcs[id].style->figure.glow.color = Color::Purple;
    }
}


void QTPainter::managerMoving() {
    if (ModeManager::getActiveMode(MouseMode::RightClick)) {
        activeContainer->drawing = false;
        selectedClear();
    }

    // The mouse button is clamped
    if (!activeContainer->drawing) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if (findClosestObject()) {
                activeContainer->drawing = true;
                poseMovingFigures();
            }
        } else {
            selectedClear();
        }
    } else {
        if (!ModeManager::getActiveMode(MouseMode::ReleasingLeft)) {
            if (!activeContainer->selectedIDLengthLine.empty()) {
                const ID id = *activeContainer->selectedIDLengthLine.begin();

                if (activeContainer->visibleLines.contains(id)) {
                    QPointF off = Scaling::logic(Scaling::getCursorDelta());
                    activeContainer->visibleLines[id].style->serviceLine.offset += QPointF{off.x(), off.y()};
                }
            } else {
                emitMoveFigures();
            }
        } else {
            if (!activeContainer->selectedIDLengthLine.empty()) {
                const ID id = *activeContainer->selectedIDLengthLine.begin();
                activeContainer->visibleLines[id].style->serviceLine.lineStyle.glow.activity = false;
                activeContainer->selectedIDLengthLine.clear();

            } else {
                emit EndMoving();
            }

            activeContainer->drawing = false;
        }
    }
}


void QTPainter::doubleClickEvent() {
    constexpr qint8 SIZE = 1;
    QVector<ID> selectedIDPoint = getVecSelectedIDPoints();
    QVector<ID> selectedIDLine = getVecSelectedIDLines();
    QVector<ID> selectedIDCircle = getVecSelectedIDCircles();
    QVector<ID> selectedIDArc = getVecSelectedIDArcs();

    if (selectedIDPoint.size() == SIZE) {
        auto it = selectedIDPoint.begin();
        ID key = *it;
        emit DoubleClickOnObject(key);
    } else if (selectedIDLine.size() == SIZE) {
        auto it = selectedIDLine.begin();
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
    } else {
        bool flag = false;

        for (auto it = activeContainer->visibleLines.cbegin(); it != activeContainer->visibleLines.cend(); ++it) {
            const ID id = it->first;
            if (activeContainer->visibleLines[id].style->serviceLine.active) {

                LineStyle* style = it->second.style;
                const QPointF cursor = Scaling::logicCursor();
                const QPointF lt{style->lengthLabel.label.position.lt.x(), -style->lengthLabel.label.position.lt.y()};
                const QPointF rt{style->lengthLabel.label.position.rt.x(), -style->lengthLabel.label.position.rt.y()};
                const QPointF lb{style->lengthLabel.label.position.lb.x(), -style->lengthLabel.label.position.lb.y()};
                const QPointF rb{style->lengthLabel.label.position.rb.x(), -style->lengthLabel.label.position.rb.y()};

                QPolygonF poly;
                poly << (lt) << (rt)
                     << (rb) << (lb);
                QRectF rect = poly.boundingRect();

                //p.fillRect(Scaling::scaleCoordinate(rect), Qt::red);
                //qDebug()<<rect<<cursor;
                if (ClosestPoint::pointInRect(cursor, rect)) {
                    style->lengthLabel.label.active = false;
                    style->lengthLabel.label.editing = true;
                    flag = true;
                }

                if (flag) {
                    return;
                }

            }
        }

    }


}


void QTPainter::emitMoveFigures() {
    QVector<ID> selectedIDLine = getVecSelectedIDLines();
    QVector<ID> selectedIDPoint = getVecSelectedIDPoints();
    QVector<ID> selectedIDCircle = getVecSelectedIDCircles();
    QVector<ID> selectedIDArc = getVecSelectedIDArcs();

    if (!selectedIDLine.empty()) {
        emit MovingSection(selectedIDLine, activeContainer->pressLineVecBeg, activeContainer->pressLineVecEnd);
    }
    if (!selectedIDPoint.empty()) {
        emit MovingPoint(selectedIDPoint);
    }
    if (!selectedIDCircle.empty()) {
        emit MovingCircle(selectedIDCircle, activeContainer->pressPointCircle);
    }
    if (!selectedIDArc.empty()) {
        emit MovingArc(selectedIDArc);
    }
}


void QTPainter::poseMovingFigures() const {
    const QPointF cursorPressPos = Scaling::logicCursor();
    QVector<ID> selectedIdLines = getVecSelectedIDLines();
    QVector<ID> selectedIDCircle = getVecSelectedIDCircles();

    if (!selectedIdLines.empty()) {
        const ID id = *selectedIdLines.begin();

        if (objectContainer->caseSections->contains(id)) {
            const Section* s = (*objectContainer->caseSections)[id];
            activeContainer->pressLineVecBeg = QPointF(s->beg->x, s->beg->y) - cursorPressPos;
            activeContainer->pressLineVecEnd = QPointF(s->end->x, s->end->y) - cursorPressPos;
        }
    }

    if (!selectedIDCircle.empty()) {
        const ID id = *selectedIDCircle.begin();

        if (objectContainer->caseCircles->contains(id)) {
            const Circle* c = (*objectContainer->caseCircles)[id];
            const QPointF center(c->center->x, c->center->y);
            activeContainer->pressPointCircle = center - cursorPressPos;
        }
    }
}


void QTPainter::drawRectangle(QPainter& painter) const {
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool releasingClick = ModeManager::getActiveMode(MouseMode::ReleasingLeft);
    const QPointF cursor = Scaling::logicCursor();

    if (leftClick) {
        activeContainer->rectTool->pressButton(cursor);
    }

    if (releasingClick) {
        activeContainer->rectTool->releasingButton();
    }

    activeContainer->rectTool->draw(painter, cursor);

    const QRectF rect = activeContainer->rectTool->getRect();
    pointInRect(rect);
    lineInRect(rect);
    circleInRect(rect);
    arcsInRect(rect);
}


void QTPainter::pointInRect(const QRectF& rect) const {
    if (objectContainer->casePoints == nullptr) {
        return;
    }

    QVector<ID> selectedIDPoint = getVecSelectedIDPoints();

    QVector<ID> vecPointID = ClosestPoint::enteringInRect(*objectContainer->casePoints, rect);
    for (const ID& id: selectedIDPoint) {
        activeContainer->visiblePoints[id].style->createNormalPointStyle();
    }

    selectedIDPoint.clear();
    for (const ID& id: vecPointID) {
        activeContainer->visiblePoints[id].style->createSelectedPointStyle();
    }
}


void QTPainter::lineInRect(const QRectF& rect) const {
    if (objectContainer->caseSections == nullptr) {
        return;
    }

    QVector<ID> selectedIDLine = getVecSelectedIDLines();

    QVector<ID> vecSectionID = ClosestPoint::enteringInRect(*objectContainer->caseSections, rect);
    for (const ID& id: selectedIDLine) {
        activeContainer->visibleLines[id].style->createNormalLineStyle();
    }
    selectedIDLine.clear();
    for (const ID& id: vecSectionID) {
        activeContainer->visibleLines[id].style->createSelectedLineStyle();
    }
}


void QTPainter::circleInRect(const QRectF& rect) const {
    if (objectContainer->caseCircles == nullptr) {
        return;
    }

    QVector<ID> selectedIDCircle = getVecSelectedIDCircles();

    QVector<ID> pressPointCircleID = ClosestPoint::enteringInRect(*objectContainer->caseCircles, rect);
    for (const ID& id: selectedIDCircle) {
        activeContainer->visibleCircles[id].style->createNormalCircleStyle();
    }
    selectedIDCircle.clear();

    for (const ID& id: pressPointCircleID) {
        activeContainer->visibleCircles[id].style->createSelectedCircleStyle();
    }
}


void QTPainter::arcsInRect(const QRectF& rect) const {
    if (objectContainer->caseArcs == nullptr) {
        return;
    }

    QVector<ID> selectedIDArc = getVecSelectedIDArcs();

    QVector<ID> vecArcID = ClosestPoint::enteringInRect(*objectContainer->caseArcs, rect);
    for (const ID& id: selectedIDArc) {
        activeContainer->visibleArcs[id].style->createNormalArcStyle();
    }
    selectedIDArc.clear();
    for (const ID& id: vecArcID) {
        activeContainer->visibleArcs[id].style->createSelectedArcStyle();
    }
}


[[maybe_unused]] void QTPainter::resizeEvent(QResizeEvent*) {
    Scaling::setActualMonitorSize(this->size());
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
                if (objectContainer && objectContainer->casePoints != nullptr) {
                    const QPointF cursor = Scaling::logicCursor();
                    const QPointF closest = ClosestPoint::findClosestPoint(*objectContainer->casePoints,
                                                                     cursor); // Finding the closest points
                    activeContainer->mouseManager->setClosestPoint(closest);
                }
            }

        activeContainer->mouseManager->managerMouseDrawing(painter);
    }


    /****************** Highlight and move functions ********************/
    if (ModeManager::getActiveMode(WorkModes::Editor)) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if (activeContainer->leftClickFlag) {
                findClosestObject();
            }
        } else if (ModeManager::getActiveMode(MouseMode::DoubleClickLeft)) {
//            if (leftClickFlag && findClosestObject()) {
//                doubleClickEvent();
//            }
            if (activeContainer->leftClickFlag) {
                doubleClickEvent();
            }
        }
    } else if (ModeManager::getActiveMode(WorkModes::Selected)) {
        drawRectangle(painter);
    } else if (ModeManager::getActiveMode(WorkModes::Move)) {
        managerMoving();
    } else if (ModeManager::getActiveMode(WorkModes::ShowSize)) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if (activeContainer->leftClickFlag) {
                findClosestObject();
            }
        }
    }


    if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
        activeContainer->leftClickFlag = leftClickTimer();
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


void QTPainter::draw() {
    inArea();
    update();
}


void QTPainter::clear() {
    selectedClear();
    Scaling::setZoomZero();
    activeContainer->mouseManager->clear();
}

void QTPainter::initObjectContainer(ObjectContainer& container) {
    objectContainer = &container;
}



