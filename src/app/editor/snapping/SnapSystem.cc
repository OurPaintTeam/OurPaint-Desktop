#include "SnapSystem.h"

#include <limits>
#include <utility>

#include "geometry/Algorithms.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"

namespace snap {

SnapResult SnapSystem::getSnapCandidate(const SnapRequest& request) const {
    std::optional<SnapResult> best;

    // Выбираем кандидата с минимальным score.
    auto consider = [&best](std::optional<SnapResult> c) {
        if (!c) {
            return;
        }
        if (!best || c->score < best->score) {
            best = std::move(c);
        }
    };

    // От дешёвых стратегий к дорогим.
    consider(findPointCandidate(request));
    consider(findMidpointCandidate(request));
    consider(findIntersectionCandidate(request));
    consider(findExtendedIntersectionCandidate(request));

    return best ? *best : SnapResult{};
}

// ==================== Point ====================

std::optional<SnapResult> SnapSystem::findPointCandidate(const SnapRequest& request) const {
    const auto pointsResult = sketch_.points();

    if (!pointsResult) {
        return std::nullopt;
    }

    const auto& points = pointsResult.value();

    std::optional<SnapResult> bestCandidate;
    double bestScore = std::numeric_limits<double>::max();

    const core::sketch::Vec2 cursor{request.cursor.x, request.cursor.y};

    for (const auto& entity : points) {
        const core::ID objectId(entity.id.get());

        /*if (isExcluded(objectId, request.excludedObjects)) {
            continue;
        }*/

        const auto& pointGeometry = std::get<core::sketch::Point2>(entity.geometry);
        const auto point = pointGeometry.position;

        const double distance = geometry::distance(cursor, point);
        const double score = distance;

        if (score >= bestScore) {
            continue;
        }

        SnapResult candidate;

        candidate.snapped = true;
        candidate.distance = distance;
        candidate.score = score;
        candidate.point = Point{point.x, point.y};
        candidate.type = SnapType::Point;

        candidate.objectIds.push_back(objectId);

        bestScore = score;
        bestCandidate = std::move(candidate);
    }

    return bestCandidate;
}

// ==================== Midpoint ====================

std::optional<SnapResult> SnapSystem::findMidpointCandidate(const SnapRequest& request) const {
    const auto linesResult = sketch_.lines();

    if (!linesResult) {
        return std::nullopt;
    }

    const auto& lines = linesResult.value();

    std::optional<SnapResult> bestCandidate;
    double bestScore = std::numeric_limits<double>::max();

    const core::sketch::Vec2 cursor{request.cursor.x, request.cursor.y};

    for (const auto& entity : lines) {
        const core::ID objectId(entity.id.get());

        /*if (isExcluded(objectId, request.excludedObjects)) {
            continue;
        }*/

        const auto* lineGeometry = std::get_if<core::sketch::Line2>(&entity.geometry);
        if (!lineGeometry) {
            continue;
        }

        const auto mid = geometry::midpoint(*lineGeometry);
        if (!mid) {
            continue;
        }

        const double distance = geometry::distance(cursor, *mid);
        const double score = distance;

        if (score >= bestScore) {
            continue;
        }

        SnapResult candidate;

        candidate.snapped = true;
        candidate.distance = distance;
        candidate.score = score;
        candidate.point = Point{mid->x, mid->y};
        candidate.type = SnapType::Midpoint;

        candidate.objectIds.push_back(objectId);

        bestScore = score;
        bestCandidate = std::move(candidate);
    }

    return bestCandidate;
}

// ==================== Intersection ====================

std::optional<SnapResult> SnapSystem::findIntersectionCandidate(const SnapRequest& request) const {
    const auto linesResult = sketch_.lines();

    if (!linesResult) {
        return std::nullopt;
    }

    const auto& lines = linesResult.value();

    std::optional<SnapResult> bestCandidate;
    double bestScore = std::numeric_limits<double>::max();

    const core::sketch::Vec2 cursor{request.cursor.x, request.cursor.y};

    // O(N^2)
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const core::ID idA(lines[i].id.get());

        /*if (isExcluded(idA, request.excludedObjects)) {
            continue;
        }*/

        const auto* geomA = std::get_if<core::sketch::Line2>(&lines[i].geometry);
        if (!geomA) {
            continue;
        }

        for (std::size_t j = i + 1; j < lines.size(); ++j) {
            const core::ID idB(lines[j].id.get());

            /*
            if (isExcluded(idB, request.excludedObjects)) {
                continue;
            }*/

            const auto* geomB = std::get_if<core::sketch::Line2>(&lines[j].geometry);
            if (!geomB) {
                continue;
            }

            // Пересечение именно отрезков.
            const auto hit = geometry::intersectSegments(*geomA, *geomB);
            if (!hit) {
                continue;
            }

            const double distance = geometry::distance(cursor, *hit);
            const double score = distance;

            if (score >= bestScore) {
                continue;
            }

            SnapResult candidate;

            candidate.snapped = true;
            candidate.distance = distance;
            candidate.score = score;
            candidate.point = Point{hit->x, hit->y};
            candidate.type = SnapType::Intersection;

            candidate.objectIds = {idA, idB};

            bestScore = score;
            bestCandidate = std::move(candidate);
        }
    }

    return bestCandidate;
}

// ==================== ExtendedIntersection ====================

std::optional<SnapResult> SnapSystem::findExtendedIntersectionCandidate(const SnapRequest& request) const {
    const auto linesResult = sketch_.lines();

    if (!linesResult) {
        return std::nullopt;
    }

    const auto& lines = linesResult.value();

    std::optional<SnapResult> bestCandidate;
    double bestScore = std::numeric_limits<double>::max();

    const core::sketch::Vec2 cursor{request.cursor.x, request.cursor.y};

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const core::ID idA(lines[i].id.get());

        /*
        if (isExcluded(idA, request.excludedObjects)) {
            continue;
        }*/

        const auto* geomA = std::get_if<core::sketch::Line2>(&lines[i].geometry);
        if (!geomA) {
            continue;
        }

        for (std::size_t j = i + 1; j < lines.size(); ++j) {
            const core::ID idB(lines[j].id.get());

            /*if (isExcluded(idB, request.excludedObjects)) {
                continue;
            }*/

            const auto* geomB = std::get_if<core::sketch::Line2>(&lines[j].geometry);
            if (!geomB) {
                continue;
            }

            // Пересечение именно отрезков — уже покрыто обычной привязкой.
            if (geometry::intersectSegments(*geomA, *geomB)) {
                continue;
            }

            // Пересечение бесконечных прямых.
            const auto ext = geometry::intersectLines(*geomA, *geomB);
            if (!ext) {
                continue;
            }

            const core::sketch::Vec2 hit = ext->intersection;

            const double distance = geometry::distance(cursor, hit);
            const double score = distance;

            if (score >= bestScore) {
                continue;
            }

            SnapResult candidate;

            candidate.snapped = true;
            candidate.distance = distance;
            candidate.score = score;
            candidate.point = Point{hit.x, hit.y};
            candidate.type = SnapType::ExtendedIntersection;

            candidate.objectIds = {idA, idB};

            // Виртуальные продолжения для визуализации.
            // Отрезки от начал до точки пересечения — это и есть продолжения.
            candidate.guideLines.push_back(core::sketch::Line2{.start = ext->startA, .end = hit});
            candidate.guideLines.push_back(core::sketch::Line2{.start = ext->startB, .end = hit});

            bestScore = score;
            bestCandidate = std::move(candidate);
        }
    }

    return bestCandidate;
}

// ==================== Helpers ====================

bool SnapSystem::isExcluded(const core::ID objectId, const std::span<const core::ID> excludedObjects) {
    for (const auto excludedId : excludedObjects) {
        if (excludedId == objectId) {
            return true;
        }
    }

    return false;
}

}  // namespace snap