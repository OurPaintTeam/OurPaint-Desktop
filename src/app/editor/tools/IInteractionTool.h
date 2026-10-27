#ifndef OURPAINT_IINTERACTION_TOOL_H_
#define OURPAINT_IINTERACTION_TOOL_H_

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
            case snap::SnapKind::Collinear:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.sources = {snap_result.objects};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                break;

            case snap::SnapKind::Coincident: {
                if (snap_result.objects.at(0).sub == core::sketch::SubElement::Center) {
                    feedback.glyph = app::SnapGlyph::Center;
                } else if (snap_result.objects.at(0).sub == core::sketch::SubElement::Start || snap_result.objects.at(0).sub == core::sketch::SubElement::End) {
                    feedback.glyph = app::SnapGlyph::Endpoint;
                } else if (snap_result.objects.at(0).sub == core::sketch::SubElement::Whole) {
                    feedback.glyph = app::SnapGlyph::Point;
                }

                feedback.sources = {snap_result.objects};

                for (const auto& rel : snap_result.relations) {
                    feedback.sources.insert(feedback.sources.end(), rel.objects.begin(), rel.objects.end());

                    switch (rel.type) {
                        case snap::SnapKind::Parallel:
                        case snap::SnapKind::Perpendicular:
                            if (!rel.guideLines.empty()) {
                                feedback.guides.push_back({rel.guideLines.at(0), app::SnapGuideExtent::Segment});
                            }
                            break;

                        case snap::SnapKind::Horizontal:
                        case snap::SnapKind::Vertical:
                            if (!rel.guideLines.empty()) {
                                feedback.guides.push_back({rel.guideLines.at(0), app::SnapGuideExtent::InfiniteLine});
                            }
                            break;

                        case snap::SnapKind::Collinear:
                            if (!rel.guideLines.empty()) {
                                feedback.guides.push_back({rel.guideLines.at(0), app::SnapGuideExtent::InfiniteLine});
                            }
                            break;

                        case snap::SnapKind::Tangent:
                            if (!rel.guideLines.empty()) {
                                feedback.guides.push_back({rel.guideLines.at(0), app::SnapGuideExtent::InfiniteLine});
                            }
                            break;

                        default:
                            break;
                    }
                }
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
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::VirtualIntersection:
                feedback.glyph = app::SnapGlyph::Intersection;
                feedback.sources = {snap_result.objects};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment},
                                   {snap_result.guideLines.at(1), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::Parallel:
            case snap::SnapKind::Perpendicular:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.sources = {snap_result.objects};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::Horizontal:
            case snap::SnapKind::Vertical:
                feedback.glyph = app::SnapGlyph::Direction;
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                break;

            case snap::SnapKind::Tangent:
                feedback.glyph = app::SnapGlyph::Point;
                feedback.sources = {snap_result.objects};
                if (!snap_result.guideLines.empty()) {
                    feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::InfiniteLine}};
                }
                break;

            case snap::SnapKind::AngleToLine:
                feedback.glyph = app::SnapGlyph::Point;
                feedback.sources = {snap_result.objects};
                feedback.guides = {{snap_result.guideLines.at(0), app::SnapGuideExtent::Segment}};
                break;

            case snap::SnapKind::PerpendicularBisector:
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

    // Pending input is discarded, not rolled back. A gesture may return its
    // outcome here; handled=false allows the editor to exit an empty tool.
    virtual ToolCancellation cancel() = 0;
};

#endif  // ! OURPAINT_IINTERACTION_TOOL_H_
