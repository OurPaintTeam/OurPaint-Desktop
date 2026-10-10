#include "SnapSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "geometry/Algorithms.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"

namespace snap {
namespace {

using namespace core::sketch;

constexpr double kDefaultSnapRadius = 10.0;
constexpr double kEpsilon = 1e-9;

constexpr int kEndpointPriority = 0;
constexpr int kStandalonePointPriority = 1;
constexpr int kIntersectionPriority = 2;
constexpr int kCircleCentrePriority = 2;
constexpr int kMidpointPriority = 3;
constexpr int kGeometryBodyPriority = 4;

using geometry::Circle2;
using geometry::GeometryRef;
using geometry::Line2;
using geometry::Point2;
using geometry::SubElement;
using geometry::Vec2;

Vec2 toVec2(const Point2& point) { return point.position; }

Point2 toPoint2(const Vec2& point) { return Point2{point}; }

Vec2 makeVec(double x, double y) { return Vec2{x, y}; }

Vec2 subtract(const Vec2& a, const Vec2& b) { return makeVec(a.x - b.x, a.y - b.y); }

Vec2 add(const Vec2& a, const Vec2& b) { return makeVec(a.x + b.x, a.y + b.y); }

Vec2 multiply(const Vec2& v, double scalar) { return makeVec(v.x * scalar, v.y * scalar); }

double dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }

double cross(const Vec2& a, const Vec2& b) { return a.x * b.y - a.y * b.x; }

double lengthSquared(const Vec2& v) { return dot(v, v); }

double distanceSquared(const Vec2& a, const Vec2& b) { return lengthSquared(subtract(a, b)); }

double distance(const Vec2& a, const Vec2& b) { return std::sqrt(distanceSquared(a, b)); }

bool isFinite(const core::sketch::Vec2& point) { return std::isfinite(point.x) && std::isfinite(point.y); }

bool isFinite(const Circle2& circle) { return isFinite(circle.center) && std::isfinite(circle.radius) && circle.radius > kEpsilon; }

bool isFinite(const Line2& line) { return isFinite(line.start) && isFinite(line.end) && distanceSquared(line.start, line.end) > kEpsilon * kEpsilon; }

Vec2 midpoint(const Line2& line) { return makeVec((line.start.x + line.end.x) * 0.5, (line.start.y + line.end.y) * 0.5); }

std::optional<Vec2> closestPointOnSegment(const Vec2& point, const Line2& line) {
    if (!isFinite(point) || !isFinite(line)) {
        return std::nullopt;
    }

    const Vec2 direction = subtract(line.end, line.start);
    const double denominator = lengthSquared(direction);

    if (denominator <= kEpsilon * kEpsilon) {
        return line.start;
    }

    const double t = std::clamp(dot(subtract(point, line.start), direction) / denominator, 0.0, 1.0);

    return add(line.start, multiply(direction, t));
}

std::optional<Vec2> closestPointOnCircle(const Vec2& point, const Circle2& circle) {
    if (!isFinite(point) || !isFinite(circle)) {
        return std::nullopt;
    }

    const Vec2 direction = subtract(point, circle.center);
    const double lenSquared = lengthSquared(direction);

    if (lenSquared <= kEpsilon * kEpsilon) {
        return makeVec(circle.center.x + circle.radius, circle.center.y);
    }

    const double scale = circle.radius / std::sqrt(lenSquared);
    return add(circle.center, multiply(direction, scale));
}

struct LineIntersection {
    Vec2 point;
    double t = 0.0;
    double u = 0.0;
};

std::optional<LineIntersection> intersectInfiniteLines(const Line2& first, const Line2& second) {
    if (!isFinite(first) || !isFinite(second)) {
        return std::nullopt;
    }

    const Vec2 r = subtract(first.end, first.start);
    const Vec2 s = subtract(second.end, second.start);
    const Vec2 offset = subtract(second.start, first.start);

    const double denominator = cross(r, s);

    if (std::abs(denominator) <= kEpsilon) {
        return std::nullopt;
    }

    const double t = cross(offset, s) / denominator;
    const double u = cross(offset, r) / denominator;

    return LineIntersection{add(first.start, multiply(r, t)), t, u};
}

bool isOnSegment(double t) { return t >= -kEpsilon && t <= 1.0 + kEpsilon; }

template <typename Callback>
void visitLineCircleIntersections(const Line2& line, const Circle2& circle, bool segmentOnly, Callback&& callback) {
    if (!isFinite(line) || !isFinite(circle)) {
        return;
    }

    const Vec2 direction = subtract(line.end, line.start);
    const Vec2 offset = subtract(line.start, circle.center);

    const double a = dot(direction, direction);
    if (a <= kEpsilon * kEpsilon) {
        return;
    }

    const double b = 2.0 * dot(offset, direction);
    const double c = dot(offset, offset) - circle.radius * circle.radius;

    double discriminant = b * b - 4.0 * a * c;

    if (discriminant < -kEpsilon) {
        return;
    }

    discriminant = std::max(0.0, discriminant);

    const double root = std::sqrt(discriminant);
    const double t1 = (-b - root) / (2.0 * a);
    const double t2 = (-b + root) / (2.0 * a);

    const auto emit = [&](double t) {
        if (segmentOnly && !isOnSegment(t)) {
            return;
        }

        if (!segmentOnly && isOnSegment(t)) {
            return;
        }

        callback(add(line.start, multiply(direction, t)));
    };

    emit(t1);

    if (discriminant > kEpsilon) {
        emit(t2);
    }
}

template <typename Callback>
void visitCircleCircleIntersections(const Circle2& first, const Circle2& second, Callback&& callback) {
    if (!isFinite(first) || !isFinite(second)) {
        return;
    }

    const Vec2 delta = subtract(second.center, first.center);
    const double centerDistanceSquared = lengthSquared(delta);

    if (centerDistanceSquared <= kEpsilon * kEpsilon) {
        return;
    }

    const double centerDistance = std::sqrt(centerDistanceSquared);

    if (centerDistance > first.radius + second.radius + kEpsilon) {
        return;
    }

    if (centerDistance < std::abs(first.radius - second.radius) - kEpsilon) {
        return;
    }

    const double along = (first.radius * first.radius - second.radius * second.radius + centerDistanceSquared) / (2.0 * centerDistance);

    double heightSquared = first.radius * first.radius - along * along;

    if (heightSquared < -kEpsilon) {
        return;
    }

    heightSquared = std::max(0.0, heightSquared);

    const Vec2 unit = multiply(delta, 1.0 / centerDistance);
    const Vec2 base = add(first.center, multiply(unit, along));
    const Vec2 perpendicular = makeVec(-unit.y, unit.x);
    const Vec2 offset = multiply(perpendicular, std::sqrt(heightSquared));

    callback(add(base, offset));

    if (heightSquared > kEpsilon) {
        callback(subtract(base, offset));
    }
}

SnapCandidate makeCandidate(const Vec2& point, SnapKind kind, double candidateDistance, int priority, std::vector<GeometryRef> objects = {},
                            std::vector<Line2> guideLines = {}) {
    SnapResult result;
    result.snapped = true;
    result.point = core::sketch::Point2{point};
    result.type = kind;
    result.objects = std::move(objects);
    result.guideLines = std::move(guideLines);

    return SnapCandidate{std::move(result), candidateDistance * candidateDistance, priority};
}

}  // namespace

bool SnapSystem::isBetterCandidate(const SnapCandidate& candidate, const SnapCandidate& current) {
    constexpr double epsilon = 1e-9;

    if (candidate.distanceSquared < current.distanceSquared - epsilon) {
        return true;
    }

    if (candidate.distanceSquared > current.distanceSquared + epsilon) {
        return false;
    }

    return candidate.priority < current.priority;
}

SnapSystem::SnapSystem(const core::sketch::Sketch& sketch) : sketch_(sketch) {}

SnapResult SnapSystem::getSnapCandidate(const SnapRequest& request) const { return parser(request); }

SnapRule SnapSystem::ruleFor(ToolType tool, DrawState state) {
    using M = SnapMask;

    SnapRule rule;
    rule.maxRadius = kDefaultSnapRadius;

    constexpr M commonPosition = M::StandalonePoint | M::LineEndpoint | M::LineMidpoint | M::LineBody | M::CircleBody | M::CircleCentre | M::Intersection;

    switch (tool) {
        case ToolType::Select:
            // Selection itself should be handled by object picking.
            return {commonPosition, kDefaultSnapRadius};
            break;

        case ToolType::Move:
            rule.mask = commonPosition;
            break;

        case ToolType::Point:
        case ToolType::Circle:
        case ToolType::Arc:
            rule.mask = commonPosition;
            break;

        case ToolType::Line:
            rule.mask = commonPosition;

            // FirstPoint means the first point has already been placed.
            // Directional constraints are useful while positioning the second point.
            if (state == DrawState::FirstPoint) {
                rule.mask |= Direction;
            }

            break;

        case ToolType::Rotate:
        case ToolType::Scale:
            rule.mask = M::StandalonePoint | M::LineEndpoint | M::CircleCentre | M::Intersection;
            break;

        default:
            rule.mask = M::None;
            break;
    }

    return rule;
}

SnapResult SnapSystem::parser(const SnapRequest& request) const {
    const SnapRule rule = ruleFor(request.tool, request.drawState);

    if (rule.mask == SnapMask::None || !isFinite(request.cursor) || !std::isfinite(rule.maxRadius) || rule.maxRadius <= 0.0) {
        return {};
    }

    std::optional<SnapCandidate> best;

    const auto consider = [&](std::optional<SnapCandidate> candidate) {
        if (!candidate) {
            return;
        }

        if (!best || isBetterCandidate(*candidate, *best)) {
            best = std::move(candidate);
        }
    };

    if (hasSnap(rule.mask, SnapMask::StandalonePoint)) {
        consider(findBestStandalonePoint(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::LineEndpoint) || hasSnap(rule.mask, SnapMask::LineMidpoint) || hasSnap(rule.mask, SnapMask::LineBody)) {
        consider(findBestLineCandidate(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::CircleBody) || hasSnap(rule.mask, SnapMask::CircleCentre)) {
        consider(findBestCircleCandidate(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::Intersection) || hasSnap(rule.mask, SnapMask::ExtendedIntersection)) {
        consider(findBestIntersection(request, rule));
    }

    return best ? best->result : SnapResult{};
}

bool SnapSystem::isAllowed(SnapKind kind, double candidateDistance, const SnapRule& rule) {
    if (!std::isfinite(candidateDistance) || candidateDistance < 0.0 || !std::isfinite(rule.maxRadius) || candidateDistance > rule.maxRadius) {
        return false;
    }

    switch (kind) {
        case SnapKind::StandalonePoint:
            return hasSnap(rule.mask, SnapMask::StandalonePoint);
        case SnapKind::LineEndpoint:
            return hasSnap(rule.mask, SnapMask::LineEndpoint);
        case SnapKind::LineMidpoint:
            return hasSnap(rule.mask, SnapMask::LineMidpoint);
        case SnapKind::LineBody:
            return hasSnap(rule.mask, SnapMask::LineBody);
        case SnapKind::CircleBody:
            return hasSnap(rule.mask, SnapMask::CircleBody);
        case SnapKind::CircleCentre:
            return hasSnap(rule.mask, SnapMask::CircleCentre);
        case SnapKind::Intersection:
            return hasSnap(rule.mask, SnapMask::Intersection);
        case SnapKind::ExtendedIntersection:
            return hasSnap(rule.mask, SnapMask::ExtendedIntersection);
        case SnapKind::Horizontal:
            return hasSnap(rule.mask, SnapMask::Horizontal);
        case SnapKind::Vertical:
            return hasSnap(rule.mask, SnapMask::Vertical);
        case SnapKind::Parallel:
            return hasSnap(rule.mask, SnapMask::Parallel);
        case SnapKind::Perpendicular:
            return hasSnap(rule.mask, SnapMask::Perpendicular);
        case SnapKind::Angle:
        case SnapKind::None:
        default:
            return false;
    }
}

std::optional<SnapCandidate> SnapSystem::findBestStandalonePoint(const SnapRequest& request, const SnapRule& rule) const {
    auto entitiesResult = sketch_.points();
    if (!entitiesResult) {
        return std::nullopt;
    }

    std::optional<SnapCandidate> best;

    for (const auto& entity : entitiesResult.value()) {
        const auto* point = std::get_if<Point2>(&entity.geometry);
        if (!point || !isFinite(point->position)) {
            continue;
        }

        const double candidateDistance = distance(request.cursor, point->position);

        if (!isAllowed(SnapKind::StandalonePoint, candidateDistance, rule)) {
            continue;
        }

        auto candidate =
            makeCandidate(point->position, SnapKind::StandalonePoint, candidateDistance, kStandalonePointPriority, {{entity.id, SubElement::Whole}});

        if (!best || isBetterCandidate(candidate, *best)) {
            best = std::move(candidate);
        }
    }

    return best;
}

std::optional<SnapCandidate> SnapSystem::findBestLineCandidate(const SnapRequest& request, const SnapRule& rule) const {
    auto entitiesResult = sketch_.lines();
    if (!entitiesResult) {
        return std::nullopt;
    }

    std::optional<SnapCandidate> best;

    const auto consider = [&](SnapCandidate candidate) {
        if (!best || isBetterCandidate(candidate, *best)) {
            best = std::move(candidate);
        }
    };

    for (const auto& entity : entitiesResult.value()) {
        const auto* line = std::get_if<Line2>(&entity.geometry);
        if (!line || !isFinite(*line)) {
            continue;
        }

        const GeometryRef whole{entity.id, SubElement::Whole};
        const GeometryRef start{entity.id, SubElement::Start};
        const GeometryRef end{entity.id, SubElement::End};

        if (hasSnap(rule.mask, SnapMask::LineEndpoint)) {
            const double startDistance = distance(request.cursor, line->start);

            if (isAllowed(SnapKind::LineEndpoint, startDistance, rule)) {
                consider(makeCandidate(line->start, SnapKind::LineEndpoint, startDistance, kEndpointPriority, {start}));
            }

            const double endDistance = distance(request.cursor, line->end);

            if (isAllowed(SnapKind::LineEndpoint, endDistance, rule)) {
                consider(makeCandidate(line->end, SnapKind::LineEndpoint, endDistance, kEndpointPriority, {end}));
            }
        }

        if (hasSnap(rule.mask, SnapMask::LineMidpoint)) {
            const Vec2 middle = midpoint(*line);
            const double middleDistance = distance(request.cursor, middle);

            if (isAllowed(SnapKind::LineMidpoint, middleDistance, rule)) {
                consider(makeCandidate(middle, SnapKind::LineMidpoint, middleDistance, kMidpointPriority, {whole}));
            }
        }

        if (hasSnap(rule.mask, SnapMask::LineBody)) {
            const auto nearest = closestPointOnSegment(request.cursor, *line);

            if (nearest) {
                const double bodyDistance = distance(request.cursor, *nearest);

                if (isAllowed(SnapKind::LineBody, bodyDistance, rule)) {
                    consider(makeCandidate(*nearest, SnapKind::LineBody, bodyDistance, kGeometryBodyPriority, {whole}));
                }
            }
        }
    }

    return best;
}

std::optional<SnapCandidate> SnapSystem::findBestCircleCandidate(const SnapRequest& request, const SnapRule& rule) const {
    auto entitiesResult = sketch_.circles();
    if (!entitiesResult) {
        return std::nullopt;
    }

    std::optional<SnapCandidate> best;

    const auto consider = [&](SnapCandidate candidate) {
        if (!best || isBetterCandidate(candidate, *best)) {
            best = std::move(candidate);
        }
    };

    for (const auto& entity : entitiesResult.value()) {
        const auto* circle = std::get_if<Circle2>(&entity.geometry);
        if (!circle || !isFinite(*circle)) {
            continue;
        }

        const GeometryRef whole{entity.id, SubElement::Whole};
        const GeometryRef centre{entity.id, SubElement::Center};

        if (hasSnap(rule.mask, SnapMask::CircleCentre)) {
            const double centreDistance = distance(request.cursor, circle->center);

            if (isAllowed(SnapKind::CircleCentre, centreDistance, rule)) {
                consider(makeCandidate(circle->center, SnapKind::CircleCentre, centreDistance, kCircleCentrePriority, {centre}));
            }
        }

        if (hasSnap(rule.mask, SnapMask::CircleBody)) {
            const auto nearest = closestPointOnCircle(request.cursor, *circle);

            if (nearest) {
                const double bodyDistance = distance(request.cursor, *nearest);

                if (isAllowed(SnapKind::CircleBody, bodyDistance, rule)) {
                    consider(makeCandidate(*nearest, SnapKind::CircleBody, bodyDistance, kGeometryBodyPriority, {whole}));
                }
            }
        }
    }

    return best;
}

std::optional<SnapCandidate> SnapSystem::findBestIntersection(const SnapRequest& request, const SnapRule& rule) const {
    const bool allowReal = hasSnap(rule.mask, SnapMask::Intersection);

    const bool allowExtended = hasSnap(rule.mask, SnapMask::ExtendedIntersection);

    if (!allowReal && !allowExtended) {
        return std::nullopt;
    }

    auto linesResult = sketch_.lines();
    auto circlesResult = sketch_.circles();

    if (!linesResult || !circlesResult) {
        return std::nullopt;
    }

    const auto& lines = linesResult.value();
    const auto& circles = circlesResult.value();

    std::optional<SnapCandidate> best;

    const auto considerPoint = [&](const Vec2& point, SnapKind kind, int priority, GeometryRef first, GeometryRef second) {
        if (!isFinite(point)) {
            return;
        }

        const double candidateDistance = distance(request.cursor, point);

        if (!isAllowed(kind, candidateDistance, rule)) {
            return;
        }

        auto candidate = makeCandidate(point, kind, candidateDistance, priority, {first, second});

        if (!best || isBetterCandidate(candidate, *best)) {
            best = std::move(candidate);
        }
    };

    // Line-line intersections.
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto* firstLine = std::get_if<Line2>(&lines[i].geometry);

        if (!firstLine || !isFinite(*firstLine)) {
            continue;
        }

        for (std::size_t j = i + 1; j < lines.size(); ++j) {
            const auto* secondLine = std::get_if<Line2>(&lines[j].geometry);

            if (!secondLine || !isFinite(*secondLine)) {
                continue;
            }

            const auto intersection = intersectInfiniteLines(*firstLine, *secondLine);

            if (!intersection) {
                continue;
            }

            const bool onBothSegments = isOnSegment(intersection->t) && isOnSegment(intersection->u);

            if (onBothSegments && allowReal) {
                considerPoint(intersection->point, SnapKind::Intersection, kIntersectionPriority, {lines[i].id, SubElement::Whole},
                              {lines[j].id, SubElement::Whole});
            } else if (!onBothSegments && allowExtended) {
                considerPoint(intersection->point, SnapKind::ExtendedIntersection, kIntersectionPriority, {lines[i].id, SubElement::Whole},
                              {lines[j].id, SubElement::Whole});
            }
        }
    }

    // Line-circle intersections.
    for (const auto& lineEntity : lines) {
        const auto* line = std::get_if<Line2>(&lineEntity.geometry);
        if (!line || !isFinite(*line)) {
            continue;
        }

        for (const auto& circleEntity : circles) {
            const auto* circle = std::get_if<Circle2>(&circleEntity.geometry);

            if (!circle || !isFinite(*circle)) {
                continue;
            }

            if (allowReal) {
                visitLineCircleIntersections(*line, *circle, true, [&](const Vec2& point) {
                    considerPoint(point, SnapKind::Intersection, kIntersectionPriority, {lineEntity.id, SubElement::Whole},
                                  {circleEntity.id, SubElement::Whole});
                });
            }

            if (allowExtended) {
                visitLineCircleIntersections(*line, *circle, false, [&](const Vec2& point) {
                    considerPoint(point, SnapKind::ExtendedIntersection, kIntersectionPriority, {lineEntity.id, SubElement::Whole},
                                  {circleEntity.id, SubElement::Whole});
                });
            }
        }
    }

    // Circle-circle intersections are real intersections; circles have no
    // natural infinite extension.
    if (allowReal) {
        for (std::size_t i = 0; i < circles.size(); ++i) {
            const auto* firstCircle = std::get_if<Circle2>(&circles[i].geometry);

            if (!firstCircle || !isFinite(*firstCircle)) {
                continue;
            }

            for (std::size_t j = i + 1; j < circles.size(); ++j) {
                const auto* secondCircle = std::get_if<Circle2>(&circles[j].geometry);

                if (!secondCircle || !isFinite(*secondCircle)) {
                    continue;
                }

                visitCircleCircleIntersections(*firstCircle, *secondCircle, [&](const Vec2& point) {
                    considerPoint(point, SnapKind::Intersection, kIntersectionPriority, {circles[i].id, SubElement::Whole}, {circles[j].id, SubElement::Whole});
                });
            }
        }
    }

    return best;
}

}  // namespace snap