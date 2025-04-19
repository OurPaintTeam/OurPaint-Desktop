#ifndef OURPAINT_HEADERS_ID_H_
#define OURPAINT_HEADERS_ID_H_

#include <fstream>

class ID {
    const long long int value;

public:
    explicit ID(long long int i = 0) : value(i) {}
    ID(const ID&) = default;
    ID(ID&&) = default;

    long long int get() const { return value; }

    ID& operator=(const ID&) = delete;
    ID operator=(ID&&) = delete;

    ID& operator++() = delete;
    ID operator++(int) = delete;
    ID& operator--() = delete;
    ID operator--(int) = delete;

    friend bool operator==(const ID &left, const ID &right) { return left.value == right.value; }
    friend bool operator!=(const ID &left, const ID &right) { return !(left == right); }
    friend bool operator<(const ID &left, const ID &right) { return left.value < right.value; }
    friend bool operator>(const ID &left, const ID &right) { return right < left; }
    friend bool operator<=(const ID &left, const ID &right) { return !(left > right); }
    friend bool operator>=(const ID &left, const ID &right) { return !(left < right); }

    friend std::ostream &operator<<(std::ostream &os, const ID &x) {
        return os << x.value;
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
};

#endif // ! OURPAINT_HEADERS_ID_H_