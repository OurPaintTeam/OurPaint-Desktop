#ifndef OURPAINT_HEADERS_BOUNDINGBOX_H_
#define OURPAINT_HEADERS_BOUNDINGBOX_H_

#include <algorithm>
#include <limits>

struct BoundingBox {
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    BoundingBox unite(const BoundingBox& other) const;
    BoundingBox operator|(const BoundingBox& other) const;

    BoundingBox intersect(const BoundingBox& other) const;
    BoundingBox operator&(const BoundingBox& other) const;

    bool isValid() const;
};

#endif // ! OURPAINT_HEADERS_BOUNDINGBOX_H_