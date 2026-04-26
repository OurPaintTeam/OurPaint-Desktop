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
        case Mode::TwoPoints:
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
        case Mode::ThreePoints:
            if (step_ == Step::WaitingThirdInput) {
                const glm::dvec2& a = points_[0];
                const glm::dvec2& b = points_[1];
                const glm::dvec2& c = cursor;

                const double d = 2.0 * (
                    a.x * (b.y - c.y) +
                    b.x * (c.y - a.y) +
                    c.x * (a.y - b.y)
                    );

                const double a2 = a.x * a.x + a.y * a.y;
                const double b2 = b.x * b.x + b.y * b.y;
                const double c2 = c.x * c.x + c.y * c.y;

                const double ux =
                    (a2 * (b.y - c.y) +
                     b2 * (c.y - a.y) +
                     c2 * (a.y - b.y)) / d;

                const double uy =
                    (a2 * (c.x - b.x) +
                     b2 * (a.x - c.x) +
                     c2 * (b.x - a.x)) / d;

                const glm::dvec2 center{ux, uy};
                const double r = glm::distance(center, a);


                if (!overlay_.circles_.empty()) {
                    overlay_.circles_[0].r = r;
                    overlay_.circles_[0].cx = center.x;
                    overlay_.circles_[0].cy = center.y;
                }
                if (overlay_.points_.size() > 3) {
                    overlay_.points_[2].x = cursor.x;
                    overlay_.points_[2].y = cursor.y;
                    overlay_.points_[3].x = center.x;
                    overlay_.points_[3].y = center.y;
                }
            }
            break;

        case Mode::TangentTwoLines:
            //handleTangentTwoLinesMove(cursor);
            break;

        case Mode::TangentThreeLines:
            //handleTangentThreeLinesMove(cursor);
            break;
    }
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
                Document* document = documentManager_.getActiveDocument();
                UndoRedo::Transaction* txn = document->commandManager().invoke("CIRCLE", {cx, cy, r});
                document->undoRedoManager().push(std::move(*txn));
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
                Document* document = documentManager_.getActiveDocument();
                UndoRedo::Transaction* txn = document->commandManager().invoke("CIRCLE", {cx, cy, r});
                document->undoRedoManager().push(std::move(*txn));
                reset();
            }
            break;
        case Mode::TwoPoints:
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

                Document* document = documentManager_.getActiveDocument();
                UndoRedo::Transaction* txn = document->commandManager().invoke("CIRCLE", {center.x, center.y, r});
                document->undoRedoManager().push(std::move(*txn));
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
                glm::dvec2 a = points_[0];
                glm::dvec2 b = points_[1];
                glm::dvec2 c = cursor;

                double d = 2.0 * (
                    a.x * (b.y - c.y) +
                    b.x * (c.y - a.y) +
                    c.x * (a.y - b.y)
                    );

                double a2 = a.x * a.x + a.y * a.y;
                double b2 = b.x * b.x + b.y * b.y;
                double c2 = c.x * c.x + c.y * c.y;

                double ux =
                    (a2 * (b.y - c.y) +
                     b2 * (c.y - a.y) +
                     c2 * (a.y - b.y)) / d;

                double uy =
                    (a2 * (c.x - b.x) +
                     b2 * (a.x - c.x) +
                     c2 * (b.x - a.x)) / d;

                glm::dvec2 center{ux, uy};
                double r = glm::distance(center, a);

                Document* document = documentManager_.getActiveDocument();
                UndoRedo::Transaction* txn = document->commandManager().invoke("CIRCLE", {center.x, center.y, r});
                document->undoRedoManager().push(std::move(*txn));
                reset();
            }
            break;
        case Mode::TangentTwoLines:
            //handleTangentTwoLinesClick(cursor);
            break;

        case Mode::TangentThreeLines:
            //handleTangentThreeLinesClick(cursor);
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
    return camera_.screenToWorld({x, y});
}

void CircleTool::reset() {
    points_.clear();
    step_ = Step::WaitingFirstInput;
    overlay_.circles_.clear();
    overlay_.points_.clear();
}

/*
void CircleTool::handlePointModeMove(glm::dvec2 cursor) {
    if (points_.empty()) {
        return;
    }

    std::optional<CircleDraft> preview = buildPreviewCircle(cursor);
    if (!preview.has_value()) {
        return;
    }

    updateOverlayCircle(preview.value());
}

void CircleTool::handlePointClick(glm::dvec2 cursor) {
    points_.push_back(cursor);


    updateStep();

    std::optional<CircleDraft> finalCircle = buildFinalCircle();

    if (!finalCircle.has_value()) {
        return;
    }

    commitCircle(finalCircle.value());
    reset();
}

std::optional<CircleTool::CircleDraft>
CircleTool::buildPreviewCircle(glm::dvec2 cursor) const {
    switch (mode_) {
        case Mode::CenterRadius:
            if (points_.size() == 1) {
                return buildCenterRadius(cursor);
            }
            break;

        case Mode::CenterDiameter:
            if (points_.size() == 1) {
                return buildCenterDiameter(cursor);
            }
            break;

        case Mode::TwoPoints:
            if (points_.size() == 1) {
                return buildTwoPoints(cursor);
            }
            break;

        case Mode::ThreePoints:
            if (points_.size() == 2) {
                return buildThreePoints(cursor);
            }
            break;

        case Mode::TangentTwoLines:
        case Mode::TangentThreeLines:
            break;
    }

    return std::nullopt;
}

std::optional<CircleTool::CircleDraft>
CircleTool::buildFinalCircle() const {
    switch (mode_) {
        case Mode::CenterRadius:
            if (points_.size() == 2) {
                return buildCenterRadius(points_[1]);
            }
            break;

        case Mode::CenterDiameter:
            if (points_.size() == 2) {
                return buildCenterDiameter(points_[1]);
            }
            break;

        case Mode::TwoPoints:
            if (points_.size() == 2) {
                return buildTwoPoints(points_[1]);
            }
            break;

        case Mode::ThreePoints:
            if (points_.size() == 3) {
                return buildThreePoints(points_[2]);
            }
            break;

        case Mode::TangentTwoLines:
        case Mode::TangentThreeLines:
            break;
    }

    return std::nullopt;
}

std::optional<CircleTool::CircleDraft>
CircleTool::buildCenterRadius(glm::dvec2 secondPoint) const {
    if (points_.empty()) {
        return std::nullopt;
    }

    const glm::dvec2 center = points_[0];
    const double radius = glm::distance(center, secondPoint);

    if (radius <= kEpsilon) {
        return std::nullopt;
    }

    return CircleDraft{center, radius};
}

std::optional<CircleTool::CircleDraft>
CircleTool::buildCenterDiameter(glm::dvec2 secondPoint) const {
    if (points_.empty()) {
        return std::nullopt;
    }

    const glm::dvec2 center = points_[0];
    const double diameter = glm::distance(center, secondPoint);
    const double radius = diameter / 2.0;

    if (radius <= kEpsilon) {
        return std::nullopt;
    }

    return CircleDraft{center, radius};
}

std::optional<CircleTool::CircleDraft>
CircleTool::buildTwoPoints(glm::dvec2 secondPoint) const {
    if (points_.empty()) {
        return std::nullopt;
    }

    const glm::dvec2 a = points_[0];
    const glm::dvec2 b = secondPoint;

    const glm::dvec2 center = (a + b) * 0.5;
    const double radius = glm::distance(a, b) * 0.5;

    if (radius <= kEpsilon) {
        return std::nullopt;
    }

    return CircleDraft{center, radius};
}

std::optional<CircleTool::CircleDraft>
CircleTool::buildThreePoints(glm::dvec2 thirdPoint) const {
    if (points_.size() < 2) {
        return std::nullopt;
    }

    return circleFromThreePoints(points_[0], points_[1], thirdPoint);
}

std::optional<CircleTool::CircleDraft>
CircleTool::circleFromThreePoints(
    glm::dvec2 a,
    glm::dvec2 b,
    glm::dvec2 c
) {
    const double d =
        2.0 * (
            a.x * (b.y - c.y) +
            b.x * (c.y - a.y) +
            c.x * (a.y - b.y)
        );

    if (std::abs(d) <= kEpsilon) {
        return std::nullopt;
    }

    const double a2 = a.x * a.x + a.y * a.y;
    const double b2 = b.x * b.x + b.y * b.y;
    const double c2 = c.x * c.x + c.y * c.y;

    const double ux =
        (a2 * (b.y - c.y) +
         b2 * (c.y - a.y) +
         c2 * (a.y - b.y)) / d;

    const double uy =
        (a2 * (c.x - b.x) +
         b2 * (a.x - c.x) +
         c2 * (b.x - a.x)) / d;

    const glm::dvec2 center{ux, uy};
    const double radius = glm::distance(center, a);

    if (radius <= kEpsilon) {
        return std::nullopt;
    }

    return CircleDraft{center, radius};
}


void CircleTool::updateOverlayCircle(const CircleDraft& circle) {
    if (renderData_.overlay.circles.empty()) {
        renderData_.overlay.circles.push_back(
            renderer::CircleArc(circle.center.x, circle.center.y, circle.radius)
        );
        return;
    }

    renderData_.overlay.circles[0].x = circle.center.x;
    renderData_.overlay.circles[0].y = circle.center.y;
    renderData_.overlay.circles[0].r = circle.radius;
}

void CircleTool::commitCircle(const CircleDraft& circle) {
    if (circle.radius <= kEpsilon) {
        return;
    }

    Document* document = documentManager_.getActiveDocument();
    if (document == nullptr) {
        return;
    }

    UndoRedo::Transaction* txn =
        document->commandManager().invoke(
            "CIRCLE",
            {circle.center.x, circle.center.y, circle.radius}
        );

    if (txn == nullptr) {
        return;
    }

    document->undoRedoManager().push(std::move(*txn));
}

void CircleTool::handleTangentTwoLinesMove(glm::dvec2 cursor) {
    (void)cursor;

    // Здесь позже:
    // 1. подсветить линию под курсором;
    // 2. если выбрано 2 линии — показать preview касательной окружности.
}

void CircleTool::handleTangentTwoLinesClick(glm::dvec2 cursor) {
    (void)cursor;

    // Здесь позже:
    // 1. найти line/segment под курсором;
    // 2. добавить ID в objects_;
    // 3. если objects_.size() == 2:
    //      - вычислить окружность;
    //      - создать Circle;
    //      - добавить Tangent constraints.
}

void CircleTool::handleTangentThreeLinesMove(glm::dvec2 cursor) {
    (void)cursor;

    // Аналогично TangentTwoLines, но для 3 линий.
}

void CircleTool::handleTangentThreeLinesClick(glm::dvec2 cursor) {
    (void)cursor;

    // Здесь позже:
    // если objects_.size() == 3:
    //      Scene::createCircleTangentToThreeLines(...)
}
*/

