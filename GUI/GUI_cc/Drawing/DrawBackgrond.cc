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


void DrawBackground::backgroundRender(QPainter &painter) {

    // To record coordinates for rendering
    QVector<QPointF> pointXR;
    QVector<QPointF> pointXL;
    QVector<QPointF> pointYU;
    QVector<QPointF> pointYD;

    const qreal screenHalfWidth = Scaling::getCenteredCoordinatesX();
    const qreal screenHalfHeight = Scaling::getCenteredCoordinatesY();
    const qint16 screenHeight = Scaling::getActualMonitorHeight();

    const qreal deltaX = Scaling::getDeltaX();
    const qreal deltaY = Scaling::getDeltaY();

    const qreal zoom = Scaling::getZoom();

    constexpr qint16 SIZE = 1;
    const QPen darkPen(Qt::darkGray, SIZE);
    const QPen greyPen(Qt::lightGray, SIZE);

    // Rendering a checkered background
    if (ModeManager::getCell()) {

        const qreal absDeltaX =  qAbs(deltaX);
        const qreal absDeltaY = qAbs(deltaY);
        
        const qreal X_SIZE=screenHalfWidth + absDeltaX;
        const qreal Y_SIZE=screenHalfHeight + absDeltaY;

        const qreal Y_UPPER_LINE=-screenHalfHeight - absDeltaY;
        const qreal Y_LOWER_LINE=screenHalfHeight + absDeltaY;
        const qreal X_LEFT_LINE=-screenHalfWidth - absDeltaX;
        const qreal X_RIGHT_LINE=screenHalfWidth + absDeltaX;

        const qreal MIN_STEP = Scaling::getUserUnitSize();
        qreal stepLogical = Step(MIN_STEP / zoom);
        qreal currentCellSize = stepLogical * zoom;

        qint16 index = 1; // Making blocks 5 by 5



        // Vertical lines
        for (qreal x = currentCellSize; x <= X_SIZE; x += currentCellSize,++index) {

            const bool IS_FIVES_LINES = (index % 5 == 0);
            painter.setPen(IS_FIVES_LINES ? darkPen : greyPen);

            if (IS_FIVES_LINES) {
                pointXR.emplace_back(x, 0);
                pointXL.emplace_back(-x, 0);
            }

            painter.drawLine(QPointF(x,Y_UPPER_LINE),
                             QPointF(x, Y_LOWER_LINE));
            painter.drawLine(QPointF(-x, Y_UPPER_LINE),
                             QPointF(-x, Y_LOWER_LINE));
        }

        // Horizontal lines
        index = 1;
        for (qreal y = currentCellSize; y <= Y_SIZE; y += currentCellSize,++index) {

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
    } else if(ModeManager::getCell()){
        painter.setPen(greyPen);

        // Ox
        painter.drawLine(QPointF((-screenHalfWidth - deltaX), 0),
                         QPointF((screenHalfWidth - deltaX), 0));

        // Oy
        painter.drawLine(QPointF(0, (-screenHalfHeight - deltaY)),
                         QPointF(0, (screenHeight - deltaY)));
    }
       

}

void DrawBackground::mainBackgroundRender(QPainter &painter){
    const qreal screenHalfWidth = Scaling::getCenteredCoordinatesX();
    const qreal screenHalfHeight = Scaling::getCenteredCoordinatesY();
    const qint16 screenHeight = Scaling::getActualMonitorHeight();

    const qreal deltaX = Scaling::getDeltaX();
    const qreal deltaY = Scaling::getDeltaY();
    const qreal absDeltaX =  qAbs(deltaX);
    const qreal absDeltaY = qAbs(deltaY);


    // Drawing the vertical Oy axis
    if (screenHalfWidth <= absDeltaX) {
        if (deltaX > 0) {
            // Far right
            // Oy
            painter.drawLine(QPointF(screenHalfWidth - 1 - deltaX, -screenHalfHeight - deltaY),
                             QPointF(screenHalfWidth - 1 - deltaX, screenHeight - deltaY));
        } else {
            // Far left
            // Oy
            painter.drawLine(QPointF(-screenHalfWidth + 1 - deltaX, -screenHalfHeight - deltaY),
                             QPointF(-screenHalfWidth + 1 - deltaX, screenHeight - deltaY));
        }
    } else {
        painter.drawLine(QPointF(0, (-screenHalfHeight - deltaY)),
                         QPointF(0, (screenHeight - deltaY)));  // Oy
    }


    // Drawing the horizontal Ox axis
    if (screenHalfHeight <= absDeltaY) {
        if (deltaY > 0) {
            // Lower
            // Ox
            painter.drawLine(QPointF(-screenHalfWidth - deltaX, screenHalfHeight - 1 - deltaY),
                             QPointF(screenHalfWidth - deltaX, screenHalfHeight - 1 - deltaY));
        } else {
            // Upper
            // Ox
            painter.drawLine(QPointF(-screenHalfWidth - deltaX, -screenHalfHeight - deltaY),
                             QPointF(screenHalfWidth - deltaX, -screenHalfHeight - deltaY));
        }
    } else {
        // Ox
        painter.drawLine(QPointF((-screenHalfWidth - deltaX), 0),
                         QPointF((screenHalfWidth - deltaX), 0));
    } 
    
}