#ifndef OURPAINTDCM_HEADERS_FIGURES_GEOMETRYSTORAGE_H
#define OURPAINTDCM_HEADERS_FIGURES_GEOMETRYSTORAGE_H

#include <deque>
#include <unordered_map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <ranges>
#include <concepts>
#include <type_traits>
#include <functional>
#include <vector>
#include <stdexcept>

#include "Point2D.h"
#include "Line.h"
#include "Circle.h"
#include "Arc.h"
#include "ID.h"
#include "IDGenerator.h"
#include "utils/Enums.h"
#include "Graph.h"

namespace OurPaintDCM::Figures {

using Utils::ID;
using Utils::IDGenerator;
using Utils::FigureType;
using Line2D   = Line<Point2D>;
using Circle2D = Circle<Point2D>;
using Arc2D    = Arc<Point2D>;
using ObjectGraph = Graph<ID, ID, UndirectedPolicy, WeightedPolicy>;

/**
 * @brief Concept to check if a type is a supported geometric figure.
 * 
 * Allows template functions to be used only with allowed types.
 * Errors will be caught at compile time with clear messages.
 */
template <typename T>
concept SupportedFigure = std::same_as<T, Point2D> ||
                          std::same_as<T, Line2D>  ||
                          std::same_as<T, Circle2D> ||
                          std::same_as<T, Arc2D>;

/**
 * @brief Structure to store metadata about a geometric object.
 * 
 * Stores:
 * - type: figure type (Point, Line, Circle, Arc)
 * - ptr: raw pointer to object (void* for universality)
 * 
 * Why void*? This allows storing all types in a single unordered_map.
 * When retrieving, we know the type from `type` field and perform safe static_cast.
 */
struct FigureEntry {
    FigureType type;  ///< Type of geometric object
    void*      ptr;   ///< Pointer to the object itself
    
    constexpr FigureEntry() noexcept : type{}, ptr{nullptr} {}
    constexpr FigureEntry(FigureType t, void* p) noexcept : type(t), ptr(p) {}
};

/**
 * @brief Input data for figure creation without exposing internal types.
 */
struct FigureData {
    struct PointData {
        double x{};
        double y{};
    };
    std::vector<PointData> points{};
    PointData center{};
    double radius{};
};


/**
 * @brief Storage for geometric objects with unified identification system.
 
 * ## Usage Example
 * @code
 * GeometryStorage storage;
 * 
 * // Create points
 * auto [id1, p1] = storage.createPoint(0.0, 5.0);
 * auto [id2, p2] = storage.createPoint(10.0, 5.0);
 * 
 * // Create line between points
 * auto [lineId, line] = storage.createLine(p1, p2);
 * 
 * // Get object by ID
 * Point2D* point = storage.get<Point2D>(id1).value();
 * 
 * // Safe retrieval with type checking
 * auto result = storage.get<Line2D>(lineId);
 * if (result) {
 *     Line2D* myLine = result.value();
 * }
 * @endcode
 */
class GeometryStorage {
public:
    /**
     * @brief Default constructor.
     * 
     * Creates empty storage with ID generator starting from 1.
     */
    GeometryStorage() = default;
    
    /// @brief Default destructor (deque will free memory itself).
    ~GeometryStorage() = default;
    
    /// @brief Copying disabled (storage contains unique objects).
    GeometryStorage(const GeometryStorage&) = delete;
    GeometryStorage& operator=(const GeometryStorage&) = delete;
    
    /// @brief Move allowed.
    GeometryStorage(GeometryStorage&&) noexcept = default;
    GeometryStorage& operator=(GeometryStorage&&) noexcept = default;
    
    /**
     * @brief Create point with given coordinates.
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @return Pair {ID, pointer to created point}
     * 
     * Complexity: O(1) amortized (push_back in deque + insert in map)
     */
    [[nodiscard]] std::pair<ID, Point2D*> createPoint(double x, double y);
    
    /**
     * @brief Create line between two points.
     * 
     * @param p1 Pointer to first point (must belong to this storage)
     * @param p2 Pointer to second point
     * @return Pair {ID, pointer to created line}
     * 
     * @warning Passed points must be created in THIS storage.
     *          Otherwise behavior is undefined on deletion.
     */
    [[nodiscard]] std::pair<ID, Line2D*> createLine(Point2D* p1, Point2D* p2);
    
    /**
     * @brief Create circle.
     * 
     * @param center Pointer to center (point from this storage)
     * @param radius Circle radius
     * @return Pair {ID, pointer to created circle}
     */
    [[nodiscard]] std::pair<ID, Circle2D*> createCircle(Point2D* center, double radius);
    
    /**
     * @brief Create arc.
     * 
     * @param p1 First endpoint of arc
     * @param p2 Second endpoint of arc
     * @param center Center of arc
     * @return Pair {ID, pointer to created arc}
     */
    [[nodiscard]] std::pair<ID, Arc2D*> createArc(Point2D* p1, Point2D* p2, Point2D* center);
    
    /**
     * @brief Create figure without exposing concrete types.
     * 
     * @param type Figure type to create
     * @param data Input data (coordinates and radius)
     * @return ID of created figure
     * @throws std::runtime_error when data invalid or type unknown
     */
    [[nodiscard]] ID createFigure(FigureType type, const FigureData& data);
    
    /**
     * @brief Get object by ID with type checking.
     * 
     * @tparam T Expected object type (Point2D, Line2D, Circle2D, Arc2D)
     * @param id Object identifier
     * @return T* if found
     * @throws std::runtime_error if ID missing or type mismatch
     * 
     * Complexity: O(1) — lookup in unordered_map
     * 
     */
    template <SupportedFigure T>
    [[nodiscard]] T* get(ID id) const;
    
    /**
     * @brief Get object by ID without type checking (unsafe).
     * 
     * @param id Object identifier
     * @return void* or nullptr if not found
     * 
     * @warning Caller must know the object type!
     * 
     * Useful when type is already known from another source (e.g., from FigureEntry).
     */
    [[nodiscard]] void* getRaw(ID id) const noexcept;
    
    /**
     * @brief Get object type by ID.
     * 
     * @param id Object identifier
     * @return std::optional<FigureType> — type or nullopt if ID not found
     */
    [[nodiscard]] std::optional<FigureType> getType(ID id) const noexcept;
    
    /**
     * @brief Get full figure entry.
     * 
     * @param id Object identifier
     * @return std::optional<FigureEntry> — entry or nullopt if not found
     */
    [[nodiscard]] std::optional<FigureEntry> getEntry(ID id) const noexcept;
    
    /**
     * @brief Check if object exists.
     * 
     * @param id Object identifier
     * @return true if object exists
     */
    [[nodiscard]] bool contains(ID id) const noexcept;

    /**
     * @brief Remove object by ID.
     * 
     * @param id Identifier of object to remove
     * @param forceCascade If true — also removes dependent objects.
     *                     If false — returns DEPENDENCY_EXISTS error if dependencies exist.
     * @throws std::runtime_error when ID not found or dependencies block removal
     * 
     * ## Algorithm
     * 1. Check existence
     * 2. If Point2D, check dependencies (Line/Circle/Arc that use it)
     * 3. If forceCascade=false and dependencies exist — error
     * 4. If forceCascade=true — first remove dependent objects
     * 5. Mark slot in deque as "deleted" (lazy deletion)
     * 
     * ## Why lazy deletion?
     * Real deletion from middle of deque is O(n).
     * We just remove entry from index. Object remains in memory,
     * but is inaccessible. Can implement compaction if needed.
     */
    void remove(ID id, bool forceCascade = false);
    
    /**
     * @brief Clear entire storage.
     * 
     * Removes all objects and resets ID generator.
     */
    void clear() noexcept;

    /**
     * @brief Get span of all points.
     * 
     * @return std::span<Point2D> — contiguous view of all points
     * 
     * @note Span invalidates when adding new points!
     *       Use for read-only iteration.
     */
    [[nodiscard]] std::span<Point2D> allPoints() noexcept;
    [[nodiscard]] std::span<const Point2D> allPoints() const noexcept;
    
    [[nodiscard]] std::span<Line2D> allLines() noexcept;
    [[nodiscard]] std::span<const Line2D> allLines() const noexcept;
    
    [[nodiscard]] std::span<Circle2D> allCircles() noexcept;
    [[nodiscard]] std::span<const Circle2D> allCircles() const noexcept;
    
    [[nodiscard]] std::span<Arc2D> allArcs() noexcept;
    [[nodiscard]] std::span<const Arc2D> allArcs() const noexcept;
    
    /**
     * @brief Get all IDs of specified type.
     * 
     * @param type Figure type for filtering
     * @return Vector of IDs of all objects of this type
     * 
     * Uses std::ranges for efficient filtering.
     */
    [[nodiscard]] std::vector<ID> getIDsByType(FigureType type) const;
    
    /**
     * @brief Get all entries (ID + Entry).
     * 
     * @return Const reference to internal index
     * 
     * Allows iterating over all objects:
     * @code
     * for (const auto& [id, entry] : storage.allEntries()) {
     *     std::cout << "ID: " << id.id << ", Type: " << static_cast<int>(entry.type) << "\n";
     * }
     * @endcode
     */
    [[nodiscard]] const std::unordered_map<ID, FigureEntry>& allEntries() const noexcept;
    
    /**
     * @brief Total number of objects in storage.
     */
    [[nodiscard]] std::size_t size() const noexcept;
    
    /**
     * @brief Number of points.
     */
    [[nodiscard]] std::size_t pointCount() const noexcept;
    
    /**
     * @brief Number of lines.
     */
    [[nodiscard]] std::size_t lineCount() const noexcept;
    
    /**
     * @brief Number of circles.
     */
    [[nodiscard]] std::size_t circleCount() const noexcept;
    
    /**
     * @brief Number of arcs.
     */
    [[nodiscard]] std::size_t arcCount() const noexcept;
    
    /**
     * @brief Check if empty.
     */
    [[nodiscard]] bool empty() const noexcept;
    
    /**
     * @brief Get current ID generator value.
     * 
     * Useful for serialization/deserialization.
     */
    [[nodiscard]] const ID& currentID() const noexcept;
    /**
     * @brief Find all objects depending on given point.
     * 
     * @param pointId Point ID
     * @return Vector of IDs of objects (Line, Circle, Arc) using this point
     * 
     * Useful before deleting point to see what will "break".
     */
    [[nodiscard]] std::vector<ID> getDependents(ID pointId) const;
    
    /**
     * @brief Find all points that object depends on.
     * 
     * @param id Object ID (Line, Circle, Arc)
     * @return Vector of IDs of points referenced by object
     */
    [[nodiscard]] std::vector<ID> getDependencies(ID id) const;

    /**
     * @brief Build graph of objects where vertices are IDs and edges connect related objects.
     * 
     * Edge weight is ID(-1) as helper value for connected objects
     */
    [[nodiscard]] ObjectGraph buildObjectGraph() const;
    
    /**
     * @brief Build shallow subgraph for a given object ID (one-level neighborhood).
     * 
     * Includes the object and its direct dependencies/dependents only.
     * @throws std::runtime_error if ID not found
     */
    [[nodiscard]] ObjectGraph buildObjectSubgraph(ID id) const;

private:
    /**
     * @brief Storage for each object type.
     * 
     * We use deque instead of vector because:
     * - On push_back, pointers to EXISTING elements are NOT invalidated
     * - This is critical since Line/Circle/Arc store Point2D*
     * - Vector on reallocation would move all Point2D, breaking all pointers
     */
    std::deque<Point2D>  m_points;
    std::deque<Line2D>   m_lines;
    std::deque<Circle2D> m_circles;
    std::deque<Arc2D>    m_arcs;
    
    /**
     * @brief Main index: ID -> (type + pointer).
     * 
     * Provides O(1) access to any object by its unique ID.
     * Hash function for ID is defined in ID.h (std::hash specialization).
     */
    std::unordered_map<ID, FigureEntry> m_index;
    
    /**
     * @brief Unique ID generator.
     * 
     * Single generator for entire storage all objects get unique ID
     * from unified sequence.
     */
    IDGenerator m_idGen;
    
    /**
     * @brief Reverse index: Point2D* -> ID.
     */
    std::unordered_map<const Point2D*, ID> m_pointToID;

    /**
     * @brief Convert type T to corresponding FigureType.
     */
    template <SupportedFigure T>
    static constexpr FigureType typeToEnum() noexcept;
    
    /**
     * @brief Find point ID by pointer.
     */
    [[nodiscard]] std::optional<ID> findPointID(const Point2D* ptr) const noexcept;
};

template <SupportedFigure T>
constexpr FigureType GeometryStorage::typeToEnum() noexcept {
    if constexpr (std::same_as<T, Point2D>)  return FigureType::ET_POINT2D;
    if constexpr (std::same_as<T, Line2D>)   return FigureType::ET_LINE;
    if constexpr (std::same_as<T, Circle2D>) return FigureType::ET_CIRCLE;
    if constexpr (std::same_as<T, Arc2D>)    return FigureType::ET_ARC;
}

template <SupportedFigure T>
T* GeometryStorage::get(ID id) const {
    auto it = m_index.find(id);
    if (it == m_index.end()) {
        throw std::runtime_error("Object with given ID not found");
    }
    
    const FigureEntry& entry = it->second;
    if (entry.type != typeToEnum<T>()) {
        throw std::runtime_error("Requested type does not match actual type");
    }
    
    return static_cast<T*>(entry.ptr);
}


} // namespace OurPaintDCM::Figures

#endif // OURPAINTDCM_HEADERS_FIGURES_GEOMETRYSTORAGE_H

