#include "DrawAdditionalInf.h"
#include "RenderStyle.h"


void DrawAdditionalInf::drawCoordinateLabels(QPainter& painter,
                                             const QVector<QPointF>& pointsXRight,
                                             const QVector<QPointF>& pointsXLeft,
                                             const QVector<QPointF>& pointsYUp,
                                             const QVector<QPointF>& pointsYDown) {
    constexpr quint16 FontSize = 8;
    constexpr quint16 MARGIN = 3;

    QFont font = painter.font();
    font.setPointSize(FontSize);
    painter.setFont(font);

    QFontMetrics fontMetrics(font);
    const quint16 TEXT_HEIGHT = fontMetrics.height();
    const quint16 MARGIN_HEIGHT = TEXT_HEIGHT - MARGIN;

    painter.setPen(Qt::black);

    constexpr quint16 PRECISION= 1;
    auto drawLabel = [&](const QPointF& point, const qreal value,const bool below) {
        QPointF textPos = point;
        textPos.setX(point.x() + MARGIN);
        textPos.setY(point.y() + (below ? MARGIN_HEIGHT : -MARGIN));
        painter.drawText(textPos, QString::number(value, 'f', PRECISION));
    };

    // Draw X-axis labels
    for (const QPointF& point: pointsXRight) {
        drawLabel(point, Scaling::logic(point.x()), true);
    }

    for (const QPointF& point: pointsXLeft) {
        drawLabel(point, Scaling::logic(point.x()), true);
    }

    // Draw Y-axis labels
    for (const QPointF& point: pointsYDown) {
        drawLabel(point, Scaling::logic(-point.y()), false);
    }

    for (const QPointF& point: pointsYUp) {
        drawLabel(point, Scaling::logic(-point.y()), false);
    }
}


[[maybe_unused]]
void DrawAdditionalInf::drawAxes(QPainter& painter) {

    // Centering
    const QSizeF centerMonitor = Scaling::getCenteredCoordinates();

    // Offset
    const QPointF delta = Scaling::getDelta();

    // Logical coordinates
    const qreal logicRightX = Scaling::logic(centerMonitor.width()) - Scaling::logic(delta.x());
    const qreal logicUpperY = Scaling::logic(centerMonitor.height()) + Scaling::logic(delta.y());
    const qreal logicLeftX = -Scaling::logic(centerMonitor.width()) - Scaling::logic(delta.x());
    const qreal logicLowerY = -Scaling::logic(centerMonitor.height()) + Scaling::logic(delta.y());

    // Window dimensions
    const QSize windowSize = Scaling::getActualMonitorSize();

    // Font metrics
    QFontMetrics metrics(painter.font());
    const quint16 textHeight = metrics.height();

    constexpr quint16 MARGIN = 5;

    // Check axis visibility
    bool xAxisVisible = (centerMonitor.width() + delta.y() >= 0) && (centerMonitor.height() + delta.y() <= windowSize.height());
    bool yAxisVisible = (centerMonitor.width() + delta.x() >= 0) && (centerMonitor.height() + delta.x() <= windowSize.width());

    auto drawTextCorner = [&](const QString& text,const qint32 x,const qint32 y) {
        painter.drawText(x, y, text);
    };

    // Draw left X value
    if (logicLeftX < 0 && xAxisVisible) {
        QString text = QString::number(logicLeftX);
        const quint16 x = MARGIN;
        const qint32 y = static_cast<qint32>(centerMonitor.width()) + delta.y() - MARGIN;
        drawTextCorner(text, x, y);
    } else if (!xAxisVisible && qAbs(logicLeftX) > logicRightX) {
        const QString text = QString::number(logicLeftX);
        const qint32 y = (qAbs(logicUpperY) > qAbs(logicLowerY)) ? (windowSize.height() - MARGIN) : textHeight;
        drawTextCorner(text, MARGIN, y);
    }

    // Draw right X value
    if (logicRightX > 0 && xAxisVisible) {
        const QString text = QString::number(logicRightX);
        const quint16 textWidth = metrics.horizontalAdvance(text);
        const qint32 x = windowSize.height() - textWidth - MARGIN;
        const qint32 y = static_cast<qint32>(centerMonitor.height()) + delta.y() - MARGIN;
        drawTextCorner(text, x, y);
    } else if (!xAxisVisible && qAbs(logicRightX) > qAbs(logicLeftX)) {
        const QString text = QString::number(logicRightX);
        const quint16 textWidth = metrics.horizontalAdvance(text);
        const qint32 x = windowSize.width() - textWidth - MARGIN;
        const qint32 y = (qAbs(logicUpperY) > qAbs(logicLowerY)) ? (windowSize.height() - MARGIN) : textHeight;
        drawTextCorner(text, x, y);
    }

    // Draw upper Y value
    if (logicUpperY > 0 && yAxisVisible) {
        const QString text = QString::number(logicUpperY);
        const qint32 x = static_cast<qint32>(centerMonitor.width()) + delta.x() + MARGIN;
        drawTextCorner(text, x, textHeight);
    } else if (!yAxisVisible && qAbs(logicUpperY) > qAbs(logicLowerY)) {
        const QString text = QString::number(logicUpperY);
        const qint32 x = (qAbs(logicRightX) > qAbs(logicLeftX)) ? MARGIN : (windowSize.width() -
                                                                           metrics.horizontalAdvance(text) - MARGIN);
        drawTextCorner(text, x, textHeight);
    }

    // Draw lower Y value
    if (logicLowerY < 0 && yAxisVisible) {
        const QString text = QString::number(logicLowerY);
        const qint32 x = static_cast<qint32>(centerMonitor.width()) + delta.x() + MARGIN;
        const qint32 y = windowSize.height() - MARGIN;
        drawTextCorner(text, x, y);
    } else if (!yAxisVisible && qAbs(logicLowerY) > qAbs(logicUpperY)) {
        const QString text = QString::number(logicLowerY);
        const qint32 y = windowSize.height() - MARGIN;
        const qint32 x = (qAbs(logicRightX) < qAbs(logicLeftX))
                ? (windowSize.width() - metrics.horizontalAdvance(text) - MARGIN)
                : MARGIN;
        drawTextCorner(text, x, y);
    }
}


void DrawAdditionalInf::drawCursor(QPainter& painter) {
    const QPointF cursor=Scaling::scaleCursor();

    const QPointF logicCursor = Scaling::logicCursor();
    constexpr quint16 RANGE = 5;

    painter.setPen(Qt::black);

    constexpr quint16 PRECISION= 1;
    painter.drawText(QPointF(cursor.x() + RANGE, cursor.y() - RANGE),
                     QStringLiteral("X: %1, Y: %2").arg(logicCursor.x(), 0, 'f', PRECISION)
                             .arg(logicCursor.y(), 0, 'f', PRECISION));

}









