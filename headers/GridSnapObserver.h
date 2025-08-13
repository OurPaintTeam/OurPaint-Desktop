#ifndef OURPAINT_HEADERS_GRID_SNAP_OBSERVER_H_
#define OURPAINT_HEADERS_GRID_SNAP_OBSERVER_H_

// Port
class IGridSnapObserver {
public:
    virtual void gridStateChanged(bool state) = 0;

    ~IGridSnapObserver() = default;
};

#ifdef QT_VERSION
#include <QtPlugin>
Q_DECLARE_INTERFACE(IGridSnapObserver,"org.ourpaint.IGridSnapObserver/1.0")
#endif

#endif // ! OURPAINT_HEADERS_GRID_SNAP_OBSERVER_H_