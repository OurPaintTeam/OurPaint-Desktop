#include "SnapEngine.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "Scene.h"

SnapEngine::SnapEngine() {}

SnapEngine::SnapResult SnapEngine::getHint(const SnapRequest& req) {
    if (req.object != TypeObject::point) {
        return SnapResult{};
    }

    SnapCandidate best;
    best.valid = false;
    best.dist = std::numeric_limits<double>::max();

    for (auto& p : req.scene.getPoints()) {
        if (const auto candidate = snapPoint(req, p); candidate.valid && candidate.dist < best.dist) {
            best = candidate;
        }
    }

    for (auto& l : req.scene.getLines()) {
        if (const auto candidate = snapLine(req, l); candidate.valid && candidate.dist < best.dist) {
            best = candidate;
        }
    }

    for (auto& c : req.scene.getCircles()) {
        if (const auto candidate = snapCircle(req, c); candidate.valid && candidate.dist < best.dist) {
            best = candidate;
        }
    }

    if (best.valid && best.dist < SNAP_THRESHOLD) {
        return generateSnapForObject(req, best.obj);
    }

    return axisFallback(req);
}

SnapEngine::SnapCandidate SnapEngine::snapPoint(const SnapRequest& req, const ObjectData& p) {
    SnapCandidate c;
    c.obj = p;
    c.dist = dist(req.cursor.first, req.cursor.second, p.params[0], p.params[1]);
    c.valid = true;
    return c;
}

SnapEngine::SnapCandidate SnapEngine::snapLine(const SnapRequest& req, const ObjectData& l) {
    const double px = req.cursor.first;
    const double py = req.cursor.second;

    const double x1 = l.params[0];
    const double y1 = l.params[1];
    const double x2 = l.params[2];
    const double y2 = l.params[3];

    double nx, ny;
    double distToLine = closestPointOnSegment(px, py, x1, y1, x2, y2, nx, ny);
    double distToStart = dist(px, py, x1, y1);
    double distToCenter = dist(px, py, (x1 + x2) / 2, (y1 + y2) / 2);
    double distToEnd = dist(px, py, x2, y2);

    const double minDist = std::min({distToLine, distToStart, distToCenter, distToEnd});

    SnapCandidate c;
    c.obj = l;
    c.dist = minDist;
    c.valid = true;
    c.snapPoint = {nx, ny};

    return c;
}

SnapEngine::SnapCandidate SnapEngine::snapCircle(const SnapRequest& req, const ObjectData& cobj) {
    const double px = req.cursor.first;
    const double py = req.cursor.second;

    const double cx = cobj.params[0];
    const double cy = cobj.params[1];
    const double r = cobj.params[2];

    const double dx = px - cx;
    const double dy = py - cy;
    const double len = std::sqrt(dx * dx + dy * dy);

    if (len == 0) {
        return SnapCandidate();
    }

    const double distToCircle = std::abs(len - r);
    double minDist = distToCircle;

    for (int i = 0; i < 8; ++i) {
        const double angle = i * M_PI / 4.0;
        const double kx = cx + r * std::cos(angle);
        const double ky = cy + r * std::sin(angle);
        if (const double d = dist(px, py, kx, ky); d < minDist) {
            minDist = d;
        }
    }

    const double angle = std::atan2(dy, dx);
    double nx = cx + r * std::cos(angle);
    double ny = cy + r * std::sin(angle);

    SnapCandidate c;
    c.obj = cobj;
    c.dist = minDist;
    c.valid = true;
    c.snapPoint = {nx, ny};

    return c;
}

SnapEngine::SnapResult SnapEngine::generateSnapForObject(const SnapRequest& req, const ObjectData& obj) {
    SnapResult result;
    result.ids.push_back(OurPaintDCM::Utils::ID(obj.id.get()));
    result.action = TypeAction::pointOnPoint;

    double px = req.cursor.first;
    double py = req.cursor.second;

    if (obj.et == ObjType::ET_POINT) {
        result.candidate = {roundCoord(obj.params[0]), roundCoord(obj.params[1])};
    } else if (obj.et == ObjType::ET_LINE) {
        double x1 = roundCoord(obj.params[0]);
        double y1 = roundCoord(obj.params[1]);
        double x2 = roundCoord(obj.params[2]);
        double y2 = roundCoord(obj.params[3]);

        double centerX = roundCoord((x1 + x2) / 2.0);
        double centerY = roundCoord((y1 + y2) / 2.0);

        const std::vector<std::pair<double, double>> keyPoints = {
            {x1, y1},
            {centerX, centerY},
            {x2, y2}
        };

        double bestDist = std::numeric_limits<double>::max();
        std::pair<double, double> bestPoint = keyPoints[0];

        for (const auto& point : keyPoints) {
            double d = dist(px, py, point.first, point.second);
            if (d < bestDist) {
                bestDist = d;
                bestPoint = point;
            }
        }

        double nx, ny;
        closestPointOnSegment(px, py, x1, y1, x2, y2, nx, ny);
        nx = roundCoord(nx);
        ny = roundCoord(ny);

        if (const double distToLine = dist(px, py, nx, ny); distToLine < SNAP_THRESHOLD && distToLine < bestDist) {
            result.candidate = {nx, ny};
        } else {
            result.candidate = bestPoint;
        }
    } else if (obj.et == ObjType::ET_CIRCLE) {
        const double cx = roundCoord(obj.params[0]);
        const double cy = roundCoord(obj.params[1]);
        const double r = obj.params[2];

        std::vector<std::pair<double, double>> circlePoints;

        for (double angles[] = {0, 45, 90, 135, 180, 225, 270, 315}; double deg : angles) {
            const double rad = deg * M_PI / 180.0;
            const double nx = cx + r * std::cos(rad);
            const double ny = cy + r * std::sin(rad);
            circlePoints.push_back({roundCoord(nx), roundCoord(ny)});
        }

        double bestDist = std::numeric_limits<double>::max();
        auto bestPoint = circlePoints[0];

        for (const auto& point : circlePoints) {
            if (const double d = dist(px, py, point.first, point.second); d < bestDist) {
                bestDist = d;
                bestPoint = point;
            }
        }

        const double dx = px - cx;
        const double dy = py - cy;

        if (const double len = std::sqrt(dx * dx + dy * dy); len > 0) {
            if (const double distToCircle = std::abs(len - r); distToCircle < SNAP_THRESHOLD) {
                const double angleRad = std::atan2(dy, dx);
                double angleDeg = angleRad * 180.0 / M_PI;
                if (angleDeg < 0) angleDeg += 360.0;

                const int nearestMultiple = static_cast<int>(std::round(angleDeg / 45.0)) % 8;
                const double snapDeg = nearestMultiple * 45.0;
                const double snapRad = snapDeg * M_PI / 180.0;

                const double nx = cx + r * std::cos(snapRad);
                const double ny = cy + r * std::sin(snapRad);
                result.candidate = {roundCoord(nx), roundCoord(ny)};
                return result;
            }
        }

        result.candidate = bestPoint;
    }

    return result;
}

SnapEngine::SnapResult SnapEngine::axisFallback(const SnapRequest& req) {
    SnapResult r;
    r.action = TypeAction::pointOnAxis;

    double px = req.cursor.first;
    double py = req.cursor.second;

    if (std::abs(py) < SNAP_THRESHOLD) {
        r.candidate = {px, 0.0};
    } else if (std::abs(px) < SNAP_THRESHOLD) {
        r.candidate = {0.0, py};
    } else if (std::abs(px) < std::abs(py)) {
        r.candidate = {0.0, py};
    } else {
        r.candidate = {px, 0.0};
    }

    return r;
}

double SnapEngine::closestPointOnSegment(double px, double py, double x1, double y1, double x2, double y2, double& nx, double& ny) {
    const double ax = px - x1;
    const double ay = py - y1;
    const double bx = x2 - x1;
    const double by = y2 - y1;

    const double dot = ax * bx + ay * by;
    const double len2 = bx * bx + by * by;

    if (len2 == 0) {
        nx = x1;
        ny = y1;
        return dist(px, py, x1, y1);
    }

    double t = dot / len2;
    t = std::clamp(t, 0.0, 1.0);

    nx = x1 + t * bx;
    ny = y1 + t * by;

    return dist(px, py, nx, ny);
}

double SnapEngine::dist(const double x1, const double y1, const double x2, const double y2) {
    return std::hypot(x2 - x1, y2 - y1);
}

double SnapEngine::roundCoord(const double value) {
    const double nearestInt = std::round(value);
    if (constexpr double EPS = 1e-8; std::abs(value - nearestInt) < EPS) {
        return nearestInt;
    }

    return std::round(value * 1e6) / 1e6;
}