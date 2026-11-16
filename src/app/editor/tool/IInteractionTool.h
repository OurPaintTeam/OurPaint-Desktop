#ifndef OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
#define OURPAINT_APPLICATION_IINTERACTIONTOOL_H_

#include "InputEvents.h"
#include "OverlayModel.h"
#include "SnapFeedback.h"
#include "SnapSystem.h"
#include "Document.h"

namespace core
{
    class Document;
}

class Cpu2dPicker;

namespace render
{
    class Camera2D;
}

class OverlayModel;

namespace snap
{
    class SnapSystem;
}

class IInteractionTool
{
protected:
    core::Document& document_;
    render::Camera2D& camera_;
    snap::SnapSystem& snap_;
    OverlayModel& overlay_;
    Cpu2dPicker& picker_;
    std::vector<snap::SnapResult> snap_results_;

    virtual void snapVisualClear()
    {
        snap_results_.clear();
        overlay_.clearSnap();
    }

    virtual void snapVisual(const snap::SnapRequest& request)
    {
        snapVisualClear();

        const auto results = snap_.getSnapCandidates(request);

        if (results.empty())
        {
            return;
        }

        snap_results_ = results;

        app::SnapFeedback feedback;

        for (const auto& snap_result : results)
        {
            feedback.position = {snap_result.point.position.x, snap_result.point.position.y};

            switch (snap_result.type)
            {
            case snap::SnapKind::None:
                {
                }
                break;
            case snap::SnapKind::Collinear:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.sources = {snap_result.objects};
                if (!snap_result.guideLines.empty())
                {
                    feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                }
                break;

            case snap::SnapKind::Coincident:
                {
                    const auto sub = snap_result.objects.at(0).sub;
                    if (sub == core::sketch::SubElement::Center)
                    {
                        feedback.glyph = app::SnapGlyph::Center;
                    }
                    else if (sub == core::sketch::SubElement::Start || sub == core::sketch::SubElement::End)
                    {
                        feedback.glyph = app::SnapGlyph::Endpoint;
                    }
                    else if (sub == core::sketch::SubElement::Whole)
                    {
                        feedback.glyph = app::SnapGlyph::Point;
                    }
                    else
                    {
                        feedback.glyph = app::SnapGlyph::Point;
                    }
                    feedback.sources = {snap_result.objects};
                    break;
                }

            case snap::SnapKind::Midpoint:
                feedback.glyph = app::SnapGlyph::Midpoint;
                feedback.sources = {snap_result.objects};
                break;

            case snap::SnapKind::Intersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {snap_result.objects};
                break;

            case snap::SnapKind::ExtendedIntersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {snap_result.objects};
                if (!snap_result.guideLines.empty())
                {
                    feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                }
                break;

            case snap::SnapKind::VirtualIntersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {snap_result.objects};
                if (snap_result.guideLines.size() >= 2)
                {
                    feedback.guides = {
                        {snap_result.guideLines.at(0), app::SnapGuideExtent::Segment},
                        {snap_result.guideLines.at(1), app::SnapGuideExtent::Segment}
                    };
                }
                break;

            case snap::SnapKind::Parallel:
            case snap::SnapKind::Perpendicular:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.sources = {snap_result.objects};
                if (!snap_result.guideLines.empty())
                {
                    feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                }
                break;

            case snap::SnapKind::Horizontal:
            case snap::SnapKind::Vertical:
                feedback.glyph = app::SnapGlyph::Direction;
                if (!snap_result.guideLines.empty())
                {
                    feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                }
                break;

            case snap::SnapKind::Tangent:
                feedback.glyph = app::SnapGlyph::Point;
                feedback.sources = {snap_result.objects};
                if (!snap_result.guideLines.empty())
                {
                    feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                }
                break;

            case snap::SnapKind::AngleToLine:
                feedback.glyph = app::SnapGlyph::Point;
                feedback.sources = {snap_result.objects};
                if (!snap_result.guideLines.empty())
                {
                    feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                }
                break;

            default:
                overlay_.clearSnap();
                return;
            }
        }

        overlay_.setSnapFeedback(std::move(feedback));
    }

    virtual bool applySnapConstraint(const snap::SnapResult& snap, const core::sketch::GeometryRef& target)
    {
        using SK = snap::SnapKind;
        using CT = core::sketch::ConstraintType;
        using SE = core::sketch::SubElement;

        const bool isPointRef = (target.sub == SE::Start || target.sub == SE::End || target.sub == SE::Center);

        core::sketch::GeometryRef bodyRef = target;
        bodyRef.sub = SE::Whole;

        core::sketch::ConstraintDefinition def;

        switch (snap.type)
        {
        case SK::Coincident:
        case SK::Intersection:
        case SK::ExtendedIntersection:
        case SK::VirtualIntersection:
            {
                if (snap.objects.empty()) return false;
                def.type = CT::Coincident;
                def.refs = {target, snap.objects[0]};
                break;
            }

        case SK::Midpoint:
            {
                if (snap.objects.empty()) return false;

                if (!isPointRef) return false;
                def.type = CT::Midpoint;
                def.refs = {target, snap.objects[0]};
                break;
            }

        case SK::Tangent:
            {
                if (snap.objects.empty()) return false;

                def.type = CT::Tangent;
                def.refs = {bodyRef, snap.objects[0]};
                break;
            }

        case SK::Horizontal:
            {
                def.type = CT::Horizontal;
                def.refs = {bodyRef};
                break;
            }

        case SK::Vertical:
            {
                def.type = CT::Vertical;
                def.refs = {bodyRef};
                break;
            }

        case SK::Parallel:
            {
                if (snap.objects.empty()) return false;

                def.type = CT::Parallel;
                def.refs = {bodyRef, snap.objects[0]};
                break;
            }

        case SK::Perpendicular:
            {
                if (snap.objects.empty()) return false;

                def.type = CT::Perpendicular;
                def.refs = {bodyRef, snap.objects[0]};
                break;
            }

        default:
            return false;
        }

        if (!document_.sketch().supportsConstraint(def)) return false;
        auto c = document_.sketch().addConstraint(def);
        return static_cast<bool>(c);
    }

public:
    IInteractionTool(core::Document& document, render::Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay,
                     Cpu2dPicker& picker_)
        : document_(document), camera_(camera), snap_(snap), overlay_(overlay), picker_(picker_)
    {
    }

    virtual ~IInteractionTool()
    {
    }

    virtual void onMouseMove(const input::MouseMoveEvent& e) = 0;
    virtual void onMouseButton(const input::MouseButtonEvent& e) = 0;
    virtual void onKey(const input::KeyEvent& e) = 0;

    // true => cancel handled by tool
    // false => nothing to cancel, caller may switch tool
    virtual bool cancel() = 0;
};

#endif  // ! OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
