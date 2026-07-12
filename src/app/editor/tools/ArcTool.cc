#include "ArcTool.h"

#include "ConsoleManager.h"
#include "Document.h"
#include "Document.h"
#include "RenderData.h"
#include "Scene.h"
#include "Transaction.h"
#include "UndoRedo.h"
#include "objects/Objects.h"

ArcTool::ArcTool(Document& document, Camera2D& camera, Cpu2dPicker& picker, OverlayModel& overlay)
    : document_(document), camera_(camera), picker_(picker), overlay_(overlay) {}

void ArcTool::setMode(Mode mode) {
    if (mode_ == mode) {
        return;
    }

    mode_ = mode;
    reset();
}

void ArcTool::reset() {
    points_.clear();
    step_ = Step::WaitingFirstInput;
    overlay_.clear();
}

void ArcTool::onMouseMove(const input::MouseMoveEvent& e) {
    const glm::dvec2 cursor = screenToWorld(e.x, e.y);

    switch (mode_) {
        case Mode::ThreePoints:
            if (step_ == Step::WaitingThirdInput) {
                Arc ca = buildArcFromThreePoints(points_[0], points_[1], cursor);
                if (!ca.valid) {
                    return;
                }
                if (!overlay_.arcs_.empty()) {
                    overlay_.arcs_[0].r = ca.r;
                    overlay_.arcs_[0].cx = ca.cx;
                    overlay_.arcs_[0].cy = ca.cy;
                    overlay_.arcs_[0].startAngle = ca.startAngle;
                    overlay_.arcs_[0].endAngle = ca.endAngle;
                }
                if (overlay_.points_.size() > 3) {
                    overlay_.points_[2].x = cursor.x;
                    overlay_.points_[2].y = cursor.y;
                    overlay_.points_[3].x = ca.cx;
                    overlay_.points_[3].y = ca.cy;
                }
            }
            break;
    }
}

void ArcTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (!(e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press)) {
        return;
    }

    const glm::dvec2 cursor = screenToWorld(e.x, e.y);

    switch (mode_) {
        case Mode::ThreePoints:
            if (step_ == Step::WaitingFirstInput) {
                points_.push_back(cursor);
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingSecondInput;
            }
            else if (step_ == Step::WaitingSecondInput) {
                points_.push_back(cursor);
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.arcs_.push_back(OverlayModel::Arc()); // just init
                step_ = Step::WaitingThirdInput;
            }
            else if (step_ == Step::WaitingThirdInput) {
                //Document* document = documentManager_.getActiveDocument();
                //UndoRedo::Transaction* txn = document->commandManager().invoke("ARC", {firstPoint_X, firstPoint_Y, r});
                //document->undoRedoManager().push(std::move(*txn));

                reset();
            }
            break;
    }
}

void ArcTool::onKey(const input::KeyEvent& e) {
    (void)e;
}

bool ArcTool::cancel() {
    if (points_.empty()) {
        return false;
    }

    reset();
    return true;
}

glm::dvec2 ArcTool::screenToWorld(double x, double y) const {
    return camera_.screenLogicalToWorld({x, y});
}

ArcTool::Arc ArcTool::buildArcFromThreePoints(const glm::dvec2& p0, const glm::dvec2& p1, const glm::dvec2& p2) {
    const double x0 = p0.x;
    const double y0 = p0.y;
    const double x1 = p1.x;
    const double y1 = p1.y;
    const double x2 = p2.x;
    const double y2 = p2.y;

    /*
        | x0 y0 1 |
    S = | x1 y1 1 |
        | x2 y2 1 |
    */

    const double det =
        x0 * (y1 - y2) +
        x1 * (y2 - y0) +
        x2 * (y0 - y1);

    // Points are (almost) collinear -> circle is undefined
    const double eps = 1e-10;
    if (std::fabs(det) < eps) {
        return {0, 0, 0, 0, 0, false};
    }

    /*

    (x0 - cx)^2 + (y0 - cy)^2 = r^2
    (x1 - cx)^2 + (y1 - cy)^2 = r^2
    (x2 - cx)^2 + (y2 - cy)^2 = r^2

    */

    // Precompute squared lengths: si = xi^2 + yi^2
    const double s0 = x0 * x0 + y0 * y0;
    const double s1 = x1 * x1 + y1 * y1;
    const double s2 = x2 * x2 + y2 * y2;

    /*
    Closed-form solution for circle center using determinants.

    These formulas come from solving the linear system derived
    from the circle equation.
    */

    const double cx =
        (s0 * (y1 - y2) +
         s1 * (y2 - y0) +
         s2 * (y0 - y1)) / (2.0 * det);

    const double cy =
        (s0 * (x2 - x1) +
         s1 * (x0 - x2) +
         s2 * (x1 - x0)) / (2.0 * det);

    // Radius
    const double r = std::hypot(x0 - cx, y0 - cy);

    // Normalize angle to [0, 2pi)
    auto normalizeAngle = [&](double a) -> double {
        constexpr double TAU = 2.0 * M_PI;
        a = std::fmod(a, TAU);
        if (a < 0.0) {
            a += TAU;
        }
        return a;
    };

    // Convert point to polar angle relative to circle center
    auto angleOf = [&](double x, double y) -> double {
        return normalizeAngle(std::atan2(y - cy, x - cx));
    };

    const double a0 = angleOf(x0, y0);
    const double a1m = angleOf(x1, y1); // middle point
    const double a2 = angleOf(x2, y2);

    double startAngle;
    double endAngle;

    /*
    Check if 'angle' lies on the CCW arc from 'start' to 'end'.

    Handles wrap-around at 2pi:
    - If start <= end: simple interval [start, end]
    - If start > end: arc crosses 2pi -> angle >= start OR angle <= end
    */
    auto angleOnCCWArc = [&](double angle, double start, double end) -> bool {
        angle = normalizeAngle(angle);
        start = normalizeAngle(start);
        end   = normalizeAngle(end);

        if (start <= end) {
            return angle >= start && angle <= end;
        } else {
            return angle >= start || angle <= end;
        }
    };

    /*
    There are two possible arcs between p0 and p2:
    - one passing through p1
    - the other going the opposite way

    We select the arc that contains p1.
    */
    if (angleOnCCWArc(a1m, a0, a2)) {
        startAngle = a0;
        endAngle   = a2;
    } else {
        startAngle = a2;
        endAngle   = a0;
    }

    return {cx, cy, r, startAngle, endAngle, true};
}


