#include "GridSnapQtAdapter.h"
#include "GridSnap.h"

GridSnapQtAdapter::GridSnapQtAdapter(GridSnap &gs) : QObject(nullptr) {
    gs.setObserver(this);
}

void GridSnapQtAdapter::gridStateChanged(bool state) {
    emit gridStateChangedQt(state);
}