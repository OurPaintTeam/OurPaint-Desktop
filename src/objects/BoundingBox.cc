#include "objects/BoundingBox.h"

BoundingBox BoundingBox::unite(const BoundingBox& other) const {
    return {
            std::min(min_x, other.min_x),
            std::max(max_x, other.max_x),
            std::min(min_y, other.min_y),
            std::max(max_y, other.max_y)
    };
}

BoundingBox BoundingBox::intersect(const BoundingBox& other) const {
    return {
            std::max(min_x, other.min_x),
            std::min(max_x, other.max_x),
            std::max(min_y, other.min_y),
            std::min(max_y, other.max_y)
    };
}

BoundingBox BoundingBox::operator|(const BoundingBox &other) const {
    return unite(other);
}

BoundingBox BoundingBox::operator&(const BoundingBox &other) const {
    return intersect(other);
}

bool BoundingBox::isValid() const {
    return min_x <= max_x && min_y <= max_y;
}