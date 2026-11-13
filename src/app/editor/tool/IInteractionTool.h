#ifndef OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
#define OURPAINT_APPLICATION_IINTERACTIONTOOL_H_

#include "../InputEvents.h"
#include "../OverlayModel.h"
#include "../render/SnapFeedback.h"
#include "snap/SnapSystem.h"

class Cpu2dPicker;
class Document;
namespace render {
class Camera2D;
}
class OverlayModel;
namespace snap {
class SnapSystem;
}

class IInteractionTool {
protected:
    Document& document_;
    render::Camera2D& camera_;
    snap::SnapSystem& snap_;
    OverlayModel& overlay_;
    Cpu2dPicker& picker_;
    snap::SnapResult snap_result_;

    void snapVisual(const snap::SnapRequest& request) {
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
                feedback.sources = {snap_result.objects.at(0)};
                break;

            case snap::SnapKind::LineEndpoint:
            case snap::SnapKind::ArcEndpoint:
                feedback.glyph = app::SnapGlyph::Endpoint;
                feedback.sources = {snap_result.objects.at(0)};
                break;

            case snap::SnapKind::LineMidpoint:
                feedback.glyph = app::SnapGlyph::Midpoint;
                feedback.sources = {snap_result.objects.at(0)};
                break;

            case snap::SnapKind::CircleCentre:
            case snap::SnapKind::ArcCentre:
                feedback.glyph = app::SnapGlyph::Center;
                feedback.sources = {snap_result.objects.at(0)};
                break;

            case snap::SnapKind::Intersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {snap_result.objects.at(0), snap_result.objects.at(1)};
                break;

            case snap::SnapKind::ExtendedIntersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {snap_result.objects.at(0), snap_result.objects.at(1)};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::VirtualIntersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {snap_result.objects.at(0), snap_result.objects.at(1)};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment},
                                   {snap_result.guideLines.at(1), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::Parallel:
            case snap::SnapKind::Perpendicular:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.sources = {snap_result.objects.at(0)};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::Horizontal:
            case snap::SnapKind::Vertical:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                break;

            case snap::SnapKind::Tangent:
                feedback.glyph = app::SnapGlyph::Point;
                feedback.sources = {snap_result.objects.at(0)};
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
    IInteractionTool(Document& document, render::Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker_)
        : document_(document), camera_(camera), snap_(snap), overlay_(overlay), picker_(picker_) {}

    virtual ~IInteractionTool() {}

    virtual void onMouseMove(const input::MouseMoveEvent& e) = 0;
    virtual void onMouseButton(const input::MouseButtonEvent& e) = 0;
    virtual void onKey(const input::KeyEvent& e) = 0;

    // true => cancel handled by tool
    // false => nothing to cancel, caller may switch tool
    virtual bool cancel() = 0;
};

#endif  // ! OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
