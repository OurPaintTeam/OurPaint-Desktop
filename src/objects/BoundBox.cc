#include "objects/BoundBox.h"

BoundBox2D BoundBox2D::unite(const BoundBox2D &other) const {
    return {
            std::min(min_x, other.min_x),
            std::max(max_x, other.max_x),
            std::min(min_y, other.min_y),
            std::max(max_y, other.max_y)
    };
}

BoundBox2D BoundBox2D::intersect(const BoundBox2D &other) const {
    BoundBox2D result = {
            std::max(min_x, other.min_x),
            std::min(max_x, other.max_x),
            std::max(min_y, other.min_y),
            std::min(max_y, other.max_y)
    };
    if (!result.isValid()) {
        return {};
    }
    return result;
}

BoundBox2D BoundBox2D::operator|(const BoundBox2D &other) const {
    return unite(other);
}

BoundBox2D BoundBox2D::operator&(const BoundBox2D &other) const {
    return intersect(other);
}

constexpr bool BoundBox2D::isValid() const {
    return min_x <= max_x && min_y <= max_y;
}

double BoundBox2D::width() const {
    return isValid() ? (max_x - min_x) : 0.0;
}

double BoundBox2D::height() const {
    return isValid() ? (max_y - min_y) : 0.0;
}

