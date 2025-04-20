#ifndef OURPAINT_HEADERS_BOUNDINGBOX_H_
#define OURPAINT_HEADERS_BOUNDINGBOX_H_

#include <algorithm>
#include <limits>

class BoundBox2D {
public:
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    constexpr bool isValid() const;
    double width() const;
    double height() const;

    BoundBox2D unite(const BoundBox2D &other) const;
    BoundBox2D operator|(const BoundBox2D &other) const;

    BoundBox2D intersect(const BoundBox2D &other) const;
    BoundBox2D operator&(const BoundBox2D &other) const;
};

#endif // ! OURPAINT_HEADERS_BOUNDINGBOX_H_