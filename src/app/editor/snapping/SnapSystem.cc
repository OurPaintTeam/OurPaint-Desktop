#include "SnapSystem.h"

#include <limits>
#include <utility>

#include "geometry/Algorithms.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"

namespace snap {

SnapResult SnapSystem::getSnapCandidate(const SnapRequest& request) const {
    std::optional<SnapResult> best;

    auto consider = [&best](std::optional<SnapResult> candidate) {
        if (!candidate) {
            return;
        }

        if (!best || candidate->score < best->score) {
            best = std::move(candidate);
        }
    };

    consider(findPointCandidate(request));
    consider(findMidpointCandidate(request));
    consider(findIntersectionCandidate(request));
    consider(findExtendedIntersectionCandidate(request));

    return best ? *best : SnapResult{};
}

// ==================== Point ====================

std::optional<SnapResult> SnapSystem::findPointCandidate(
    const SnapRequest& request) const {

    const auto elementsResult = sketch_.pointElements(
        core::sketch::PointElementScope::All);

    if (!elementsResult) {
        return std::nullopt;
    }

    const auto& elements = elementsResult.value();

    std::optional<SnapResult> bestCandidate;
    double bestScore = std::numeric_limits<double>::max();

    const core::sketch::Vec2 cursor{
        request.cursor.x,
        request.cursor.y
    };

    for (const auto& element : elements) {
        const auto& position = element.position;

        const double distance = geometry::distance(cursor, position);
        const double score = distance;

        if (score >= bestScore) {
            continue;
        }

        SnapResult candidate;

        candidate.snapped = true;
        candidate.distance = distance;
        candidate.score = score;
        candidate.point = Point{position.x, position.y};
        candidate.type = SnapType::Point;

        // Сохраняем ссылку именно на точку/подэлемент,
        // а не только ID родительской геометрии.
        candidate.objectIds.push_back(element.ref);

        bestScore = score;
        bestCandidate = std::move(candidate);
    }

    return bestCandidate;
}

// ==================== Midpoint ====================

std::optional<SnapResult> SnapSystem::findMidpointCandidate(
    const SnapRequest& request) const {

    const auto linesResult = sketch_.lines();

    if (!linesResult) {
        return std::nullopt;
    }

    const auto& lines = linesResult.value();

    std::optional<SnapResult> bestCandidate;
    double bestScore = std::numeric_limits<double>::max();

    const core::sketch::Vec2 cursor{
        request.cursor.x,
        request.cursor.y
    };

    for (const auto& entity : lines) {
        const auto* lineGeometry =
            std::get_if<core::sketch::Line2>(&entity.geometry);

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

        // Середина не является Start/End подэлементом,
        // поэтому указываем родительскую линию целиком.
        candidate.objectIds.push_back({
            entity.id,
            core::sketch::SubElement::Whole
        });

        bestScore = score;
        bestCandidate = std::move(candidate);
    }

    return bestCandidate;
}

// ==================== Intersection ====================

std::optional<SnapResult> SnapSystem::findIntersectionCandidate(
    const SnapRequest& request) const {

    const auto linesResult = sketch_.lines();

    if (!linesResult) {
        return std::nullopt;
    }

    const auto& lines = linesResult.value();

    std::optional<SnapResult> bestCandidate;
    double bestScore = std::numeric_limits<double>::max();

    const core::sketch::Vec2 cursor{
        request.cursor.x,
        request.cursor.y
    };

    // O(N^2)
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto* geomA =
            std::get_if<core::sketch::Line2>(&lines[i].geometry);

        if (!geomA) {
            continue;
        }

        for (std::size_t j = i + 1; j < lines.size(); ++j) {
            const auto* geomB =
                std::get_if<core::sketch::Line2>(&lines[j].geometry);

            if (!geomB) {
                continue;
            }

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

            // Пересечение задаётся двумя участвующими линиями.
            candidate.objectIds = {
                {
                    lines[i].id,
                    core::sketch::SubElement::Whole
                },
                {
                    lines[j].id,
                    core::sketch::SubElement::Whole
                }
            };

            bestScore = score;
            bestCandidate = std::move(candidate);
        }
    }

    return bestCandidate;
}

// ==================== ExtendedIntersection ====================

std::optional<SnapResult> SnapSystem::findExtendedIntersectionCandidate(
    const SnapRequest& request) const {

    const auto linesResult = sketch_.lines();

    if (!linesResult) {
        return std::nullopt;
    }

    const auto& lines = linesResult.value();

    std::optional<SnapResult> bestCandidate;
    double bestScore = std::numeric_limits<double>::max();

    const core::sketch::Vec2 cursor{
        request.cursor.x,
        request.cursor.y
    };

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto* geomA =
            std::get_if<core::sketch::Line2>(&lines[i].geometry);

        if (!geomA) {
            continue;
        }

        for (std::size_t j = i + 1; j < lines.size(); ++j) {
            const auto* geomB =
                std::get_if<core::sketch::Line2>(&lines[j].geometry);

            if (!geomB) {
                continue;
            }

            // Пересечение отрезков уже обрабатывается выше.
            if (geometry::intersectSegments(*geomA, *geomB)) {
                continue;
            }

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

            candidate.objectIds = {
                {
                    lines[i].id,
                    core::sketch::SubElement::Whole
                },
                {
                    lines[j].id,
                    core::sketch::SubElement::Whole
                }
            };

            // Виртуальные направляющие для визуализации.
            candidate.guideLines.push_back(
                core::sketch::Line2{
                    .start = ext->startA,
                    .end = hit
                });

            candidate.guideLines.push_back(
                core::sketch::Line2{
                    .start = ext->startB,
                    .end = hit
                });

            bestScore = score;
            bestCandidate = std::move(candidate);
        }
    }

    return bestCandidate;
}

}  // namespace snap