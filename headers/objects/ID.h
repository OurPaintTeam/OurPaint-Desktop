#ifndef OURPAINT_HEADERS_ID_H_
#define OURPAINT_HEADERS_ID_H_

#include <fstream>

class ID {
private:
    long long int _value;

public:
    explicit ID(long long int value = 0) : _value(value) {}
    ID(const ID&) = default;
    ID(ID&&) = default;

    ID& operator=(const ID&) = default;
    ID& operator=(ID&&) noexcept = default;

    long long int get() const { return _value; }

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
            return hash<long long int>()(id.get());
        }
    };
}

class IDGenerator {
    long long int last_id = 0;

public:
    ID generate() {
        return ID(++last_id);
    }

    void reset(long long int _id = 0) {
        last_id = _id;
    }
};

#endif // ! OURPAINT_HEADERS_ID_H_