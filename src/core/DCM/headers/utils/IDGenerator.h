#ifndef HEADERS_UTILS_IDGENERATOR_H
#define HEADERS_UTILS_IDGENERATOR_H

#include "utils/ID.h"

namespace OurPaintDCM::Utils {

/**
 * @brief Monotonic unique ID generator.
 *
 * This class generates unique IDs for geometric entities,
 * requirements, or any other objects inside the solver.
 *
 * - IDs start from 1 by default.
 * - Each call to nextID() returns a new unique ID.
 * - The generator is non-copyable and non-movable.
 *
 * Example:
 * @code
 * Utils::IDGenerator gen;
 * Utils::ID a = gen.nextID(); // ID(1)
 * Utils::ID b = gen.nextID(); // ID(2)
 * @endcode
 */
class IDGenerator {
    ID _id; ///< Current state of the generator (last issued ID + 1)

public:
    /// @brief Construct a new IDGenerator starting from 1.
    IDGenerator() noexcept : _id(1) {}

    /// @brief Deleted copy constructor (non-copyable).
    IDGenerator(const IDGenerator&) = delete;

    /// @brief Deleted copy assignment (non-copyable).
    IDGenerator& operator=(const IDGenerator&) = delete;

    /// @brief Deleted move constructor (non-movable).
    IDGenerator(IDGenerator&&) = delete;

    /// @brief Deleted move assignment (non-movable).
    IDGenerator& operator=(IDGenerator&&) = delete;

    /// @brief Default destructor.
    ~IDGenerator() = default;

    /**
     * @brief Get current ID without increment.
     * @return The current ID (does not change internal state).
     */
    [[nodiscard]] constexpr const ID& current() const noexcept {
        return _id;
    }

    /**
     * @brief Generate and return the next unique ID.
     * @return A new unique ID.
     */
    [[nodiscard]] ID nextID() noexcept {
        return ID(_id++);
    }

    /**
     * @brief Set the generator state to a given ID.
     * @param id New starting ID.
     */
    void set(ID id) noexcept {
        _id = id;
    }

    /**
     * @brief Reset generator back to initial state (ID = 1).
     */
    void reset() noexcept {
        _id = ID(1);
    }
};

}

#endif // HEADERS_UTILS_IDGENERATOR_H
