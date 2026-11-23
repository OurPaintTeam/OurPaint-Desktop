#ifndef OURPAINT_APPLICATION_RECT_SELECTION_MODE_H_
#define OURPAINT_APPLICATION_RECT_SELECTION_MODE_H_

namespace app {

enum class RectSelectionMode {
    Window,   // Select curves fully contained in the rectangle.
    Crossing  // Also select curves that intersect its boundary.
};

}  // namespace app

#endif
