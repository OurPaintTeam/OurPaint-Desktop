#include "CircleTool.h"

#include <cmath>
#include <algorithm>

#include "Document.h"
#include "Transaction.h"
#include "UndoRedo.h"
#include "ConsoleManager.h"
#include "Scene.h"

CircleTool::CircleTool(DocumentManager& documentManager,
                       Camera2D& camera,
                       Cpu2dPicker& picker,
                       OverlayModel& overlay)
    : documentManager_(documentManager), camera_(camera), picker_(picker), overlay_(overlay) {}

void CircleTool::setMode(Mode mode) {
    if (mode_ == mode) {
        return;
    }

    mode_ = mode;
    reset();
}


void CircleTool::onMouseMove(const input::MouseMoveEvent& e) {
    const glm::dvec2 cursor = screenToWorld(e.x, e.y);

    switch (mode_) {
        case Mode::CenterRadius:
            if (step_ == Step::WaitingSecondInput) {
                double r = glm::distance(cursor, points_[0]);
                overlay_.circles_[0].r = r;
                if (overlay_.points_.size() > 1) {
                    overlay_.points_[1] = {static_cast<float>(cursor.x), static_cast<float>(cursor.y)};
                }
            }
            break;
        case Mode::CenterDiameter:
            if (step_ == Step::WaitingSecondInput) {
                double diameter = glm::distance(cursor, points_[0]);
                double r = diameter / 2.0;
                if (!overlay_.circles_.empty()) {
                    overlay_.circles_[0].r = r;
                }
                if (overlay_.points_.size() > 1) {
                    overlay_.points_[1] = {static_cast<float>(cursor.x), static_cast<float>(cursor.y)};
                }
            }
            break;
        case Mode::DiameterTwoPoints:
            if (step_ == Step::WaitingSecondInput) {
                glm::dvec2 a = points_[0];
                glm::dvec2 b = cursor;
                glm::dvec2 center = (a + b) * 0.5;
                double r = glm::distance(a, b) * 0.5;
                if (!overlay_.circles_.empty()) {
                    overlay_.circles_[0].cx = center.x;
                    overlay_.circles_[0].cy = center.y;
                    overlay_.circles_[0].r = r;
                }
                if (overlay_.points_.size() > 2) {
                    // center point
                    overlay_.points_[1].x = center.x;
                    overlay_.points_[1].y = center.y;
                    // second point
                    overlay_.points_[2].x = cursor.x;
                    overlay_.points_[2].y = cursor.y;
                }
            }
            break;
        case Mode::ThreePoints:
            if (step_ == Step::WaitingThirdInput) {
                Circle c = buildCircleFromThreePoints(points_[0], points_[1], cursor);

                if (!overlay_.circles_.empty()) {
                    overlay_.circles_[0].r = c.r;
                    overlay_.circles_[0].cx = c.cx;
                    overlay_.circles_[0].cy = c.cy;
                }
                if (overlay_.points_.size() > 3) {
                    overlay_.points_[2].x = cursor.x;
                    overlay_.points_[2].y = cursor.y;
                    overlay_.points_[3].x = c.cx;
                    overlay_.points_[3].y = c.cy;
                }
            }
            break;

        case Mode::TangentTwoObjectsRadius:
            //handleTangentTwoLinesMove(cursor);
            break;

        case Mode::TangentThreeObjects:
            //handleTangentThreeLinesMove(cursor);
            break;
    }
}

bool intersectLines(
    glm::dvec2 p1, glm::dvec2 d1,
    glm::dvec2 p2, glm::dvec2 d2,
    glm::dvec2& out)
{
    double det = d1.x * d2.y - d1.y * d2.x;

    if (std::abs(det) < 1e-12)
        return false; // параллельны

    glm::dvec2 diff = p2 - p1;

    double t = (diff.x * d2.y - diff.y * d2.x) / det;

    out = p1 + t * d1;
    return true;
}

void CircleTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (!(e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press)) {
        return;
    }

    const glm::dvec2 cursor = screenToWorld(e.x, e.y);

    switch (mode_) {
        case Mode::CenterRadius:
            if (step_ == Step::WaitingFirstInput) {
                points_.push_back(cursor);
                overlay_.circles_.push_back(OverlayModel::Circle(cursor.x, cursor.y, 0.0));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingSecondInput;
            }
            else if (step_ == Step::WaitingSecondInput) {
                double cx = points_[0].x;
                double cy = points_[0].y;
                double r = glm::distance(cursor, points_[0]);
                pushCircleToModel({cx, cy, r});
                reset();
            }
            break;
        case Mode::CenterDiameter:
            if (step_ == Step::WaitingFirstInput) {
                points_.push_back(cursor);
                overlay_.circles_.push_back(OverlayModel::Circle(cursor.x, cursor.y, 0.0));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingSecondInput;
            }
            else if (step_ == Step::WaitingSecondInput) {
                double cx = points_[0].x;
                double cy = points_[0].y;
                double diameter = glm::distance(cursor, points_[0]);
                double r = diameter / 2.0;
                pushCircleToModel({cx, cy, r});
                reset();
            }
            break;
        case Mode::DiameterTwoPoints:
            if (step_ == Step::WaitingFirstInput) {
                points_.push_back(cursor);
                overlay_.circles_.push_back(OverlayModel::Circle(cursor.x, cursor.y, 0.0));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingSecondInput;
            }
            else if (step_ == Step::WaitingSecondInput) {
                glm::dvec2 a = points_[0];
                glm::dvec2 b = cursor;
                glm::dvec2 center = (a + b) * 0.5;
                double r = glm::distance(a, b) * 0.5;

                pushCircleToModel({center.x, center.y, r});
                reset();
            }
            break;
        case Mode::ThreePoints:
            if (step_ == Step::WaitingFirstInput) {
                points_.push_back(cursor);
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y)); // first point
                step_ = Step::WaitingSecondInput;
            }
            else if (step_ == Step::WaitingSecondInput) {
                points_.push_back(cursor);
                overlay_.circles_.push_back(OverlayModel::Circle(cursor.x, cursor.y, 0.0)); // just init
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y)); // second point
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y)); // third point
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y)); // center
                step_ = Step::WaitingThirdInput;
            }
            else if (step_ == Step::WaitingThirdInput) {
                Circle c = buildCircleFromThreePoints(points_[0], points_[1], cursor);
                pushCircleToModel(c);
                reset();
            }
            break;
        case Mode::TangentTwoObjectsRadius:
            if (step_ == Step::WaitingFirstInput) {
                auto obj = picker_.pickAt(e.x, e.y);
                if (!obj.has_value()) {
                    return;
                }
                objects_.push_back(obj.value().id);
                points_.push_back(cursor);
                step_ = Step::WaitingSecondInput;
            }
            else if (step_ == Step::WaitingSecondInput) {
                auto obj = picker_.pickAt(e.x, e.y);
                if (!obj.has_value()) {
                    return;
                }
                if (!obj.has_value()) {
                    return;
                }
                Scene& scene = documentManager_.getActiveDocument()->scene();
                ObjectData od1 = scene.getObjectData(objects_[0]);
                ObjectData od2 = scene.getObjectData(obj.value().id);

                if (od1.et == ObjType::ET_POINT && od2.et == ObjType::ET_POINT) {

                }
                else if (od1.et == ObjType::ET_LINE && od2.et == ObjType::ET_LINE) {

                }
                else if (od1.et == ObjType::ET_CIRCLE && od2.et == ObjType::ET_CIRCLE) {

                }
                else if ((od1.et == ObjType::ET_POINT && od2.et == ObjType::ET_LINE) ||
                         (od1.et == ObjType::ET_LINE && od2.et == ObjType::ET_POINT)) {

                }
                else if ((od1.et == ObjType::ET_POINT && od2.et == ObjType::ET_CIRCLE) ||
                         (od1.et == ObjType::ET_CIRCLE && od2.et == ObjType::ET_POINT)) {

                }
                else if ((od1.et == ObjType::ET_LINE && od2.et == ObjType::ET_CIRCLE) ||
                         (od1.et == ObjType::ET_CIRCLE && od2.et == ObjType::ET_LINE)) {

                }
                reset();

                // const glm::dvec2& click1 = points_[0];
                // const glm::dvec2& click2 = cursor;
                //
                // // Line 1: A1 -> B1
                // glm::dvec2 A1 = {l1.params[0], l1.params[1]};
                // glm::dvec2 B1 = {l1.params[2], l1.params[3]};
                //
                // // Line 2: A2 -> B2
                // glm::dvec2 A2 = {l2.params[0], l2.params[1]};
                // glm::dvec2 B2 = {l2.params[2], l2.params[3]};
                //
                // glm::dvec2 d1 = glm::normalize(B1 - A1);
                // glm::dvec2 d2 = glm::normalize(B2 - A2);
                //
                // glm::dvec2 n1 = { -d1.y, d1.x };
                // glm::dvec2 n2 = { -d2.y, d2.x };
                //
                // glm::dvec2 O;
                // intersectLines(A1, d1, A2, d2, O);
                //
                // glm::dvec2 ray1 = glm::normalize(click1 - O);
                // glm::dvec2 ray2 = glm::normalize(click2 - O);
                //
                // int sign1 = glm::dot(n1, ray2) > 0.0 ? +1 : -1;
                // int sign2 = glm::dot(n2, ray1) > 0.0 ? +1 : -1;
                //
                // std::cout << sign1 << ' ' << sign2 << '\n';
            }
            break;
        case Mode::TangentThreeObjects:
            break;
    }
}

void CircleTool::onKey(const input::KeyEvent& e) {
    (void)e;
}

bool CircleTool::cancel() {
    if (points_.empty()) {
        return false;
    }

    reset();
    return true;
}

glm::dvec2 CircleTool::screenToWorld(double x, double y) const {
    return camera_.screenLogicalToWorld({x, y});
}

void CircleTool::reset() {
    points_.clear();
    objects_.clear();
    step_ = Step::WaitingFirstInput;
    overlay_.circles_.clear();
    overlay_.points_.clear();
}

CircleTool::Circle CircleTool::buildCircleFromThreePoints(const glm::dvec2& p0, const glm::dvec2& p1, const glm::dvec2& p2) {
    double det = p0.x * (p1.y - p2.y) +
                 p1.x * (p2.y - p0.y) +
                 p2.x * (p0.y - p1.y);

    const double eps = 1e-10;
    if (std::abs(det) < eps) {
        // points are collinear or almost collinear
        return {0.0, 0.0, 0.0};
    }

    double a2 = p0.x * p0.x + p0.y * p0.y;
    double b2 = p1.x * p1.x + p1.y * p1.y;
    double c2 = p2.x * p2.x + p2.y * p2.y;

    double ux =
        (a2 * (p1.y - p2.y) +
         b2 * (p2.y - p0.y) +
         c2 * (p0.y - p1.y)) / (det * 2.0);

    double uy =
        (a2 * (p2.x - p1.x) +
         b2 * (p0.x - p2.x) +
         c2 * (p1.x - p0.x)) / (det * 2.0);

    double r = glm::distance({ux, uy}, p0);

    return Circle{ux, uy, r};
}

void CircleTool::pushCircleToModel(const Circle& c) const {
    Document* document = documentManager_.getActiveDocument();
    Transaction* txn = document->commandManager().invoke("CIRCLE", {c.cx, c.cy, c.r});
    document->undoRedoManager().push(std::move(*txn));
}


