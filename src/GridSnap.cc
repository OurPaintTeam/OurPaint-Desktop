#include "GridSnap.h"
#include "GeometricObjects.h"
#include "GridSnapObserver.h"

GridSnap::GridSnap(bool gridState, bool snapGridState, bool snapObjectsState) : _gridState(gridState),
                                                                                _snapGridState(snapGridState),
                                                                                _snapObjectsState(snapObjectsState) {}

Point GridSnap::snapPoint(const Point& p) const {
    // old Point -> new Point
    return Point();
}

void GridSnap::setGridState(bool gridState) {
    _gridState = gridState;
    _o->gridStateChanged(gridState);
}

void GridSnap::setSnapGridState(bool snapGridState) {
    _snapGridState = snapGridState;
}

void GridSnap::setSnapObjectsState(bool snapObjectsState) {
    _snapObjectsState = snapObjectsState;
}
void GridSnap::setSnapState(bool snapState) {
    setSnapGridState(true);
    setSnapObjectsState(true);
}
bool GridSnap::getGridState() const {
    return _gridState;
}

bool GridSnap::getSnapState() const {
    return _snapGridState && _snapObjectsState;
}

bool GridSnap::getSnapGridState() const {
    return _snapGridState;
}

bool GridSnap::getSnapObjectsState() const {
    return _snapObjectsState;
}
void GridSnap::setObserver(IGridSnapObserver* o) {
    _o = o;
}
