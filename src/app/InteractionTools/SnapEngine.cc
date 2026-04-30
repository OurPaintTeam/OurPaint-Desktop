#include "SnapEngine.h"

#include "Scene.h"

SnapEngine::SnapEngine() {}

SnapEngine::SnapResult SnapEngine::getHint(const SnapRequest& req) {
    SnapResult result{};

    struct LocalBest {
        SnapCandidate c;
        bool valid = false;
    } best;

    auto check = [&](const SnapCandidate& c) {
        if (c.valid && (!best.valid || c.dist < best.c.dist)) {
            best.c = c;
            best.valid = true;
        }
    };

    for (auto& p : req.scene.getPoints()) check(snapPoint(req, p));

    for (auto& l : req.scene.getLines()) check(snapLine(req, l));

    for (auto& c : req.scene.getCircles()) check(snapCircle(req, c));

    if (!best.valid) {
        return axisFallback(req);
    }

    return refineSnap(req, best.c.obj);
}

SnapEngine::SnapCandidate SnapEngine::snapPoint(const SnapRequest& req, const ObjectData& p) {
    SnapCandidate c;

    const double px = req.cursor.first;
    const double py = req.cursor.second;

    const double x = p.params[0];
    const double y = p.params[1];

    c.dist = dist(px, py, x, y);
    c.obj = p;
    c.valid = true;

    return c;
}

SnapEngine::SnapCandidate SnapEngine::snapLine(const SnapRequest& req, const ObjectData& l) {
    double px = req.cursor.first;
    double py = req.cursor.second;

    double x1 = l.params[0];
    double y1 = l.params[1];
    double x2 = l.params[2];
    double y2 = l.params[3];

    double A = px - x1;
    double B = py - y1;
    double C = x2 - x1;
    double D = y2 - y1;

    double lenSq = C * C + D * D;
    double t = (lenSq != 0) ? (A * C + B * D) / lenSq : 0;

    t = std::clamp(t, 0.0, 1.0);

    double nx = x1 + t * C;
    double ny = y1 + t * D;

    SnapCandidate c;
    c.dist = dist(px, py, nx, ny);
    c.obj = l;
    c.valid = true;

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

    SnapCandidate c;
    c.dist = std::abs(len - r);
    c.obj = cobj;
    c.valid = true;

    return c;
}

SnapEngine::SnapResult SnapEngine::refineSnap(const SnapRequest& req, const ObjectData& obj) {
    if (obj.et == ObjType::ET_POINT) {
        auto res = refinePoint(req, obj);
        if (req.object == point) {
            res.action = pointOnPoint;
        }
    }

    if (obj.et == ObjType::ET_LINE) {
        auto res = refineLine(req, obj);
        if (req.object == point) {
            res.action = pointOnLine;
        }
    }

    if (obj.et == ObjType::ET_CIRCLE) {
        auto res = refineCircle(req, obj);
        if (req.object == point) {
            res.action = pointOnCircle;
        }
    }

    auto res = axisFallback(req);
    if (req.object == point) {
        res.action = pointOnAxis;
    }

    return res;
}

SnapEngine::SnapResult SnapEngine::refinePoint(const SnapRequest&, const ObjectData& p) {
    SnapResult r{};
    const OurPaintDCM::Utils::ID id((p.id.get()));
    r.ids.push_back(id);
    r.candidate.first = p.params[0];
    r.candidate.second = p.params[1];
    return r;
}

SnapEngine::SnapResult SnapEngine::refineLine(const SnapRequest& req, const ObjectData& l) {
    SnapResult r{};
    const OurPaintDCM::Utils::ID id((l.id.get()));
    r.ids.push_back(id);

    double x1 = l.params[0];
    double y1 = l.params[1];
    double x2 = l.params[2];
    double y2 = l.params[3];

    double px = req.cursor.first;
    double py = req.cursor.second;

    return r;
}

SnapEngine::SnapResult SnapEngine::refineCircle(const SnapRequest& req, const ObjectData& c) {
    SnapResult r{};
    const OurPaintDCM::Utils::ID id((c.id.get()));
    r.ids.push_back(id);

    const double cx = c.params[0];
    double cy = c.params[1];
    const double radius = c.params[2];

    const double px = req.cursor.first;
    const double py = req.cursor.second;

    const double dx = px - cx;
    const double dy = py - cy;

    const double len = std::sqrt(dx * dx + dy * dy);
    if (len == 0.0) {
        r.candidate = {cx + radius, cy};
        return r;
    }

    double nx = cx + (dx / len) * radius;
    double ny = cy + (dy / len) * radius;

    r.candidate = {nx, ny};

    return r;
}

SnapEngine::SnapResult SnapEngine::axisFallback(const SnapRequest& req) {
    SnapResult r{};

    double px = req.cursor.first;

    if (double py = req.cursor.second; std::abs(px) < std::abs(py)) {
        r.candidate = {0.0, py};
    } else {
        r.candidate = {px, 0.0};
    }

    return r;
}

double SnapEngine::dist(const double x1, const double y1, const double x2, const double y2) { return std::hypot(x2 - x1, y2 - y1); }