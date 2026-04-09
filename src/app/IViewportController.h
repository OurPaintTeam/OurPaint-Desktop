#ifndef OURPAINT_APPLICATION_IVIEWPORTEVENTSINK_H_
#define OURPAINT_APPLICATION_IVIEWPORTEVENTSINK_H_

#include "InputEvents.h"

class IViewportController {
public:
    virtual ~IViewportController() = default;

    virtual bool onResize     (const input::ResizeEvent& e)      = 0;
    virtual bool onMouseMove  (const input::MouseMoveEvent& e)   = 0;
    virtual bool onMouseButton(const input::MouseButtonEvent& e) = 0;
    virtual bool onWheel      (const input::WheelEvent& e)       = 0;
    virtual bool onKey        (const input::KeyEvent& e)         = 0;

    virtual void render() = 0;
};

#endif // ! OURPAINT_APPLICATION_IVIEWPORTEVENTSINK_H_