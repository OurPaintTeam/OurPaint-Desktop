#ifndef OURPAINT_DRAWINGLINE_H
#define OURPAINT_DRAWINGLINE_H

#include <QPainter>
#include <QPointF>
#include <QLineEdit>

#include "Scaling.h"
#include "RenderBaseFigures.h"
#include "Colors.h"
#include "ID.h"
#include "GeometricObjects.h"
#include "RenderStyle.h"

// --- Sections --- //
namespace render::util{

    /// This class serve to emit a signal in a static class

    class RenderLinesEmitter : public QObject {
    Q_OBJECT
    private:
        RenderLinesEmitter() = default;

    public:
        static RenderLinesEmitter* instance() {
            static RenderLinesEmitter inst;
            return &inst;
        }

    signals:
        void linesUpdated(ID id,const qreal length);

    };

    qreal calculateLength(const QPointF& start, const QPointF& end);
    qreal calculateLength(const qreal dx, const qreal dy);

    inline void setGradientColor(QLinearGradient& gradient, const Color& color);

    void drawGlow(QPainter& painter, const QPointF& start, const QPointF& end,const GlowStyle& style);

    void drawID(QPainter& painter, const ID& sectionID,const qreal angle,const QPointF& mid,
                const IDLabelStyle& style);

    bool drawLengthOverLine(QPainter& painter,ID id,const qreal angle,const qreal length,const QPointF& mid, LineLengthLabel& style);
    bool drawLengthUnderLine(QPainter& painter,ID id,const qreal angle,const qreal length,const QPointF& mid, LineLengthLabel& style);

    void editCoordinateLine(QPainter& painter,ID id, const qreal& angle,const QString& lengthText,LineLengthLabel& style);

    qreal calculateAngle(const QPointF& start, const QPointF& end);

    void drawArrow(QPainter& painter, const QPointF& from, const QPointF& to, LineStyle& style);
    void setupPen(QPainter& painter, const LineStyle& style);

    [[maybe_unused]] void drawLineWithRightArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style);
    void drawLineWithLeftRightArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style);
    void drawLengthLine(QPainter& painter,ID id, const QPointF& start, const QPointF& end, LineStyle& style);

}

namespace render {

    struct lineShell {
        Section* object;
        LineStyle* style;
    };

    inline void notifyLinesUpdated(ID id,const qreal length) {
        emit util::RenderLinesEmitter::instance()->linesUpdated(id,length);
    }

     void drawFigures(QPainter& painter,  std::unordered_map<ID, lineShell>& sections);
     void drawFigure(QPainter &painter,const QPointF beg,const QPointF end, LineStyle& style);

    void drawScaleArrow(QPainter& painter, const QPointF& from, const QPointF& to, LineStyle& style);
    [[maybe_unused]] void drawScaleLineWithLeftArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style);
    [[maybe_unused]] void drawScaleLineWithRightArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style);
    void drawScaleLineWithLeftRightArrow(QPainter& painter, const QPointF& start, const QPointF& end, LineStyle& style);
    [[maybe_unused]] void drawScaleLengthLine(QPainter& painter,ID id, const QPointF& Start, const QPointF& End, LineStyle& style);

}

#endif //OURPAINT_DRAWINGLINE_H
