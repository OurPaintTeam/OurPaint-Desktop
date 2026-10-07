#include "SnapSystem.h"

#include "geometry/Algoritms.h"

namespace snap {

SnapResult SnapSystem::getSnapCandidate(
    const SnapRequest& request
) const
{
    // Пока разбираем только инструмент Point.
    if (request.tool == ToolType::Point)
    {
        if (const auto candidate = findPointCandidate(request))
            return makeResult(*candidate);
    }

    // Пока snap не найден.
    return {};
}


std::optional<SnapSystem::SnapCandidate>
SnapSystem::findPointCandidate(
    const SnapRequest& request
) const
{
    std::optional<SnapCandidate> bestCandidate;

    for (const auto& object : request.objects)
    {
        if (isExcluded(
            object.id,
            request.excludedObjects))
        {
            continue;
        }

        // Пока ищем только объекты типа Point.
        const Point* point =
            std::get_if<Point>(&object.geometry);

        if (!point)
            continue;

        const auto result =
            geometry::closestPoint(
                request.cursor.x,
                request.cursor.y
            );

        const Point candidatePoint{
            result.first,
            result.second
        };

        const double dx =
            request.cursor.x - candidatePoint.x;

        const double dy =
            request.cursor.y - candidatePoint.y;

        const double distance =
            std::sqrt(dx * dx + dy * dy);

        if (!bestCandidate ||
            distance < bestCandidate->distance)
        {
            SnapCandidate candidate;

            candidate.point = candidatePoint;
            candidate.distance = distance;
            candidate.type = SnapType::Point;

            candidate.objectIds.push_back(object.id);

            bestCandidate = std::move(candidate);
        }
    }

    return bestCandidate;
}


bool SnapSystem::isExcluded(
    ObjectId objectId,
    std::span<const ObjectId> excludedObjects
) {
    for (const ObjectId excludedId : excludedObjects)
    {
        if (excludedId == objectId)
            return true;
    }

    return false;
}


SnapResult SnapSystem::makeResult(
    const SnapCandidate& candidate
) {
    SnapResult result;

    result.snapped = true;
    result.point = candidate.point;
    result.type = candidate.type;

    result.objectIds = candidate.objectIds;
    result.guideLines = candidate.guideLines;
    result.previewObject = candidate.previewObject;
    result.constraint = candidate.constraint;

    return result;
}

} // namespace snap