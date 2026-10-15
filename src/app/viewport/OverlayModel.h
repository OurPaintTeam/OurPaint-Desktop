#ifndef OURPAINT_APPLICATION_OVERLAY_MODEL_H_
#define OURPAINT_APPLICATION_OVERLAY_MODEL_H_

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "SelectionModel.h"
#include "SnapFeedback.h"

class OverlayModel {
public:
    enum class OverlayType {
        None,
        Preview,
        Selection,
        Snap,
    };

    struct Color {
        float r{};
        float g{};
        float b{};
        float a{};
    };

    struct Rect {
        float xMin{};
        float yMin{};
        float xMax{};
        float yMax{};
        OverlayType type;
        Color color;
    };

    struct Point {
        float x{};
        float y{};
        OverlayType type;
        Color color;
    };


    struct Circle {
        float cx{};
        float cy{};
        float r{};
        OverlayType type;
        Color color;
    };

    struct Line {
        float x1{};
        float y1{};
        float x2{};
        float y2{};
        OverlayType type;
        Color color;
    };

    struct Arc {
        float cx{};
        float cy{};
        float r{};
        float startAngle{};
        float endAngle{};
        OverlayType type;
        Color color;
    };

    struct {
        SelectionModel model;
        OverlayType type;
        Color color;
    } selection_;

    std::optional<Rect> selectionRect_;

    std::vector<Point> points_;
    std::vector<Line> lines_;
    std::vector<Circle> circles_;
    std::vector<Arc> arcs_;
    // Pending tool operands do not change the shared user selection.
    std::vector<core::sketch::GeometryRef> constraintRefs_;

    // Replaces the complete hint without touching tool preview or user selection.
    void setSnapFeedback(app::SnapFeedback feedback) { snapFeedback_ = std::move(feedback); }

    const std::optional<app::SnapFeedback>& snapFeedback() const noexcept { return snapFeedback_; }

    void clearSnap() { snapFeedback_.reset(); }

    void clearPreview() {
        points_.clear();
        lines_.clear();
        circles_.clear();
        arcs_.clear();
        constraintRefs_.clear();
    }

    void clearSelection() {
        selection_.model.clear();
        selectionRect_.reset();
    }

    void clear() {
        clearPreview();
        clearSelection();
        clearSnap();
    }

private:
    std::optional<app::SnapFeedback> snapFeedback_;
};

#endif // ! OURPAINT_APPLICATION_OVERLAY_MODEL_H_
