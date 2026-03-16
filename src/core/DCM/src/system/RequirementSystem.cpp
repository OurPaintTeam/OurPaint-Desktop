#include "system/RequirementSystem.h"
#include <stdexcept>

namespace OurPaintDCM::System {

RequirementSystem::RequirementSystem(Figures::GeometryStorage* storage)
    : _storage(storage) {}

Utils::ID RequirementSystem::addRequirement(const Utils::RequirementDescriptor& descriptor) {
    descriptor.validate();

    const auto& ids = descriptor.objectIds;
    std::shared_ptr<Function::RequirementFunction> func;

    switch (descriptor.type) {
        case Utils::RequirementType::ET_POINTLINEDIST: {
            auto* point = _storage->get<Figures::Point2D>(ids[0]);
            auto* line = _storage->get<Figures::Line2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createPointLineDist(point, line, descriptor.param.value());
            break;
        }
        case Utils::RequirementType::ET_POINTONLINE: {
            auto* point = _storage->get<Figures::Point2D>(ids[0]);
            auto* line = _storage->get<Figures::Line2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createPointOnLine(point, line);
            break;
        }
        case Utils::RequirementType::ET_POINTPOINTDIST: {
            auto* p1 = _storage->get<Figures::Point2D>(ids[0]);
            auto* p2 = _storage->get<Figures::Point2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createPointPointDist(p1, p2, descriptor.param.value());
            break;
        }
        case Utils::RequirementType::ET_POINTONPOINT: {
            auto* p1 = _storage->get<Figures::Point2D>(ids[0]);
            auto* p2 = _storage->get<Figures::Point2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createPointOnPoint(p1, p2);
            break;
        }
        case Utils::RequirementType::ET_LINECIRCLEDIST: {
            auto* line = _storage->get<Figures::Line2D>(ids[0]);
            auto* circle = _storage->get<Figures::Circle2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createLineCircleDist(line, circle, descriptor.param.value());
            break;
        }
        case Utils::RequirementType::ET_LINEONCIRCLE: {
            auto* line = _storage->get<Figures::Line2D>(ids[0]);
            auto* circle = _storage->get<Figures::Circle2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createLineOnCircle(line, circle);
            break;
        }
        case Utils::RequirementType::ET_LINEINCIRCLE: {
            // Factory method for LineInCircle might not exist yet
            throw std::runtime_error("LineInCircle requirement is not yet supported via unified interface");
        }
        case Utils::RequirementType::ET_LINELINEPARALLEL: {
            auto* l1 = _storage->get<Figures::Line2D>(ids[0]);
            auto* l2 = _storage->get<Figures::Line2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createLineLineParallel(l1, l2);
            break;
        }
        case Utils::RequirementType::ET_LINELINEPERPENDICULAR: {
            auto* l1 = _storage->get<Figures::Line2D>(ids[0]);
            auto* l2 = _storage->get<Figures::Line2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createLineLinePerpendicular(l1, l2);
            break;
        }
        case Utils::RequirementType::ET_LINELINEANGLE: {
            auto* l1 = _storage->get<Figures::Line2D>(ids[0]);
            auto* l2 = _storage->get<Figures::Line2D>(ids[1]);
            func = Function::RequirementFunctionFactory::createLineLineAngle(l1, l2, descriptor.param.value());
            break;
        }
        case Utils::RequirementType::ET_VERTICAL: {
            auto* line = _storage->get<Figures::Line2D>(ids[0]);
            func = Function::RequirementFunctionFactory::createVertical(line);
            break;
        }
        case Utils::RequirementType::ET_HORIZONTAL: {
            auto* line = _storage->get<Figures::Line2D>(ids[0]);
            func = Function::RequirementFunctionFactory::createHorizontal(line);
            break;
        }
        case Utils::RequirementType::ET_ARCCENTERONPERPENDICULAR: {
            auto* arc = _storage->get<Figures::Arc2D>(ids[0]);
            func = Function::RequirementFunctionFactory::createArcCenterOnPerpendicular(arc);
            break;
        }
    }

    addFunction(func);

    Utils::ID reqId = _reqIdGen.nextID();
    _requirements.push_back({reqId, descriptor.type, descriptor.objectIds, descriptor.param});

    return reqId;
}

const RequirementSystem::RequirementEntry* RequirementSystem::getRequirement(Utils::ID reqId) const noexcept {
    for (const auto& entry : _requirements) {
        if (entry.id == reqId) {
            return &entry;
        }
    }
    return nullptr;
}

bool RequirementSystem::hasRequirement(Utils::ID reqId) const noexcept {
    return getRequirement(reqId) != nullptr;
}

std::optional<Utils::RequirementType> RequirementSystem::getRequirementType(Utils::ID reqId) const noexcept {
    const auto* entry = getRequirement(reqId);
    if (entry) {
        return entry->type;
    }
    return std::nullopt;
}

std::optional<std::vector<Utils::ID>> RequirementSystem::getRequirementObjectIds(Utils::ID reqId) const noexcept {
    const auto* entry = getRequirement(reqId);
    if (entry) {
        return entry->objectIds;
    }
    return std::nullopt;
}

std::optional<double> RequirementSystem::getRequirementParam(Utils::ID reqId) const noexcept {
    const auto* entry = getRequirement(reqId);
    if (entry) {
        return entry->param;
    }
    return std::nullopt;
}

void RequirementSystem::addPointLineDist(Utils::ID pointId, Utils::ID lineId, double dist) {
    addRequirement(Utils::RequirementDescriptor::pointLineDist(pointId, lineId, dist));
}

void RequirementSystem::addPointOnLine(Utils::ID pointId, Utils::ID lineId) {
    addRequirement(Utils::RequirementDescriptor::pointOnLine(pointId, lineId));
}

void RequirementSystem::addPointPointDist(Utils::ID p1Id, Utils::ID p2Id, double dist) {
    addRequirement(Utils::RequirementDescriptor::pointPointDist(p1Id, p2Id, dist));
}

void RequirementSystem::addPointOnPoint(Utils::ID p1Id, Utils::ID p2Id) {
    addRequirement(Utils::RequirementDescriptor::pointOnPoint(p1Id, p2Id));
}

void RequirementSystem::addLineCircleDist(Utils::ID lineId, Utils::ID circleId, double dist) {
    addRequirement(Utils::RequirementDescriptor::lineCircleDist(lineId, circleId, dist));
}

void RequirementSystem::addLineOnCircle(Utils::ID lineId, Utils::ID circleId) {
    addRequirement(Utils::RequirementDescriptor::lineOnCircle(lineId, circleId));
}

void RequirementSystem::addLineLineParallel(Utils::ID l1Id, Utils::ID l2Id) {
    addRequirement(Utils::RequirementDescriptor::lineLineParallel(l1Id, l2Id));
}

void RequirementSystem::addLineLinePerpendicular(Utils::ID l1Id, Utils::ID l2Id) {
    addRequirement(Utils::RequirementDescriptor::lineLinePerpendicular(l1Id, l2Id));
}

void RequirementSystem::addLineLineAngle(Utils::ID l1Id, Utils::ID l2Id, double angle) {
    addRequirement(Utils::RequirementDescriptor::lineLineAngle(l1Id, l2Id, angle));
}

void RequirementSystem::addVertical(Utils::ID lineId) {
    addRequirement(Utils::RequirementDescriptor::vertical(lineId));
}

void RequirementSystem::addHorizontal(Utils::ID lineId) {
    addRequirement(Utils::RequirementDescriptor::horizontal(lineId));
}

void RequirementSystem::addArcCenterOnPerpendicular(Utils::ID arcId) {
    addRequirement(Utils::RequirementDescriptor::arcCenterOnPerpendicular(arcId));
}

void RequirementSystem::clear() {
    RequirementFunctionSystem::clear();
    _requirements.clear();
}

Figures::ObjectGraph RequirementSystem::buildDependencyGraph() const {
    auto graph = _storage->buildObjectGraph();
    for (const auto& req : _requirements) {
        for (std::size_t i = 0; i < req.objectIds.size(); ++i) {
            for (std::size_t j = i + 1; j < req.objectIds.size(); ++j) {
                graph.addEdge(req.objectIds[i], req.objectIds[j], req.id);
            }
        }
    }

    return graph;
}

} // namespace OurPaintDCM::System
