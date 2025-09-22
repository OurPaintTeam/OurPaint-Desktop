#include "RenderLines.h"


namespace render::util {


    void editCoordinateLine(QPainter& painter, ID id, const qreal& angle,
                            const QString& lengthText,
                            LineLengthLabel& style) {

        if (style.label.textEdit == nullptr) {
            QWidget* host = static_cast<QWidget*>(painter.device());
            auto angled = new AngleLineEdit(host);
            style.label.textEdit = angled;
            style.label.textEdit->setFocus();

            angled->setFlagNumber(true);
            angled->setText(lengthText);
            angled->show();

            QObject::connect(angled, &AngleLineEdit::editingFinished, [&,id, angled]() {
                style.label.editing = false;
                notifyLinesUpdated(id,angled->getText().toDouble());
                angled->setText("exit");
                angled->hide();
            });
        }


        auto angled = qobject_cast<AngleLineEdit*>(style.label.textEdit);
        angled->setFocus(Qt::OtherFocusReason);
        if(angled->getText()=="exit"){
            angled->setText(lengthText);
        }

        angled->setRotation(angle);

        // --- Size widget on text ---
        QString currentText;
        currentText = angled->getText();

        QFontMetrics fm(angled->font());
        qint32 w = fm.horizontalAdvance(currentText) + 10;
        qint32 h = fm.height();

        QRectF deviceRect = painter.transform().mapRect(QRectF(-2, -13, w, h));
        QRect newRect = deviceRect.toAlignedRect();

        if (style.label.prevRect != newRect) {
            style.label.textEdit->setGeometry(newRect);
            style.label.prevRect = newRect;
        }

        style.label.textEdit->show();
    }


    bool drawLengthUnderLine(QPainter& painter,ID id,const qreal angle,const qreal length,const QPointF& mid
                            , LineLengthLabel& style) {
        QTransform transform;

        //const qreal dx = start.x() - end.x();
        //const qreal dy = start.y() - end.y();
        //const qreal length = Scaling::logic(qSqrt(dx * dx + dy * dy));

        const QString lengthText = QString::number(length, 'f', 2);
        const QFontMetrics metrics(painter.font());
        const QRectF textRect = metrics.boundingRect(lengthText);

        /// ЕСЛИ РАЗМЕР БОЛЬШЕ ЛИНИИ ВАЖНО!!!!
       // qDebug()<<Scaling::scaleCoordinate(length)<<" "<<textRect.width();
        if (textRect.width()+5 >= Scaling::scaleCoordinate(length)) {
            return false;
        }

        // Middle of the line
       // const QPointF mid((start + end) / 2);

        painter.save();
        painter.translate(mid);
        transform.translate(mid.x(), mid.y());


        QPointF lt = transform.map(QPointF(-5, -5));
        QPointF rt = transform.map(QPointF(textRect.width()+5, -5));
        QPointF lb = transform.map(QPointF(-5, textRect.height()+5));
        QPointF rb = transform.map(QPointF(textRect.width()+5, textRect.height()+5));


        QPointF offset;
        // The case of a vertical line
        if (angle==180 || angle == - 180) {
            const qreal offsetX = textRect.width() / 2 + style.label.offset.x();
            const qreal offsetY = textRect.height() / 2 + style.label.offset.y();
            offset = QPointF(offsetX, offsetY);


            transform.translate(offset.x(), offset.y());
            painter.translate(offset.x(), offset.y());


            transform.rotate(angle);
            painter.rotate(angle);  // Rotate the text by 90 degrees
        } else {
            // Calculating the angle of the line
            // Offset down from the line
            const qreal perpAngle = angle + 90.0;
            const qreal rad = qDegreesToRadians(perpAngle);
            const qreal offsetX = qCos(rad);
            const qreal offsetY = qSin(rad);

            painter.rotate(angle);
            transform.rotate(angle);

            offset = QPointF(offsetX - textRect.width() / 2, offsetY + textRect.height());

            painter.translate(offset.x(), offset.y());
            transform.translate(offset.x(), offset.y());

            QPen pen(ColorToQColor(style.label.color));
            pen.setWidth(style.label.size);
            painter.setPen(pen);
        }

        style.label.position.lt = Scaling::logic(lt);
        style.label.position.rt = Scaling::logic(rt);
        style.label.position.lb = Scaling::logic(lb);
        style.label.position.rb = Scaling::logic(rb);

        const QPointF lt1 = style.label.position.lt;
        const QPointF rt1 = style.label.position.rt;
        const QPointF lb1 = style.label.position.lb;
        const QPointF rb1 = style.label.position.rb;

       /* QPolygonF poly;
        poly << (lt1) << (rt1)
             << (rb1) << (lb1);
        QRectF rect1 = poly.boundingRect();*/

        if (!style.label.editing) {
            painter.drawText(0, 0, lengthText);
        } else {
            editCoordinateLine(painter, id, angle, lengthText, style);
        }

        painter.restore();
        return true;
    }


    bool drawLengthOverLine(QPainter& painter,ID id,const qreal angle,const qreal length,const QPointF& mid
            , LineLengthLabel& style) {
        QTransform transform;

        //const qreal dx = start.x() - end.x();
        //const qreal dy = start.y() - end.y();
        //const qreal length = Scaling::logic(qSqrt(dx * dx + dy * dy));

        const QString lengthText = QString::number(length, 'f', 2);
        const QFontMetrics metrics(painter.font());
        const QRectF textRect = metrics.boundingRect(lengthText);

        /// ЕСЛИ РАЗМЕР БОЛЬШЕ ЛИНИИ ВАЖНО!!!!
       // qDebug()<<Scaling::scaleCoordinate(length)<<" "<<textRect.width();
        if (textRect.width()+5 >= Scaling::scaleCoordinate(length)) {
            return false;
        }

        // Middle of the line
        // const QPointF mid((start + end) / 2);

        painter.save();
        painter.translate(mid);
        transform.translate(mid.x(), mid.y());


        QPointF lt = transform.map(QPointF(-5, -5));
        QPointF rt = transform.map(QPointF(textRect.width()+5, -5));
        QPointF lb = transform.map(QPointF(-5, textRect.height()+5));
        QPointF rb = transform.map(QPointF(textRect.width()+5, textRect.height()+5));


        QPointF offset;
        // The case of a vertical line
        if (angle==180 || angle == - 180) {
            const qreal offsetX = textRect.width() / 2 + style.label.offset.x();
            const qreal offsetY = -textRect.height() / 2 + style.label.offset.y();
            offset = QPointF(offsetX, offsetY);


            transform.translate(offset.x(), offset.y());
            painter.translate(offset.x(), offset.y());


            transform.rotate(angle);
            painter.rotate(angle);  // Rotate the text by 90 degrees
        } else {
            // Calculating the angle of the line


            // Offset down from the line
            const qreal perpAngle = angle + 90.0;
            const qreal rad = qDegreesToRadians(perpAngle);
            const qreal offsetX = qCos(rad);
            const qreal offsetY = qSin(rad);

            painter.rotate(angle);
            transform.rotate(angle);

            offset = QPointF(offsetX - textRect.width() / 2, offsetY - textRect.height()/2);

            painter.translate(offset.x(), offset.y());
            transform.translate(offset.x(), offset.y());

            QPen pen(ColorToQColor(style.label.color));
            pen.setWidth(style.label.size);
            painter.setPen(pen);
        }


        style.label.position.lt = Scaling::logic(lt);
        style.label.position.rt = Scaling::logic(rt);
        style.label.position.lb = Scaling::logic(lb);
        style.label.position.rb = Scaling::logic(rb);

        const QPointF lt1 = style.label.position.lt;
        const QPointF rt1 = style.label.position.rt;
        const QPointF lb1 = style.label.position.lb;
        const QPointF rb1 = style.label.position.rb;

//        QPolygonF poly;
//        poly << (lt1) << (rt1)
//             << (rb1) << (lb1);
//        QRectF rect1 = poly.boundingRect();

        //   qDebug()<<rect1;

        if (!style.label.editing) {
            painter.drawText(0, 0, lengthText);
        } else {
            editCoordinateLine(painter, id, angle, lengthText, style);
        }

        painter.restore();
        return true;
    }


    qreal calculateLength(const QPointF& start, const QPointF& end){
        const qreal dx = end.x() - start.x();
        const qreal dy = end.y() - start.y();
        qreal length = qSqrt(dx * dx + dy * dy);
        return length;
    }


    qreal calculateLength(const qreal dx, const qreal dy){
        qreal length = qSqrt(dx * dx + dy * dy);
        return length;
    }


    qreal calculateAngle(const QPointF& start, const QPointF& end){
        const qreal dx = start.x() - end.x();
        const qreal dy = start.y() - end.y();
        qreal angle = 0.0;
        if(dx==0) {
            angle = -90;
        }
        else {
            angle = qAtan2(dy, dx) * 180.0 / M_PI;

            // Check for the opposite direction
            if (angle > 90 || angle < -90) {
                angle += 180;
            }
        }
        return angle;

    }


    void drawGlow(QPainter& painter, const QPointF& start, const QPointF& end, const GlowStyle& style) {

        // Calculating the direction of the line
        const qreal dx = end.x() - start.x();
        const qreal dy = end.y() - start.y();
        qreal length = calculateLength(dx,dy);

        if (length == 0) {
            length = 1;
        }

        // Perpendicular vector
        const qreal perpX = -dy / length;
        const qreal perpY = dx / length;

        // Setting the distance for the gradient
        constexpr quint16 glowDistance = 10;

        // Defining the points of the beginning and end of the gradient
        const qreal gradStartX = start.x() + perpX * glowDistance;
        const qreal gradStartY = start.y() + perpY * glowDistance;
        const qreal gradEndX = start.x() - perpX * glowDistance;
        const qreal gradEndY = start.y() - perpY * glowDistance;

        painter.save();
        // Creating a linear gradient perpendicular to the line
        QLinearGradient gradient(gradStartX, gradStartY, gradEndX, gradEndY);
        setGradientColor(gradient, style.color);

        QPen glowPen;
        const quint16 SIZE_PEN = style.size;
        glowPen.setWidth(SIZE_PEN);
        glowPen.setBrush(gradient);
        glowPen.setCapStyle(Qt::RoundCap);
        painter.setPen(glowPen);

        painter.drawLine(start.x(), start.y(), end.x(), end.y());
        painter.restore();
    }


    inline void setGradientColor(QLinearGradient& gradient, const Color& color) {
        QColor baseColor = colorToGlow(color);
        baseColor.setAlpha(0);
        gradient.setColorAt(0.0, baseColor);
        baseColor.setAlpha(40);
        gradient.setColorAt(0.3, baseColor);
        baseColor.setAlpha(80);
        gradient.setColorAt(0.5, baseColor);
        baseColor.setAlpha(40);
        gradient.setColorAt(0.7, baseColor);
        baseColor.setAlpha(0);
        gradient.setColorAt(1.0, baseColor);
    }


    void drawID(QPainter& painter, const ID& sectionID,const qreal angle,const QPointF& mid,
                const IDLabelStyle& style) {
        if (!render::isValidID(sectionID)) {
            return;
        }

        // Delta for angle
        //const qreal dx = end.x() - start.x();
        //const qreal dy = end.y() - start.y();

        // Calculating the slope angle of the line
        //qreal angle =util::calculateAngle(start,end);// qAtan2(dy, dx) * 180.0 / M_PI;

        // If the line is in the opposite direction, flip the text
        /*if (angle > 90 || angle < -90) {
            angle += 180;
        }*/

        const QString idText = QStringLiteral("ID: %1").arg(sectionID.get());
        const QFontMetrics metrics(painter.font());
        const QRectF textRect = metrics.boundingRect(idText);

        // Offset from the line
        const quint16 FIX_OFFSET = style.label.size;
        const qreal rad = qDegreesToRadians(angle - 90.0);

        // Offset by a fixed distance from the line
        const qreal offsetX = FIX_OFFSET * qCos(rad) + style.label.offset.x();
        const qreal offsetY = FIX_OFFSET * qSin(rad) + style.label.offset.y();

        // Midpoint
       // const qreal midX = (start.x() + end.x()) / 2.0;
        //const qreal midY = (start.y() + end.y()) / 2.0;

        painter.save();
        painter.translate(mid);
        painter.rotate(angle);

        // Centering horizontally and vertically
        painter.translate(offsetX - textRect.width() / 2, offsetY - textRect.height() / 2);

        painter.setPen(ColorToQColor(style.label.color));
        painter.drawText(0, 0, idText);
        painter.restore();
    }


    void setupPen(QPainter& painter, const LineStyle& style) {

        QPen currentPen((ColorToQColor(style.figure.object.color)));
        const quint16 SIZE_PEN = style.figure.object.size;
        currentPen.setWidth(SIZE_PEN);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);
    }


    void drawArrow(QPainter& painter, const QPointF& from, const QPointF& to, LineStyle& style) {

        QLineF line(from, to);

        qreal arrowLength = 1;
        arrowLength = Scaling::scaleCoordinate(arrowLength);
        if (arrowLength > 10.0) {
            arrowLength = 10.0;
        }

        qreal angleDeg = 15.0;

        double angleRad = std::atan2(-line.dy(), line.dx());
        double angleOffset = qDegreesToRadians(angleDeg);

        QPointF arrowP1 = to - QPointF(std::cos(angleRad + angleOffset) * arrowLength,
                                       -std::sin(angleRad + angleOffset) * arrowLength);
        QPointF arrowP2 = to - QPointF(std::cos(angleRad - angleOffset) * arrowLength,
                                       -std::sin(angleRad - angleOffset) * arrowLength);

        if(style.serviceLine.lineStyle.glow.activity) {
            drawGlow(painter, to, arrowP1, style.serviceLine.lineStyle.glow);
            drawGlow(painter, to, arrowP2, style.serviceLine.lineStyle.glow);
        }

        painter.drawLine(to, arrowP1);
        painter.drawLine(to, arrowP2);
    }


    void drawLineWithLeftArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style) {
        painter.drawLine(start, end);
        drawArrow(painter, end, start, style); // begin Arrow
    }


    [[maybe_unused]] void drawLineWithRightArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style) {
        painter.drawLine(start, end);
        drawArrow(painter, start, end, style); // end Arrow
    }


    void drawLineWithLeftRightArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style) {
        QPen pen;
        pen.setWidth(1);
        pen.setColor(Qt::darkRed);
        painter.setPen(pen);

        if(style.serviceLine.lineStyle.glow.activity) {
            drawGlow(painter, start, end, style.serviceLine.lineStyle.glow);
        }

        painter.drawLine(start, end);

        QPointF copyStart=QPointF(start.x(),-start.y());
        QPointF copyEnd=QPointF(end.x(),-end.y());

        style.serviceLine.position=Scaling::logic(copyStart,copyEnd);

        drawArrow(painter, start, end, style); // begin
        drawArrow(painter, end, start, style); // end


    }

    void drawLengthLine(QPainter& painter,ID id, const QPointF& Start, const QPointF& End, LineStyle& style){
        // Вектор линии
        // это дельта от базовых координат на которую перемещает мышка линию со стрелочками (не стартовую)
        //style.serviceLine.offset.y()
        //style.serviceLine.offset.y()
        const QPointF mouseDeltaPx= Scaling::scaleCoordinate({style.serviceLine.offset.x(),-style.serviceLine.offset.y()});
        QPointF start = Start;
        QPointF end   = End;

        qreal dx = end.x() - start.x();
        qreal dy = end.y() - start.y();

        // Перпендикуляр (нормаль)
        QPointF normal(-dy, dx);
        qreal length = std::hypot(normal.x(), normal.y());
        if (length == 0) return;
        normal /= length;

        qreal lenDev = std::hypot(normal.x(), normal.y());
        if (lenDev == 0) return;
        normal /= lenDev;

        qreal proj = mouseDeltaPx.x() * normal.x() + mouseDeltaPx.y() * normal.y();
        QPointF offset = normal * proj;
        offset += normal * Scaling::scaleCoordinate(0.7);

        QPointF startPerp = start + offset;
        QPointF endPerp   = end   + offset;


        const qreal angle = util::calculateAngle(start, end);
        const QPointF mid((startPerp + endPerp) / 2);
        const qreal Length = Scaling::logic(util::calculateLength(start,end));
        bool draw = true;

        if (offset.x() < 0 && offset.y() < 0) {
            // линия вправо-вниз
            draw=drawLengthOverLine(painter, id, angle, Length, mid, style.lengthLabel);
        }
        else if (offset.x() > 0 && offset.y() > 0) {
            // линия вправо-вверх
            draw=drawLengthUnderLine(painter, id, angle, Length, mid, style.lengthLabel);
        }
        else if (offset.x() > 0 && offset.y() < 0) {
            // линия влево-вверх
            draw=drawLengthOverLine(painter, id, angle, Length, mid, style.lengthLabel);
        }
        else if (offset.x() < 0 && offset.y() > 0) {
            // линия влево-вниз
            draw=drawLengthUnderLine(painter, id, angle, Length, mid, style.lengthLabel);
        }
        else if (offset.y() == 0 && offset.x() > 0) {
            // вертикальная вверх
            draw=drawLengthUnderLine(painter, id, angle, Length, mid, style.lengthLabel);
        }
        else if (offset.y() == 0 && offset.x() < 0) {
            // вертикальная вниз
            draw=drawLengthOverLine(painter, id, angle, Length, mid, style.lengthLabel);
        }
        else if (offset.x() == 0 && offset.y() > 0) {
            // горизонтальная вправо
            draw=drawLengthUnderLine(painter, id, angle, Length, mid, style.lengthLabel);
        }
        else if (offset.x() == 0 && offset.y() < 0) {
            // горизонтальная влево
            draw=drawLengthOverLine(painter, id, angle, Length, mid, style.lengthLabel);
        }

        if(!draw){
            return;
        }

        QPen pen;
        pen.setWidth(1);
        pen.setColor(Qt::darkRed);
        painter.setPen(pen);
        painter.drawLine(start, startPerp);
        painter.drawLine(end,   endPerp);

        drawLineWithLeftRightArrow(painter, startPerp, endPerp, style);

    }
}


namespace render {
    void drawFigure(QPainter& painter, const QPointF beg, const QPointF end, LineStyle& style) {

        const QPointF start(Scaling::scaleCoordinate(beg.x()), Scaling::scaleCoordinate(-beg.y()));
        const QPointF endLogic(Scaling::scaleCoordinate(end.x()), Scaling::scaleCoordinate(-end.y()));

        util::setupPen(painter, style);
        painter.drawLine(start, endLogic);

        const qreal angle=util::calculateAngle(start,endLogic);
        const QPointF mid((start + endLogic) / 2);
        const qreal length = Scaling::logic(util::calculateLength(start,endLogic));
        util::drawLengthUnderLine(painter,ID(0),angle,length,mid, style.lengthLabel);
    }


    void drawFigures(QPainter& painter, std::unordered_map<ID, lineShell>& sections) {
        if (sections.empty()) {
            return;
        }

        const LineStyle defaultStyle;

        for (auto& elem: sections) {
            const Section* section = elem.second.object;

            const QPointF start(Scaling::scaleCoordinate(section->beg->x),
                                Scaling::scaleCoordinate(-section->beg->y));

            const QPointF end(Scaling::scaleCoordinate(section->end->x),
                              Scaling::scaleCoordinate(-section->end->y));


            if (!elem.second.style) {
                qWarning() << "Line" << elem.first.get() << "has null style!";
                continue;
            }

            LineStyle& style = *elem.second.style;

            util::setupPen(painter, style);
            painter.drawLine(start, end);


            if(style.serviceLine.active) {
                const ID& id = elem.first;
                util::drawLengthLine(painter,id, start, end, style);
            }

            if (style.figure.glow.activity) {
                util::drawGlow(painter, start, end, style.figure.glow);
            }
            if (style.figure.id.activity) {
                const ID& id = elem.first;
                const qreal angle = util::calculateAngle(start, end);
                const QPointF mid((start + end) / 2);
                util::drawID(painter, id, angle, mid, style.figure.id);
            }
        }

    }


    void drawScaleArrow(QPainter& painter, const QPointF& from, const QPointF& to, LineStyle& style) {
        QPointF ScaleFrom(Scaling::scaleCoordinate(from.x()), Scaling::scaleCoordinate(-from.y()));
        QPointF ScaleTo(Scaling::scaleCoordinate(to.x()), Scaling::scaleCoordinate(-to.y()));
        util::drawArrow(painter,ScaleFrom,ScaleTo,style);
    }


    [[maybe_unused]] void drawScaleLineWithLeftArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style) {
        drawFigure(painter, start, end, style);
        drawScaleArrow(painter, end, start, style); // begin Arrow
    }


    [[maybe_unused]] void drawScaleLineWithRightArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style) {
        drawFigure(painter, start, end, style);
        drawScaleArrow(painter, start, end, style); // end Arrow
    }


    void drawScaleLineWithLeftRightArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style) {
        drawFigure(painter, start, end, style);
        drawScaleArrow(painter, start, end, style); // begin
        drawScaleArrow(painter, end, start, style); // end
    }


    [[maybe_unused]] void drawScaleLengthLine(QPainter& painter,ID id, const QPointF& Start, const QPointF& End, LineStyle& style){
        const QPointF start(Scaling::scaleCoordinate(Start.x()), Scaling::scaleCoordinate(-Start.y()));
        const QPointF end(Scaling::scaleCoordinate(End.x()), Scaling::scaleCoordinate(-End.y()));

        util::drawLengthLine(painter,id,start,end,style);
    }

}