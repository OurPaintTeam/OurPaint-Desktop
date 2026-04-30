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
} // namespace

class Scene;
struct ObjectData;

class SnapEngine {
public:
    SnapEngine();

    enum TypeObject { point, line, circle, arc, beze };

    enum TypeAction {
        pointOnPoint,
        pointOnLine,
        pointOnCircle,
        pointOnArc,
        pointOnBeze,
        pointOnAxis
    };

    struct SnapRequest {
        TypeObject object;
        Scene& scene;
        std::pair<double,double> cursor;
    };

    struct SnapResult {
        TypeAction action;
        std::vector<OurPaintDCM::Utils::ID> ids;
        std::pair<double,double> candidate;
    };

    static SnapResult getHint(const SnapRequest& req);

private:
    struct SnapCandidate {
        ObjectData obj;
        double dist = 1e9;
        bool valid = false;
    };

private:
    // stage 1
    static SnapCandidate snapPoint(const SnapRequest& req, const ObjectData& p);
    static SnapCandidate snapLine(const SnapRequest& req, const ObjectData& l);
    static SnapCandidate snapCircle(const SnapRequest& req, const ObjectData& c);

    // stage 2
    static SnapResult refineSnap(const SnapRequest& req, const ObjectData& obj);

    static SnapResult refinePoint(const SnapRequest& req, const ObjectData& p);
    static SnapResult refineLine(const SnapRequest& req, const ObjectData& l);
    static SnapResult refineCircle(const SnapRequest& req, const ObjectData& c);

    static SnapResult axisFallback(const SnapRequest& req);
    static double dist(double x1, double y1, double x2, double y2);
};

#endif