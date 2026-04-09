#ifndef OURPAINT_RENDERING_IVEIWPORTHOST_H_
#define OURPAINT_RENDERING_IVEIWPORTHOST_H_

class ViewportSize;
class IViewportController;
class RenderTargetDesc;

#include <cstdint>

class IViewportHost {
public:
    virtual ~IViewportHost() = default;

    virtual uint32_t id() const = 0;

    virtual ViewportSize getSize() const = 0;
    virtual float getDevicePixelRatio() const = 0;
    virtual RenderTargetDesc renderTargetDesc() const = 0;

    // Запросить один кадр.
    virtual void requestRedraw() = 0;

    // Включить непрерывную отрисовку.
    virtual void setContinuousRedraw(bool enabled) = 0;

    // Приемник событий. Хост не владеет им.
    virtual void setEventSink(IViewportController* sink) = 0;

    // Вызывается в конце.
    virtual void presentFrame() = 0;
};

#endif // ! OURPAINT_RENDERING_IVEIWPORTHOST_H_