#include "QTPainter.h"


QTPainter::QTPainter(QWidget* parent) : QFrame(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: \"#ffffff\"");

    Scaling::updateScaling();
    Scaling::setStartMonitorSize(this->size());
    Scaling::setStartMonitorSize(this->size());
}


MouseDrawingManager* QTPainter::getMouseManager() {
    return mouseManager.get();
}


KeyWorkWindow* QTPainter::getKeyWW() {
    return keyWW.get();
}


void QTPainter::createNormalPointStyle(PointStyle* style) {
    style->figure.object.size = 1;
    style->pointRadius = 1;
    style->figure.glow.activity = false;
    style->figure.id.activity = false;
}


void QTPainter::createNormalCircleStyle(CircleStyle* style) {
    style->figure.object.size = 1;
    style->figure.glow.activity = false;
    style->figure.id.activity = false;
}


void QTPainter::createNormalLineStyle(LineStyle* style) {
    style->figure.object.size = 1;
    style->figure.glow.activity = false;
    style->figure.id.activity = false;
}


void QTPainter::createNormalArcStyle(ArcStyle* style) {
    style->figure.object.size = 1;
    style->figure.glow.activity = false;
    style->figure.id.activity = false;
}


void QTPainter::createSelectedPointStyle(PointStyle* style) {
    style->figure.object.color = Color::Black;
    style->figure.object.size = 1;
    style->pointRadius = 2;
    style->figure.glow.color = Color::Blue;
    style->figure.glow.size = 5;
    style->figure.glow.activity = true;
    style->figure.id.activity = true;
}


void QTPainter::createSelectedLineStyle(LineStyle* style) {
    style->figure.object.color = Color::Black;
    style->figure.object.size = 2;
    style->figure.glow.color = Color::Blue;
    style->figure.glow.size = 5;
    style->figure.glow.activity = true;
    style->figure.id.activity = true;
}


void QTPainter::createSelectedCircleStyle(CircleStyle* style) {
    style->figure.object.color = Color::Black;
    style->figure.object.size = 2;
    style->figure.glow.color = Color::Blue;
    style->figure.glow.size = 5;
    style->figure.glow.activity = true;
    style->figure.id.activity = true;
}


void QTPainter::createSelectedArcStyle(ArcStyle* style) {
    style->figure.object.color = Color::Black;
    style->figure.object.size = 2;
    style->figure.glow.color = Color::Blue;
    style->figure.glow.size = 5;
    style->figure.glow.activity = true;
    style->figure.id.activity = true;
}


void QTPainter::inArea() {
    QSize size = Scaling::getActualMonitorSize();
    QRectF visibleRect(QPointF(0, 0), size);

    visiblePoints.clear();
    visibleLines.clear();
    visibleCircles.clear();
    visibleArcs.clear();

    // Points
    for (const auto& [id, point]: *casePoints) {
        QPointF screenPos(
                Scaling::scaleCoordinate(point->x) + Scaling::getDelta().x() +
                Scaling::getCenteredCoordinates().width(),
                Scaling::scaleCoordinate(point->y) + Scaling::getDelta().y() +
                Scaling::getCenteredCoordinates().height()
        );

        if (visibleRect.contains(screenPos)) {
            if (bufferPointStyle.contains(id)) {
                PointStyle* style = &bufferPointStyle[id];
                visiblePoints.emplace(id, render::pointShell{point, style});
            } else {
                PointStyle style;
                createNormalPointStyle(&style);
                auto [it, inserted] = bufferPointStyle.emplace(id, style);
                PointStyle* stylePtr = &it->second;
                visiblePoints.emplace(id, render::pointShell{point, stylePtr});
            }

        }
    }

    // Lines
    for (const auto& [id, line]: *caseSections) {
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

        QRectF lineBounding = QRectF(p1, p2).normalized();
        if (visibleRect.contains(p1) || visibleRect.contains(p2) || visibleRect.intersects(lineBounding)) {

            if (bufferLineStyle.contains(id)) {
                LineStyle* style = &bufferLineStyle[id];
                visibleLines.emplace(id, render::lineShell{line, style});
            } else {
                LineStyle style;
                createNormalLineStyle(&style);
                auto [it, inserted] = bufferLineStyle.emplace(id, style);
                LineStyle* stylePtr = &it->second;
                visibleLines.emplace(id, render::lineShell{line, stylePtr});
            }


        }
    }

    // Circles
    for (const auto& [id, circle]: *caseCircles) {
        QPointF center(
                Scaling::scaleCoordinate(circle->center->x) + Scaling::getDelta().x() +
                Scaling::getCenteredCoordinates().width(),
                Scaling::scaleCoordinate(circle->center->y) + Scaling::getDelta().y() +
                Scaling::getCenteredCoordinates().height()
        );
        qreal r = circle->r * Scaling::getZoom();

        QRectF bounding(center - QPointF(r, r), QSizeF(2 * r, 2 * r));
        if (visibleRect.intersects(bounding)) {

            if (bufferCircleStyle.contains(id)) {
                CircleStyle* style = &bufferCircleStyle[id];
                visibleCircles.emplace(id, render::circleShell{circle, style});
            } else {
                CircleStyle style;
                createNormalCircleStyle(&style);
                auto [it, inserted] = bufferCircleStyle.emplace(id, style);
                CircleStyle* stylePtr = &it->second;
                visibleCircles.emplace(id, render::circleShell{circle, stylePtr});
            }

        }
    }

    // Arcs
    for (const auto& [id, arc]: *caseArcs) {
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

            if (bufferArcStyle.contains(id)) {
                ArcStyle* style = &bufferArcStyle[id];
                visibleArcs.emplace(id, render::arcShell{arc, style});
            } else {
                ArcStyle style;
                createNormalArcStyle(&style);
                auto [it, inserted] = bufferArcStyle.emplace(id, style);
                ArcStyle* stylePtr = &it->second;
                visibleArcs.emplace(id, render::arcShell{arc, stylePtr});
            }

        }
    }
}


QVector<ID> QTPainter::getVecSelectedIDPoints() {
    QVector<ID> vec_id;
    vec_id.reserve(visiblePoints.size());

    for (const auto& elem: visiblePoints) {
        const PointStyle* style = elem.second.style;
        if (style->figure.glow.activity) {
            const ID id = elem.first;
            vec_id.push_back(id);
        }
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDLines() {
    QVector<ID> vec_id;
    vec_id.reserve(visibleLines.size());

    for (const auto& elem: visibleLines) {
        const LineStyle* style = elem.second.style;
        if (style->figure.glow.activity) {
            const ID id = elem.first;
            vec_id.push_back(id);
        }
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDCircles() {
    QVector<ID> vec_id;
    vec_id.reserve(visibleCircles.size());

    for (const auto& elem: visibleCircles) {
        const CircleStyle* style = elem.second.style;
        if (style->figure.glow.activity) {
            const ID id = elem.first;
            vec_id.push_back(id);
        }
    }

    return vec_id;
}


QVector<ID> QTPainter::getVecSelectedIDArcs() {
    QVector<ID> vec_id;
    vec_id.reserve(visibleArcs.size());

    for (const auto& elem: visibleArcs) {
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


void QTPainter::selectedClear() {
    QVector<ID> selectedIDPoint = getVecSelectedIDPoints();
    QVector<ID> selectedIDLine = getVecSelectedIDLines();
    QVector<ID> selectedIDCircle = getVecSelectedIDCircles();
    QVector<ID> selectedIDArc = getVecSelectedIDArcs();

    for (const ID& id: selectedIDPoint) {
        createNormalPointStyle(visiblePoints[id].style);
    }
    for (const ID& id: selectedIDCircle) {
        createNormalCircleStyle(visibleCircles[id].style);
    }
    for (const ID& id: selectedIDLine) {
        createNormalLineStyle(visibleLines[id].style);
    }
    for (const ID& id: selectedIDArc) {
        createNormalArcStyle(visibleArcs[id].style);
    }

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

                if (!visiblePoints.contains(id)) {
                    break;
                }

                if (visiblePoints[id].style->figure.glow.activity) {
                    createNormalPointStyle(visiblePoints[id].style);
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    createSelectedPointStyle(visiblePoints[id].style);
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
            const ID& id = it->first;

            if (ClosestPoint::checkFigure(startPoint, endPoint, Scaling::logicCursor(), Scaling::getZoom())) {

                if (!visibleLines.contains(id)) {
                    break;
                }


                if (ModeManager::getActiveMode(WorkModes::ShowSize)) {
                    if (visibleLines[id].style->serviceLine.active) {
                        visibleLines[id].style->serviceLine.active = false;
                    } else {
                        visibleLines[id].style->serviceLine.active = true;
                    }

                    if (!visibleLines[id].style->figure.glow.activity) {
                        createSelectedLineStyle(visibleLines[id].style);
                    }
                    return true;
                }

                if (visibleLines[id].style->figure.glow.activity) {
                    createNormalLineStyle(visibleLines[id].style);
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    createSelectedLineStyle(visibleLines[id].style);
                }


                return true;
            } else if (ClosestPoint::checkFigure(visibleLines[id].style->serviceLine.position,
                                                 Scaling::logicCursor(), Scaling::getZoom())) {
                selectedIDLengthLine.push_back(id);
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

                if (!visibleCircles.contains(id)) {
                    break;
                }
                if (visibleCircles[id].style->figure.glow.activity) {
                    createNormalCircleStyle(visibleCircles[id].style);
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    createSelectedCircleStyle(visibleCircles[id].style);
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

                if (!visibleArcs.contains(id)) {
                    break;
                }
                if (visibleArcs[id].style->figure.glow.activity) {
                    createNormalArcStyle(visibleArcs[id].style);
                } else {
                    if (!shiftPress) {
                        selectedClear();
                    }
                    createSelectedArcStyle(visibleArcs[id].style);
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

    const qint32 rows = 4;
    const qint32 cols = 4;
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

    size_t width = rectangle->width() > 500 ? rectangle->width() : 500;
    size_t height = rectangle->height() > 500 ? rectangle->height() : 500;

    if (chosenFormat == "SVG") {
        QSvgGenerator generator;
        generator.setFileName(filePath);
        generator.setSize(QSize(width, rectangle->height()));
        generator.setViewBox(QRect(0, 0, width, rectangle->height()));
        generator.setTitle("Exported SVG");
        generator.setDescription("Generated by QTPainter");
        QPainter painter(&generator);
        drawGostFrame(&painter, QSize(width, rectangle->height()));
        painter.translate((qint32) (width / 2), (qint32) (height / 2));
        this->drawingFigures(painter);
        painter.end();
    } else if (chosenFormat == "PDF") {
        qint32 dpi = 300;
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
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

        painter.translate((qint32) (width / 2) + 20, (qint32) (height / 2 + 20));
        drawingFigures(painter);
        painter.end();
        if (!pixmapCopy.save(filePath, chosenFormat.toUtf8().constData())) {
            QMessageBox::warning(this, tr("Error"), tr("Failed to save the image: %1").arg(filePath));
        }
    }
}


void QTPainter::selectedElemByID(ID id, const std::string& type) {
    if (type == "Point") {
        createSelectedPointStyle(visiblePoints[id].style);
        visiblePoints[id].style->figure.glow.color = Color::Purple;
    } else if (type == "Circle") {
        createSelectedCircleStyle(visibleCircles[id].style);
        visibleCircles[id].style->figure.glow.color = Color::Purple;
    } else if (type == "Section") {
        createSelectedLineStyle(visibleLines[id].style);
        visibleLines[id].style->figure.glow.color = Color::Purple;
    } else if (type == "Arc") {
        createSelectedArcStyle(visibleArcs[id].style);
        visibleArcs[id].style->figure.glow.color = Color::Purple;
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
            if (!selectedIDLengthLine.empty()) {
                const ID id = *selectedIDLengthLine.begin();

                if (visibleLines.contains(id)) {
                    QPointF off = Scaling::logic(Scaling::getCursorDelta());
                    visibleLines[id].style->serviceLine.offset += QPointF{off.x(), off.y()};
                }
            } else {
                emitMoveFigures();
            }
        } else {
            if (!selectedIDLengthLine.empty()) {
                const ID id = *selectedIDLengthLine.begin();
                visibleLines[id].style->serviceLine.lineStyle.glow.activity = false;
                selectedIDLengthLine.clear();

            } else {
                emit EndMoving();
            }

            drawing = false;
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

        for (auto it = visibleLines.cbegin(); it != visibleLines.cend(); ++it) {
            const ID id = it->first;
            if (visibleLines[id].style->serviceLine.active) {

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
        emit MovingSection(selectedIDLine, pressLineVecBeg, pressLineVecEnd);
    }
    if (!selectedIDPoint.empty()) {
        emit MovingPoint(selectedIDPoint);
    }
    if (!selectedIDCircle.empty()) {
        emit MovingCircle(selectedIDCircle, pressPointCircle);
    }
    if (!selectedIDArc.empty()) {
        emit MovingArc(selectedIDArc);
    }
}


void QTPainter::poseMovingFigures() {
    const QPointF cursorPressPos = Scaling::logicCursor();
    QVector<ID> selectedIdLines = getVecSelectedIDLines();
    QVector<ID> selectedIDCircle = getVecSelectedIDCircles();

    if (!selectedIdLines.empty()) {
        const ID id = *selectedIdLines.begin();

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
    const bool leftClick = ModeManager::getActiveMode(MouseMode::LeftClick);
    const bool releasingClick = ModeManager::getActiveMode(MouseMode::ReleasingLeft);
    const QPointF cursor = Scaling::logicCursor();

    if (leftClick) {
        rectTool->pressButton(cursor);
    }

    if (releasingClick) {
        rectTool->releasingButton();
    }

    rectTool->draw(painter, cursor);

    QRectF rect = rectTool->getRect();
    pointInRect(rect);
    lineInRect(rect);
    circleInRect(rect);
    arcsInRect(rect);
}


void QTPainter::pointInRect(QRectF& rect) {
    if (casePoints == nullptr) {
        return;
    }

    QVector<ID> selectedIDPoint = getVecSelectedIDPoints();

    QVector<ID> vecPointID = ClosestPoint::enteringInRect(*casePoints, rect);
    for (const ID& id: selectedIDPoint) {
        createNormalPointStyle(visiblePoints[id].style);
    }

    selectedIDPoint.clear();
    for (const ID& id: vecPointID) {
        createSelectedPointStyle(visiblePoints[id].style);
    }
}


void QTPainter::lineInRect(QRectF& rect) {
    if (caseSections == nullptr) {
        return;
    }

    QVector<ID> selectedIDLine = getVecSelectedIDLines();

    QVector<ID> vecSectionID = ClosestPoint::enteringInRect(*caseSections, rect);
    for (const ID& id: selectedIDLine) {
        createNormalLineStyle(visibleLines[id].style);
    }
    selectedIDLine.clear();
    for (const ID& id: vecSectionID) {
        createSelectedLineStyle(visibleLines[id].style);
    }
}


void QTPainter::circleInRect(QRectF& rect) {
    if (caseCircles == nullptr) {
        return;
    }

    QVector<ID> selectedIDCircle = getVecSelectedIDCircles();

    QVector<ID> pressPointCircleID = ClosestPoint::enteringInRect(*caseCircles, rect);
    for (const ID& id: selectedIDCircle) {
        createNormalCircleStyle(visibleCircles[id].style);
    }
    selectedIDCircle.clear();

    for (const ID& id: pressPointCircleID) {
        createSelectedCircleStyle(visibleCircles[id].style);
    }
}


void QTPainter::arcsInRect(QRectF& rect) {
    if (caseArcs == nullptr) {
        return;
    }

    QVector<ID> selectedIDArc = getVecSelectedIDArcs();

    QVector<ID> vecArcID = ClosestPoint::enteringInRect(*caseArcs, rect);
    for (const ID& id: selectedIDArc) {
        createNormalArcStyle(visibleArcs[id].style);
    }
    selectedIDArc.clear();
    for (const ID& id: vecArcID) {
        createSelectedArcStyle(visibleArcs[id].style);
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
//            if (leftClickFlag && findClosestObject()) {
//                doubleClickEvent();
//            }
            if (leftClickFlag) {
                doubleClickEvent();
            }
        }
    } else if (ModeManager::getActiveMode(WorkModes::Selected)) {
        drawRectangle(painter);
    } else if (ModeManager::getActiveMode(WorkModes::Move)) {
        managerMoving();
    } else if (ModeManager::getActiveMode(WorkModes::ShowSize)) {
        if (ModeManager::getActiveMode(MouseMode::LeftClick)) {
            if (leftClickFlag) {
                findClosestObject();
            }
        }
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


void QTPainter::draw() {
    inArea();
    update();
}


void QTPainter::clear() {
    selectedClear();
    Scaling::setZoomZero();
    mouseManager->clear();
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



