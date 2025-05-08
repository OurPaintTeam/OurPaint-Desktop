#ifndef OURPAINT_HEADERS_ID_H_
#define OURPAINT_HEADERS_ID_H_

#include <fstream>
#include <unordered_set>
#include <cstdint>

class ID {
private:
    int64_t _value;

public:
    explicit ID(int64_t value = 0) : _value(value) {}
    ID(const ID&) = default;
    ID(ID&&) = default;

    ID& operator=(const ID&) = default;
    ID& operator=(ID&&) noexcept = default;

    int64_t get() const { return _value; }

    // Explicitly deleted to prevent misuse — IDs must not be altered.
    ID& operator++() = delete;
    ID operator++(int) = delete;
    ID& operator--() = delete;
    ID operator--(int) = delete;

    friend bool operator==(const ID &left, const ID &right) { return left._value == right._value; }
    friend bool operator!=(const ID &left, const ID &right) { return !(left == right); }
    friend bool operator<(const ID &left, const ID &right) { return left._value < right._value; }
    friend bool operator>(const ID &left, const ID &right) { return right < left; }
    friend bool operator<=(const ID &left, const ID &right) { return !(left > right); }
    friend bool operator>=(const ID &left, const ID &right) { return !(left < right); }

    friend std::ostream &operator<<(std::ostream &os, const ID &x) {
        return os << x._value;
    }
};

namespace std {
    template <>
    struct hash<ID> {
        std::size_t operator()(const ID &id) const {
            return hash<int64_t>()(id.get());
        }
    };
}

class IDGenerator {
    int64_t last_id = 0;
    std::unordered_set<ID> excluded_ids;

public:
    ID generate() {
        do {
            ++last_id;
        } while (excluded_ids.contains(ID(last_id)));
        return ID(last_id);
    }

    void exclude(ID id) {
        excluded_ids.insert(id);
    }

    void include(ID id) {
        excluded_ids.erase(id);
    }

    void reset(int64_t _id = 0) {
        last_id = _id;
    }

    ID getLast() const {
        return ID(last_id);
    }
};

#endif // ! OURPAINT_HEADERS_ID_H_