#include "CircleTool.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "Document.h"

CircleTool::CircleTool(core::Document& document, render::Camera2D& camera, snap::SnapSystem& snap, Cpu2dPicker& picker, OverlayModel& overlay)
    : IInteractionTool(document, camera, snap, overlay, picker) {}

void CircleTool::setMode(Mode mode) {
    if (mode_ == mode) {
        return;
    }

    mode_ = mode;
    reset();
}

void CircleTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 cursor = screenToWorld(e.x, e.y);

    snap::SnapRequest request{
        core::sketch::Vec2{cursor.x, cursor.y},
        snap::ActionType::Draw,
        snap::PrimitiveType::Circle,
    };

    switch (mode_) {
        case Mode::CenterRadius:
            request.drawState = snap::DrawState::Center;
            if (step_ == Step::WaitingSecondInput) {
                request.drawState = snap::DrawState::Body;
                request.anchors.push_back(core::sketch::Point2{core::sketch::Vec2{points_[0].x, points_[0].y}});
            }
            break;
        case Mode::CenterDiameter:
            request.drawState = snap::DrawState::Center;
            if (step_ == Step::WaitingSecondInput) {
                request.drawState = snap::DrawState::Body;
                request.anchors.push_back(core::sketch::Point2{core::sketch::Vec2{points_[0].x, points_[0].y}});
            }
            break;
        case Mode::DiameterTwoPoints:
            request.drawState = snap::DrawState::Body;
            if (step_ == Step::WaitingSecondInput) {
                request.anchors.push_back(core::sketch::Point2{core::sketch::Vec2{points_[0].x, points_[0].y}});
            }
            break;
        case Mode::ThreePoints:
            request.drawState = snap::DrawState::Body;
            if (step_ == Step::WaitingSecondInput) {
                request.anchors.push_back(core::sketch::Point2{core::sketch::Vec2{points_[0].x, points_[0].y}});
            } else if (step_ == Step::WaitingThirdInput) {
                request.anchors.push_back(core::sketch::Point2{core::sketch::Vec2{points_[0].x, points_[0].y}});
                request.anchors.push_back(core::sketch::Point2{core::sketch::Vec2{points_[1].x, points_[1].y}});
            }
            break;
        default:
            break;
    }

    snapVisual(request);

    if (!snap_results_.empty()) {
        cursor.x = snap_results_[0].point.position.x;
        cursor.y = snap_results_[0].point.position.y;
    }

    switch (mode_) {
        case Mode::CenterRadius:
            if (step_ == Step::WaitingSecondInput) {
                double r = glm::distance(cursor, points_[0]);
                if (!overlay_.circles_.empty()) {
                    overlay_.circles_[0].r = r;
                }
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
                    overlay_.points_[1].x = center.x;
                    overlay_.points_[1].y = center.y;
                    overlay_.points_[2].x = cursor.x;
                    overlay_.points_[2].y = cursor.y;
                }
            }
            break;
        case Mode::ThreePoints:
            if (step_ == Step::WaitingThirdInput) {
                const auto circle = buildCircleFromThreePoints(points_[0], points_[1], cursor);
                if (circle) {
                    const Circle c = *circle;
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
            }
            break;
        default:
            break;
    }
}

bool intersectLines(glm::dvec2 p1, glm::dvec2 d1, glm::dvec2 p2, glm::dvec2 d2, glm::dvec2& out) {
    double det = d1.x * d2.y - d1.y * d2.x;

    if (std::abs(det) < 1e-12) {
        return false;
    }

    glm::dvec2 diff = p2 - p1;

    double t = (diff.x * d2.y - diff.y * d2.x) / det;

    out = p1 + t * d1;
    return true;
}


void CircleTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (!(e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press)) {
        return;
    }

    glm::dvec2 cursor = screenToWorld(e.x, e.y);

    if (!snap_results_.empty()) {
        cursor.x = snap_results_[0].point.position.x;
        cursor.y = snap_results_[0].point.position.y;
    }

    switch (mode_) {
        case Mode::CenterRadius:
            if (step_ == Step::WaitingFirstInput) {
                firstSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                   : snap_results_.front();
                points_.push_back(cursor);
                overlay_.circles_.push_back(OverlayModel::Circle(cursor.x, cursor.y, 0.0));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingSecondInput;
            } else if (step_ == Step::WaitingSecondInput) {
                secondSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                    : snap_results_.front();

                double cx = points_[0].x;
                double cy = points_[0].y;
                double r = glm::distance(cursor, points_[0]);

                auto circleResult = pushCircleToModel({cx, cy, r});

                bool needsSolve = false;
                if (circleResult) {
                    const auto circleId = circleResult.value();
                    using SE = core::sketch::SubElement;
                    if (applySnapConstraint(firstSnap_,  {circleId, SE::Center})) needsSolve = true;
                    if (applySnapConstraint(secondSnap_, {circleId, SE::Whole}))  needsSolve = true;
                }
                if (needsSolve) {
                    auto solved = document_.sketch().solve();
                    if (!solved ||
                        solved.value().status != core::sketch::SolveStatus::Converged) {
                        std::cout << "solve err in CircleTool\n";
                    }
                }

                reset();
            }
            break;

        case Mode::CenterDiameter:
            if (step_ == Step::WaitingFirstInput) {
                firstSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                   : snap_results_.front();
                points_.push_back(cursor);
                overlay_.circles_.push_back(OverlayModel::Circle(cursor.x, cursor.y, 0.0));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingSecondInput;
            } else if (step_ == Step::WaitingSecondInput) {
                secondSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                    : snap_results_.front();

                double cx = points_[0].x;
                double cy = points_[0].y;
                double diameter = glm::distance(cursor, points_[0]);
                double r = diameter / 2.0;

                auto circleResult = pushCircleToModel({cx, cy, r});

                bool needsSolve = false;
                if (circleResult) {
                    const auto circleId = circleResult.value();
                    using SE = core::sketch::SubElement;
                    if (applySnapConstraint(firstSnap_,  {circleId, SE::Center})) needsSolve = true;
                    if (applySnapConstraint(secondSnap_, {circleId, SE::Whole}))  needsSolve = true;
                }
                if (needsSolve) {
                    auto solved = document_.sketch().solve();
                    if (!solved ||
                        solved.value().status != core::sketch::SolveStatus::Converged) {
                        std::cout << "solve err in CircleTool\n";
                    }
                }

                reset();
            }
            break;

        case Mode::DiameterTwoPoints:
            if (step_ == Step::WaitingFirstInput) {
                firstSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                   : snap_results_.front();
                points_.push_back(cursor);
                overlay_.circles_.push_back(OverlayModel::Circle(cursor.x, cursor.y, 0.0));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingSecondInput;
            } else if (step_ == Step::WaitingSecondInput) {
                glm::dvec2 a = points_[0];
                glm::dvec2 b = cursor;
                glm::dvec2 center = (a + b) * 0.5;
                double r = glm::distance(a, b) * 0.5;

                secondSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                    : snap_results_.front();

                auto circleResult = pushCircleToModel({center.x, center.y, r});

                bool needsSolve = false;
                if (circleResult) {
                    const auto circleId = circleResult.value();
                    using SE = core::sketch::SubElement;
                    if (applySnapConstraint(firstSnap_,  {circleId, SE::Whole})) needsSolve = true;
                    if (applySnapConstraint(secondSnap_, {circleId, SE::Whole})) needsSolve = true;
                }
                if (needsSolve) {
                    auto solved = document_.sketch().solve();
                    if (!solved ||
                        solved.value().status != core::sketch::SolveStatus::Converged) {
                        std::cout << "solve err in CircleTool\n";
                    }
                }

                reset();
            }
            break;

        case Mode::ThreePoints:
            if (step_ == Step::WaitingFirstInput) {
                firstSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                   : snap_results_.front();
                points_.push_back(cursor);
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingSecondInput;
            } else if (step_ == Step::WaitingSecondInput) {
                secondSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                    : snap_results_.front();
                points_.push_back(cursor);
                overlay_.circles_.push_back(OverlayModel::Circle(cursor.x, cursor.y, 0.0));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                overlay_.points_.push_back(OverlayModel::Point(cursor.x, cursor.y));
                step_ = Step::WaitingThirdInput;
            } else if (step_ == Step::WaitingThirdInput) {
                thirdSnap_ = snap_results_.empty() ? snap::SnapResult{}
                                                   : snap_results_.front();

                const auto circle = buildCircleFromThreePoints(points_[0], points_[1], cursor);

                if (!circle) {
                    reset();
                    break;
                }

                auto circleResult = pushCircleToModel(*circle);

                bool needsSolve = false;
                if (circleResult) {
                    const auto circleId = circleResult.value();
                    using SE = core::sketch::SubElement;
                    if (applySnapConstraint(firstSnap_,  {circleId, SE::Whole})) needsSolve = true;
                    if (applySnapConstraint(secondSnap_, {circleId, SE::Whole})) needsSolve = true;
                    if (applySnapConstraint(thirdSnap_,  {circleId, SE::Whole})) needsSolve = true;
                }
                if (needsSolve) {
                    auto solved = document_.sketch().solve();
                    if (!solved ||
                        solved.value().status != core::sketch::SolveStatus::Converged) {
                        std::cout << "solve err in CircleTool\n";
                    }
                }

                reset();
            }
            break;

        case Mode::TangentTwoObjectsRadius:
            if (step_ == Step::WaitingFirstInput) {
                auto obj = picker_.pickAtScreenLogical(e.x, e.y);
                if (!obj.has_value()) {
                    return;
                }
                objects_.push_back(obj.value().ref.entity);
                points_.push_back(cursor);
                step_ = Step::WaitingSecondInput;
            } else if (step_ == Step::WaitingSecondInput) {
                auto obj = picker_.pickAtScreenLogical(e.x, e.y);
                if (!obj.has_value()) {
                    return;
                }
                if (!obj.has_value()) {
                    return;
                }
                // Scene& scene = document_.scene();
                // ObjectData od1 = scene.getObjectData(objects_[0]);
                // ObjectData od2 = scene.getObjectData(obj.value().id);
                //
                // if (od1.et == ObjType::ET_POINT && od2.et == ObjType::ET_POINT) {
                //
                // }
                // else if (od1.et == ObjType::ET_LINE && od2.et == ObjType::ET_LINE) {
                //
                // }
                // else if (od1.et == ObjType::ET_CIRCLE && od2.et == ObjType::ET_CIRCLE) {
                //
                // }
                // else if ((od1.et == ObjType::ET_POINT && od2.et == ObjType::ET_LINE) ||
                //          (od1.et == ObjType::ET_LINE && od2.et == ObjType::ET_POINT)) {
                //
                // }
                // else if ((od1.et == ObjType::ET_POINT && od2.et == ObjType::ET_CIRCLE) ||
                //          (od1.et == ObjType::ET_CIRCLE && od2.et == ObjType::ET_POINT)) {
                //
                // }
                // else if ((od1.et == ObjType::ET_LINE && od2.et == ObjType::ET_CIRCLE) ||
                //          (od1.et == ObjType::ET_CIRCLE && od2.et == ObjType::ET_LINE)) {
                //
                // }
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
        return {};
    }

    reset();
    return false;
}

glm::dvec2 CircleTool::screenToWorld(double x, double y) const { return camera_.screenLogicalToWorld({x, y}); }

void CircleTool::reset() {
    points_.clear();
    objects_.clear();
    step_ = Step::WaitingFirstInput;
    overlay_.circles_.clear();
    overlay_.points_.clear();
    firstSnap_ = {};
    secondSnap_ = {};
    thirdSnap_ = {};
}

std::optional<CircleTool::Circle> CircleTool::buildCircleFromThreePoints(const glm::dvec2& p0, const glm::dvec2& p1, const glm::dvec2& p2) {
    double det = p0.x * (p1.y - p2.y) + p1.x * (p2.y - p0.y) + p2.x * (p0.y - p1.y);

    const double eps = 1e-10;
    if (std::abs(det) < eps) {
        // points are collinear or almost collinear
        return std::nullopt;
    }

    double a2 = p0.x * p0.x + p0.y * p0.y;
    double b2 = p1.x * p1.x + p1.y * p1.y;
    double c2 = p2.x * p2.x + p2.y * p2.y;

    double ux = (a2 * (p1.y - p2.y) + b2 * (p2.y - p0.y) + c2 * (p0.y - p1.y)) / (det * 2.0);

    double uy = (a2 * (p2.x - p1.x) + b2 * (p0.x - p2.x) + c2 * (p1.x - p0.x)) / (det * 2.0);

    double r = glm::distance({ux, uy}, p0);

    return Circle{ux, uy, r};
}

core::sketch::Result<core::sketch::EntityId> CircleTool::pushCircleToModel(const Circle& c) const {
    return
     document_.sketch().addCircle({c.cx, c.cy}, c.r);
}
