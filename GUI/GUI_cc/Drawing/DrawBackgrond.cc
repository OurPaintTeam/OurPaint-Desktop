#include "DrawBackground.h"

qreal DrawBackground::Step(qreal rawStep) {
    qreal exp = qFloor(qLn(rawStep) / qLn(10.0));
    qreal fraction = rawStep / qPow(10.0, exp);
    qreal newFraction;

    if (fraction < 1.5) { newFraction = 1; }
    else if (fraction < 2.5) { newFraction = 2; }
    else if (fraction < 3.5) { newFraction = 2.5; }
    else if (fraction < 5.0) { newFraction = 4; }
    else if (fraction < 7.5) { newFraction = 5; }
    else { newFraction = 10; }

    return newFraction * qPow(10.0, exp);
}


void DrawBackground::backgroundRender(QPainter& painter) {

    // To record coordinates for rendering
    QVector<QPointF> pointXR;
    QVector<QPointF> pointXL;
    QVector<QPointF> pointYU;
    QVector<QPointF> pointYD;

    const QPointF delta = Scaling::getDelta();
    const QPointF centredMonitor = Scaling::getCenteredCoordinates();

    const qint16 screenHeight = Scaling::getActualMonitorHeight();

    const qreal zoom = Scaling::getZoom();

    constexpr qint16 SIZE = 1;
    const QPen darkPen(Qt::darkGray, SIZE);
    const QPen greyPen(Qt::lightGray, SIZE);

    // Rendering a checkered background
    if (ModeManager::getCell()) {

        const QPointF absDelta = {qAbs(delta.x()), qAbs(delta.y())};

        const qreal X_SIZE = centredMonitor.x() + absDelta.x();
        const qreal Y_SIZE = centredMonitor.y() + absDelta.y();

        const qreal Y_UPPER_LINE = -centredMonitor.y() - absDelta.y();
        const qreal Y_LOWER_LINE = centredMonitor.y() + absDelta.y();
        const qreal X_LEFT_LINE = -centredMonitor.x() - absDelta.x();
        const qreal X_RIGHT_LINE = centredMonitor.x() + absDelta.x();

        const qreal MIN_STEP = Scaling::getUserUnitSize();
        qreal stepLogical = Step(MIN_STEP / zoom);
        qreal currentCellSize = stepLogical * zoom;

        qint16 index = 1; // Making blocks 5 by 5


        // Vertical lines
        for (qreal x = currentCellSize; x <= X_SIZE; x += currentCellSize, ++index) {

            const bool IS_FIVES_LINES = (index % 5 == 0);
            painter.setPen(IS_FIVES_LINES ? darkPen : greyPen);

            if (IS_FIVES_LINES) {
                pointXR.emplace_back(x, 0);
                pointXL.emplace_back(-x, 0);
            }

            painter.drawLine(QPointF(x, Y_UPPER_LINE),
                             QPointF(x, Y_LOWER_LINE));
            painter.drawLine(QPointF(-x, Y_UPPER_LINE),
                             QPointF(-x, Y_LOWER_LINE));
        }

        // Horizontal lines
        index = 1;
        for (qreal y = currentCellSize; y <= Y_SIZE; y += currentCellSize, ++index) {

            const bool IS_FIVES_LINES = (index % 5 == 0);
            painter.setPen(IS_FIVES_LINES ? darkPen : greyPen);

            if (IS_FIVES_LINES) {
                pointYU.emplace_back(0, y);
                pointYD.emplace_back(0, -y);
            }

            painter.drawLine(QPointF((X_LEFT_LINE), y),
                             QPointF(X_RIGHT_LINE, y));
            painter.drawLine(QPointF((X_LEFT_LINE), -y),
                             QPointF(X_RIGHT_LINE, -y));
        }

    }

    // Coordinate axes
    if (ModeManager::getAxis()) {

        painter.setPen(darkPen);
        // Rendering values
        DrawAdditionalInf::drawCoordinateLabels(painter, pointXL, pointXR, pointYU, pointYD);

        // If the axis becomes invisible, we draw it on the border
        DrawBackground::mainBackgroundRender(painter);
    } else if (ModeManager::getCell()) {
        painter.setPen(greyPen);

        // Ox
        painter.drawLine(QPointF((-centredMonitor.x() - delta.x()), 0),
                         QPointF((centredMonitor.x() - delta.x()), 0));

        // Oy
        painter.drawLine(QPointF(0, (-centredMonitor.y() - delta.y())),
                         QPointF(0, (screenHeight - delta.y())));
    }


}

void DrawBackground::mainBackgroundRender(QPainter& painter) {
    const QPointF delta = Scaling::getDelta();
    const QPointF centredMonitor = Scaling::getCenteredCoordinates();
    const QPointF absDelta = {qAbs(delta.x()), qAbs(delta.y())};

    const qint16 screenHeight = Scaling::getActualMonitorHeight();

    // Drawing the vertical Oy axis
    if (centredMonitor.x() <= absDelta.x()) {
        if (delta.x() > 0) {
            // Far right
            // Oy
            painter.drawLine(QPointF(centredMonitor.x() - 1 - delta.x(), -centredMonitor.y() - delta.y()),
                             QPointF(centredMonitor.x() - 1 - delta.x(), screenHeight - delta.y()));
        } else {
            // Far left
            // Oy
            painter.drawLine(QPointF(-centredMonitor.x() + 1 - delta.x(), -centredMonitor.y() - delta.y()),
                             QPointF(-centredMonitor.x() + 1 - delta.x(), screenHeight - delta.y()));
        }
    } else {
        painter.drawLine(QPointF(0, (-centredMonitor.y() - delta.y())),
                         QPointF(0, (screenHeight - delta.y())));  // Oy
    }


    // Drawing the horizontal Ox axis
    if (centredMonitor.y() <= absDelta.y()) {
        if (delta.y() > 0) {
            // Lower
            // Ox
            painter.drawLine(QPointF(-centredMonitor.x() - delta.x(), centredMonitor.y() - 1 - delta.y()),
                             QPointF(centredMonitor.x() - delta.x(), centredMonitor.y() - 1 - delta.y()));
        } else {
            // Upper
            // Ox
            painter.drawLine(QPointF(-centredMonitor.x() - delta.x(), -centredMonitor.y() - delta.y()),
                             QPointF(centredMonitor.x() - delta.x(), -centredMonitor.y() - delta.y()));
        }
    } else {
        // Ox
        painter.drawLine(QPointF((-centredMonitor.x() - delta.x()), 0),
                         QPointF((centredMonitor.x() - delta.x()), 0));
    }

}