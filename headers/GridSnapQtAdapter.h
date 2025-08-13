#ifndef OURPAINT_HEADERS_GRID_SNAP_QT_ADAPTER_H_
#define OURPAINT_HEADERS_GRID_SNAP_QT_ADAPTER_H_

#include <QObject>
#include "GridSnapObserver.h"

class GridSnap;

class GridSnapQtAdapter final
        : public QObject,
          public IGridSnapObserver {
    Q_OBJECT
    Q_INTERFACES(IGridSnapObserver)
public:
    explicit GridSnapQtAdapter(GridSnap& gs);

    void gridStateChanged(bool state) override;

signals:
    void gridStateChangedQt(bool state);
};

#endif // ! OURPAINT_HEADERS_GRID_SNAP_QT_ADAPTER_H_
