#ifndef OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
#define OURPAINT_APPLICATION_IINTERACTIONTOOL_H_

#include "../../platform/InputEvents.h"
#include "../ActionReport.h"
#include "OverlayModel.h"
#include "SnapFeedback.h"
#include "snapping/SnapSystem.h"

class Cpu2dPicker;
class Document;
class Camera2D;
class OverlayModel;
namespace snap {
class SnapSystem;
}

class IInteractionTool {
protected:
    Document& document_;
    Camera2D& camera_;
    snap::SnapSystem& snap_;
    OverlayModel& overlay_;
    Cpu2dPicker& picker_;
    snap::SnapResult snap_result_;


    virtual void snapVisual(const snap::SnapRequest& request) {
        const auto snap_result = snap_.getSnapCandidate(request);
        snap_result_ = snap_result;

        if (!snap_result.snapped) {
            overlay_.clearSnap();
            return;
        }

        app::SnapFeedback feedback;
        feedback.position = {snap_result.point.position.x, snap_result.point.position.y};

        switch (snap_result.type) {
            case snap::SnapKind::StandalonePoint:
            case snap::SnapKind::LineBody:
            case snap::SnapKind::CircleBody:
            case snap::SnapKind::ArcBody:
                feedback.glyph = app::SnapGlyph::Point;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub}};
                break;

            case snap::SnapKind::LineEndpoint:
            case snap::SnapKind::ArcEndpoint:
                feedback.glyph = app::SnapGlyph::Endpoint;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub}};
                break;

            case snap::SnapKind::LineMidpoint:
                feedback.glyph = app::SnapGlyph::Midpoint;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub}};
                break;

            case snap::SnapKind::CircleCentre:
            case snap::SnapKind::ArcCentre:
                feedback.glyph = app::SnapGlyph::Center;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub}};
                break;

            case snap::SnapKind::Intersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub},
                                    {snap_result.objects.at(1).entity, snap_result.objects.at(1).sub}};
                break;

            case snap::SnapKind::ExtendedIntersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub},
                                    {snap_result.objects.at(1).entity, snap_result.objects.at(1).sub}};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::VirtualIntersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub},
                                    {snap_result.objects.at(1).entity, snap_result.objects.at(1).sub}};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment},
                                   {snap_result.guideLines.at(1), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::Parallel:
            case snap::SnapKind::Perpendicular:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub}};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::Horizontal:
            case snap::SnapKind::Vertical:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                break;

            case snap::SnapKind::Tangent:
                feedback.glyph = app::SnapGlyph::Point;
                feedback.sources = {{snap_result.objects.at(0).entity, snap_result.objects.at(0).sub}};
                if (!snap_result.guideLines.empty())
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                break;

            default:
                overlay_.clearSnap();
                return;
        }

        overlay_.setSnapFeedback(std::move(feedback));
    }

public:
    IInteractionTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker_)
        : document_(document), camera_(camera), snap_(snap), overlay_(overlay), picker_(picker_) {}

    virtual ~IInteractionTool() {}

    virtual std::optional<ActionReport> onMouseMove(const input::MouseMoveEvent& e) = 0;
    virtual std::optional<ActionReport> onMouseButton(const input::MouseButtonEvent& e) = 0;
    virtual std::optional<ActionReport> onKey(const input::KeyEvent& e) = 0;

    // Pending input is discarded. Geometry edits already applied to Sketch are
    // retained; presentation previews may restore their initial state. A gesture
    // may return its outcome; handled=false lets the editor exit an empty tool.
    virtual ToolCancellation cancel() = 0;
};

#endif  // ! OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
