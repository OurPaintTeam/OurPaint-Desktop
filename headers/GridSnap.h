#ifndef OURPAINT_HEADERS_GRID_SNAP_H_
#define OURPAINT_HEADERS_GRID_SNAP_H_

struct Point;
class IGridSnapObserver;

class GridSnap {
public:
    GridSnap(bool gridState, bool snapGrid, bool snapObjects);

    Point snapPoint(const Point& p) const;

    void setGridState(bool gridState);
    void setSnapGridState(bool snapGridState);
    void setSnapObjectsState(bool snapObjectsState);
    void setSnapState(bool snapState);

    bool getGridState() const;
    bool getSnapState() const;
    bool getSnapGridState() const;
    bool getSnapObjectsState() const;

    void setObserver(IGridSnapObserver* o);

private:
    bool _gridState;
    bool _snapGridState;
    bool _snapObjectsState;

    IGridSnapObserver* _o = nullptr;
};

#endif // ! OURPAINT_HEADERS_GRID_SNAP_H_