#ifndef OURPAINT_SNAPENGINE_H
#define OURPAINT_SNAPENGINE_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include "objects/Objects.h"

namespace OurPaintDCM::Utils {
struct ID;
}

class Scene;
struct ObjectData;

class SnapEngine {
public:
    SnapEngine();

    enum TypeObject {
        point,
        line,
        circle,
        arc,
        beze
    };

    enum TypeAction {
        pointOnPoint,
        pointOnLine,
        pointOnCircle,
        pointOnArc,
        pointOnBeze,
        pointOnAxis,
    };

    struct SnapRequest {
        TypeObject object;
        Scene& scene;
        std::pair<double, double> cursor;
    };

    struct SnapResult {
        TypeAction action = pointOnAxis;
        std::vector<OurPaintDCM::Utils::ID> ids;
        std::pair<double, double> candidate;
    };

    static SnapResult getHint(const SnapRequest& req);

private:
    static constexpr double SNAP_THRESHOLD = 10.0;

    struct SnapCandidate {
        ObjectData obj;
        double dist = 1e9;
        bool valid = false;
        std::pair<double, double> snapPoint;
    };

    static SnapCandidate snapPoint(const SnapRequest& req, const ObjectData& p);
    static SnapCandidate snapLine(const SnapRequest& req, const ObjectData& l);
    static SnapCandidate snapCircle(const SnapRequest& req, const ObjectData& c);

    static SnapResult generateSnapForObject(const SnapRequest& req, const ObjectData& obj);
    static SnapResult axisFallback(const SnapRequest& req);

    static double closestPointOnSegment(double px, double py,
                                        double x1, double y1,
                                        double x2, double y2,
                                        double& nx, double& ny);
    static double dist(double x1, double y1, double x2, double y2);
    static double roundCoord(double value);
};

#endif