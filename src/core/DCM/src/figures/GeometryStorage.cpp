#include "GeometryStorage.h"

#include <algorithm>
#include <ranges>

namespace OurPaintDCM::Figures {

std::pair<ID, Point2D*> GeometryStorage::createPoint(double x, double y) {
    ID id = m_idGen.nextID();
    
    m_points.emplace_back(x, y);
    Point2D* ptr = &m_points.back();
    
    m_index.emplace(id, FigureEntry{FigureType::ET_POINT2D, ptr});
    
    m_pointToID.emplace(ptr, id);
    
    return {id, ptr};
}

std::pair<ID, Line2D*> GeometryStorage::createLine(Point2D* p1, Point2D* p2) {
    ID id = m_idGen.nextID();
    
    m_lines.emplace_back(p1, p2);
    Line2D* ptr = &m_lines.back();
    
    m_index.emplace(id, FigureEntry{FigureType::ET_LINE, ptr});
    
    return {id, ptr};
}

std::pair<ID, Circle2D*> GeometryStorage::createCircle(Point2D* center, double radius) {
    ID id = m_idGen.nextID();
    
    m_circles.emplace_back(center, radius);
    Circle2D* ptr = &m_circles.back();
    
    m_index.emplace(id, FigureEntry{FigureType::ET_CIRCLE, ptr});
    
    return {id, ptr};
}

std::pair<ID, Arc2D*> GeometryStorage::createArc(Point2D* p1, Point2D* p2, Point2D* center) {
    ID id = m_idGen.nextID();
    
    m_arcs.emplace_back(p1, p2, center);
    Arc2D* ptr = &m_arcs.back();
    
    m_index.emplace(id, FigureEntry{FigureType::ET_ARC, ptr});
    
    return {id, ptr};
}

ID GeometryStorage::createFigure(FigureType type, const FigureData& data) {
    switch (type) {
        case FigureType::ET_POINT2D: {
            if (data.points.empty()) {
                throw std::runtime_error("Point data not provided");
            }
            const auto& p = data.points.front();
            auto result = createPoint(p.x, p.y);
            return result.first;
        }
        case FigureType::ET_LINE: {
            if (data.points.size() < 2) {
                throw std::runtime_error("Line requires two points");
            }
            const auto& p1 = data.points[0];
            const auto& p2 = data.points[1];
            auto point1 = createPoint(p1.x, p1.y);
            auto point2 = createPoint(p2.x, p2.y);
            auto lineResult = createLine(point1.second, point2.second);
            return lineResult.first;
        }
        case FigureType::ET_CIRCLE: {
            auto centerPoint = createPoint(data.center.x, data.center.y);
            auto circleResult = createCircle(centerPoint.second, data.radius);
            return circleResult.first;
        }
        case FigureType::ET_ARC: {
            if (data.points.size() < 2) {
                throw std::runtime_error("Arc requires two endpoints");
            }
            const auto& p1 = data.points[0];
            const auto& p2 = data.points[1];
            auto point1 = createPoint(p1.x, p1.y);
            auto point2 = createPoint(p2.x, p2.y);
            auto centerPoint = createPoint(data.center.x, data.center.y);
            auto arcResult = createArc(point1.second, point2.second, centerPoint.second);
            return arcResult.first;
        }
        default:
            throw std::runtime_error("Unknown figure type");
    }
}

void* GeometryStorage::getRaw(ID id) const noexcept {
    auto it = m_index.find(id);
    return (it != m_index.end()) ? it->second.ptr : nullptr;
}

std::optional<FigureType> GeometryStorage::getType(ID id) const noexcept {
    auto it = m_index.find(id);
    if (it != m_index.end()) {
        return it->second.type;
    }
    return std::nullopt;
}

std::optional<FigureEntry> GeometryStorage::getEntry(ID id) const noexcept {
    auto it = m_index.find(id);
    if (it != m_index.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool GeometryStorage::contains(ID id) const noexcept {
    return m_index.contains(id);
}


void GeometryStorage::remove(ID id, bool forceCascade) {
    auto it = m_index.find(id);
    if (it == m_index.end()) {
        throw std::runtime_error("ID not found");
    }
    
    const FigureEntry& entry = it->second;
    
    if (entry.type == FigureType::ET_POINT2D) {
        std::vector<ID> dependents = getDependents(id);
        
        if (!dependents.empty()) {
            if (!forceCascade) {
                throw std::runtime_error("Dependencies exist");
            }
            
            for (ID depId : dependents) {
                try {
                    remove(depId, false);
                } catch (const std::runtime_error&) {
                    // ignore if already removed
                }
            }
        }
        
        const Point2D* ptrPoint = static_cast<const Point2D*>(entry.ptr);
        m_pointToID.erase(ptrPoint);
    }
    
    m_index.erase(it);
    
    return;
}

void GeometryStorage::clear() noexcept {
    m_points.clear();
    m_lines.clear();
    m_circles.clear();
    m_arcs.clear();
    m_index.clear();
    m_pointToID.clear();
    m_idGen.reset();
}

std::span<Point2D> GeometryStorage::allPoints() noexcept {
    if (m_points.empty()) return {};
    return std::span<Point2D>{&m_points.front(), m_points.size()};
}

std::span<const Point2D> GeometryStorage::allPoints() const noexcept {
    if (m_points.empty()) return {};
    return std::span<const Point2D>{&m_points.front(), m_points.size()};
}

std::span<Line2D> GeometryStorage::allLines() noexcept {
    if (m_lines.empty()) return {};
    return std::span<Line2D>{&m_lines.front(), m_lines.size()};
}

std::span<const Line2D> GeometryStorage::allLines() const noexcept {
    if (m_lines.empty()) return {};
    return std::span<const Line2D>{&m_lines.front(), m_lines.size()};
}

std::span<Circle2D> GeometryStorage::allCircles() noexcept {
    if (m_circles.empty()) return {};
    return std::span<Circle2D>{&m_circles.front(), m_circles.size()};
}

std::span<const Circle2D> GeometryStorage::allCircles() const noexcept {
    if (m_circles.empty()) return {};
    return std::span<const Circle2D>{&m_circles.front(), m_circles.size()};
}

std::span<Arc2D> GeometryStorage::allArcs() noexcept {
    if (m_arcs.empty()) return {};
    return std::span<Arc2D>{&m_arcs.front(), m_arcs.size()};
}

std::span<const Arc2D> GeometryStorage::allArcs() const noexcept {
    if (m_arcs.empty()) return {};
    return std::span<const Arc2D>{&m_arcs.front(), m_arcs.size()};
}

std::vector<ID> GeometryStorage::getIDsByType(FigureType type) const {
    std::vector<ID> result;
    
    for (const auto& [id, entry] : m_index) {
        if (entry.type == type) {
            result.push_back(id);
        }
    }
    
    return result;
}

const std::unordered_map<ID, FigureEntry>& GeometryStorage::allEntries() const noexcept {
    return m_index;
}

std::size_t GeometryStorage::size() const noexcept {
    return m_index.size();
}

std::size_t GeometryStorage::pointCount() const noexcept {
    return std::ranges::count_if(m_index, [](const auto& pair) {
        return pair.second.type == FigureType::ET_POINT2D;
    });
}

std::size_t GeometryStorage::lineCount() const noexcept {
    return std::ranges::count_if(m_index, [](const auto& pair) {
        return pair.second.type == FigureType::ET_LINE;
    });
}

std::size_t GeometryStorage::circleCount() const noexcept {
    return std::ranges::count_if(m_index, [](const auto& pair) {
        return pair.second.type == FigureType::ET_CIRCLE;
    });
}

std::size_t GeometryStorage::arcCount() const noexcept {
    return std::ranges::count_if(m_index, [](const auto& pair) {
        return pair.second.type == FigureType::ET_ARC;
    });
}

bool GeometryStorage::empty() const noexcept {
    return m_index.empty();
}

const ID& GeometryStorage::currentID() const noexcept {
    return m_idGen.current();
}

std::vector<ID> GeometryStorage::getDependents(ID pointId) const {
    std::vector<ID> result;
    
    auto entryOpt = getEntry(pointId);
    if (!entryOpt || entryOpt->type != FigureType::ET_POINT2D) {
        return result;
    }
    
    const Point2D* targetPoint = static_cast<const Point2D*>(entryOpt->ptr);
    
    for (const auto& [id, entry] : m_index) {
        switch (entry.type) {
            case FigureType::ET_LINE: {
                const Line2D* line = static_cast<const Line2D*>(entry.ptr);
                if (line->p1 == targetPoint || line->p2 == targetPoint) {
                    result.push_back(id);
                }
                break;
            }
            case FigureType::ET_CIRCLE: {
                const Circle2D* circle = static_cast<const Circle2D*>(entry.ptr);
                if (circle->center == targetPoint) {
                    result.push_back(id);
                }
                break;
            }
            case FigureType::ET_ARC: {
                const Arc2D* arc = static_cast<const Arc2D*>(entry.ptr);
                if (arc->p1 == targetPoint || arc->p2 == targetPoint || arc->p_center == targetPoint) {
                    result.push_back(id);
                }
                break;
            }
            default:
                break;
        }
    }
    
    return result;
}

std::vector<ID> GeometryStorage::getDependencies(ID id) const {
    std::vector<ID> result;
    
    auto entryOpt = getEntry(id);
    if (!entryOpt) {
        return result;
    }
    
    std::vector<const Point2D*> pointPtrs;
    
    switch (entryOpt->type) {
        case FigureType::ET_POINT2D:
            break;
        case FigureType::ET_LINE: {
            const Line2D* line = static_cast<const Line2D*>(entryOpt->ptr);
            pointPtrs = {line->p1, line->p2};
            break;
        }
        case FigureType::ET_CIRCLE: {
            const Circle2D* circle = static_cast<const Circle2D*>(entryOpt->ptr);
            pointPtrs = {circle->center};
            break;
        }
        case FigureType::ET_ARC: {
            const Arc2D* arc = static_cast<const Arc2D*>(entryOpt->ptr);
            pointPtrs = {arc->p1, arc->p2, arc->p_center};
            break;
        }
    }
    
    for (const Point2D* ptr : pointPtrs) {
        auto idOpt = findPointID(ptr);
        if (idOpt) {
            result.push_back(*idOpt);
        }
    }
    
    return result;
}

std::optional<ID> GeometryStorage::findPointID(const Point2D* ptr) const noexcept {
    auto it = m_pointToID.find(ptr);
    if (it != m_pointToID.end()) {
        return it->second;
    }
    return std::nullopt;
}

ObjectGraph GeometryStorage::buildObjectGraph() const {
    ObjectGraph graph;
    const ID helperWeight = ID(static_cast<unsigned long long>(-1));

    for (const auto& [id, entry] : m_index) {
        graph.addVertex(id);
    }

    for (const auto& [id, entry] : m_index) {
        switch (entry.type) {
            case FigureType::ET_LINE: {
                const Line2D* line = static_cast<const Line2D*>(entry.ptr);
                if (auto p1 = findPointID(line->p1)) {
                    graph.addEdge(id, *p1, helperWeight);
                }
                if (auto p2 = findPointID(line->p2)) {
                    graph.addEdge(id, *p2, helperWeight);
                }
                break;
            }
            case FigureType::ET_CIRCLE: {
                const Circle2D* circle = static_cast<const Circle2D*>(entry.ptr);
                if (auto c = findPointID(circle->center)) {
                    graph.addEdge(id, *c, helperWeight);
                }
                break;
            }
            case FigureType::ET_ARC: {
                const Arc2D* arc = static_cast<const Arc2D*>(entry.ptr);
                if (auto p1 = findPointID(arc->p1)) {
                    graph.addEdge(id, *p1, helperWeight);
                }
                if (auto p2 = findPointID(arc->p2)) {
                    graph.addEdge(id, *p2, helperWeight);
                }
                if (auto c = findPointID(arc->p_center)) {
                    graph.addEdge(id, *c, helperWeight);
                }
                break;
            }
            default:
                break;
        }
    }

    return graph;
}

ObjectGraph GeometryStorage::buildObjectSubgraph(ID id) const {
    auto typeOpt = getType(id);
    if (!typeOpt) {
        throw std::runtime_error("ID not found");
    }

    ObjectGraph graph;
    const ID helperWeight = ID(static_cast<unsigned long long>(-1));

    graph.addVertex(id);

    if (*typeOpt == FigureType::ET_POINT2D) {
        auto dependents = getDependents(id);
        for (const ID depId : dependents) {
            graph.addVertex(depId);
            graph.addEdge(id, depId, helperWeight);
            auto depPoints = getDependencies(depId);
            for (const ID pointId : depPoints) {
                graph.addVertex(pointId);
                graph.addEdge(depId, pointId, helperWeight);
            }
        }
        return graph;
    }

    auto dependencies = getDependencies(id);
    for (const ID depId : dependencies) {
        graph.addVertex(depId);
        graph.addEdge(id, depId, helperWeight);
    }

    return graph;
}

}

