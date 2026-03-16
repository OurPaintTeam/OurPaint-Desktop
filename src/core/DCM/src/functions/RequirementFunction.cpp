#include "functions/RequirementFunction.h"
#include <stdexcept>
#include <cmath>
//PointLineDistanceFunction Requirement
OurPaintDCM::Function::PointLineDistanceFunction::PointLineDistanceFunction(
    const std::vector<VAR> &vars, double dist) : RequirementFunction(
    Utils::RequirementType::ET_POINTLINEDIST, vars) {
    if (vars.size() != 6) {
        throw std::invalid_argument("This function must have 6 variables");
    }
    _distance = dist;
}

double OurPaintDCM::Function::PointLineDistanceFunction::evaluate() const {
    double dx = *_vars[4] - *_vars[2];
    double dy = *_vars[5] - *_vars[3];
    double lineLen = std::sqrt(dx * dx + dy * dy);
    if (lineLen < 1e-12) {
        return 0.0;
    }

    double px = *_vars[0] - *_vars[2];
    double py = *_vars[1] - *_vars[3];

    double cross = px * dy - py * dx;
    double dist = cross / lineLen;

    return dist - _distance;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::PointLineDistanceFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double dx = *_vars[4] - *_vars[2];
    double dy = *_vars[5] - *_vars[3];
    double lineLen = std::sqrt(dx * dx + dy * dy);

    if (lineLen < 1e-12) {
        for (auto v : _vars) grad[v] = 0.0;
        return grad;
    }

    double px = *_vars[0] - *_vars[2];
    double py = *_vars[1] - *_vars[3];
    double cross = px * dy - py * dx;
    double lineLen2 = lineLen * lineLen;

    // df/dPx, df/dPy
    grad[_vars[0]] = dy / lineLen;
    grad[_vars[1]] = -dx / lineLen;

    // df/dX1, df/dY1
    grad[_vars[2]] = (-dy - cross * dx / lineLen2) / lineLen;
    grad[_vars[3]] = (dx - cross * dy / lineLen2) / lineLen;

    // df/dX2, df/dY2
    grad[_vars[4]] = (cross * dx / lineLen2) / lineLen;
    grad[_vars[5]] = (cross * dy / lineLen2) / lineLen;

    return grad;
}

size_t OurPaintDCM::Function::PointLineDistanceFunction::getVarCount() const {
    return 6;
}

//PointOnLineFunction Requirement

OurPaintDCM::Function::PointOnLineFunction::PointOnLineFunction(const std::vector<VAR> &vars) : RequirementFunction(
    Utils::RequirementType::ET_POINTONLINE, vars) {
    if (vars.size() != 6) {
        throw std::invalid_argument("This function must have 6 variables");
    }
}

double OurPaintDCM::Function::PointOnLineFunction::evaluate() const {
    double dx = *_vars[4] - *_vars[2];
    double dy = *_vars[5] - *_vars[3];
    double lineLen = std::sqrt(dx * dx + dy * dy);
    if (lineLen < 1e-12) {
        return 0.0;
    }
    double num = (*_vars[0] - *_vars[2]) * dy - (*_vars[1] - *_vars[3]);
    return num / lineLen;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::PointOnLineFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double dx = *_vars[4] - *_vars[2];
    double dy = *_vars[5] - *_vars[3];
    double lineLen = std::sqrt(dx * dx + dy * dy);

    if (lineLen < 1e-10) {
        grad[_vars[0]] = 0.0;
        grad[_vars[1]] = 0.0;
        grad[_vars[2]] = 0.0;
        grad[_vars[3]] = 0.0;
        grad[_vars[4]] = 0.0;
        grad[_vars[5]] = 0.0;
        return grad;
    }

    double px = *_vars[0] - *_vars[2];
    double py = *_vars[1] - *_vars[3];
    double cross = px * dy - py * dx;
    double sign = (cross >= 0.0) ? 1.0 : -1.0;

    double lineLen2 = lineLen * lineLen;

    // df/dPx, df/dPy
    grad[_vars[0]] = sign * dy / lineLen;
    grad[_vars[1]] = -sign * dx / lineLen;

    // df/dX1, df/dY1
    grad[_vars[2]] = sign * (-dy + (cross * dx) / lineLen2) / lineLen;
    grad[_vars[3]] = sign * (dx + (cross * dy) / lineLen2) / lineLen;

    // df/dX2, df/dY2
    grad[_vars[4]] = -sign * ((-cross * dx) / lineLen2) / lineLen;
    grad[_vars[5]] = -sign * ((-cross * dy) / lineLen2) / lineLen;

    return grad;
}

size_t OurPaintDCM::Function::PointOnLineFunction::getVarCount() const {
    return 6;
}

// PointPointDistanceFunction Requirement
OurPaintDCM::Function::PointPointDistanceFunction::PointPointDistanceFunction(
    const std::vector<VAR> &vars, double dist) : RequirementFunction(
    Utils::RequirementType::ET_POINTPOINTDIST, vars) {
    if (vars.size() != 4) {
        throw std::invalid_argument("This function must have 4 variables");
    }
    _distance = dist;
}

double OurPaintDCM::Function::PointPointDistanceFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];

    double dx = x2 - x1;
    double dy = y2 - y1;
    double dist = std::sqrt(dx * dx + dy * dy);

    return dist - _distance;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::PointPointDistanceFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];

    double dx = x2 - x1;
    double dy = y2 - y1;
    double dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 1e-10) {
        grad[_vars[0]] = 0.0;
        grad[_vars[1]] = 0.0;
        grad[_vars[2]] = 0.0;
        grad[_vars[3]] = 0.0;
        return grad;
    }

    // df/dx1 = -(x2 - x1)/dist
    grad[_vars[0]] = -dx / dist;
    // df/dy1 = -(y2 - y1)/dist
    grad[_vars[1]] = -dy / dist;
    // df/dx2 =  (x2 - x1)/dist
    grad[_vars[2]] = dx / dist;
    // df/dy2 =  (y2 - y1)/dist
    grad[_vars[3]] = dy / dist;

    return grad;
}
size_t OurPaintDCM::Function::PointPointDistanceFunction::getVarCount() const {
    return 4;
}
//PointOnPointFunction Requirement
OurPaintDCM::Function::PointOnPointFunction::PointOnPointFunction(const std::vector<VAR> &vars) : RequirementFunction(
    Utils::RequirementType::ET_POINTONPOINT, vars) {
    if (vars.size() != 4) {
        throw std::invalid_argument("This function must have 4 variables");
    }
}

double OurPaintDCM::Function::PointOnPointFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];

    double dx = x2 - x1;
    double dy = y2 - y1;
    double dist = std::sqrt(dx * dx + dy * dy);

    return dist;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::PointOnPointFunction::gradient() const {
    std::unordered_map<VAR, double> grad;
    double dx = *_vars[2] - *_vars[0];
    double dy = *_vars[3] - *_vars[1];
    double dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 1e-10) {
        grad[_vars[0]] = 0.0;
        grad[_vars[1]] = 0.0;
        grad[_vars[2]] = 0.0;
        grad[_vars[3]] = 0.0;
        return grad;
    }

    // df/dx1 = -(x2 - x1)/dist
    grad[_vars[0]] = -dx / dist;
    // df/dy1 = -(y2 - y1)/dist
    grad[_vars[1]] = -dy / dist;
    // df/dx2 =  (x2 - x1)/dist
    grad[_vars[2]] = dx / dist;
    // df/dy2 =  (y2 - y1)/dist
    grad[_vars[3]] = dy / dist;

    return grad;
}

size_t OurPaintDCM::Function::PointOnPointFunction::getVarCount() const {
    return 4;
}

// LineCircleDistanceFunction Requirement

OurPaintDCM::Function::LineCircleDistanceFunction::LineCircleDistanceFunction(
    const std::vector<VAR> &vars, double dist) : RequirementFunction(
    Utils::RequirementType::ET_LINECIRCLEDIST, vars) {
    if (vars.size() != 7) {
        throw std::invalid_argument("This function must have 7 variables");
    }
    _distance = dist;
}

double OurPaintDCM::Function::LineCircleDistanceFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double cx = *_vars[4];
    double cy = *_vars[5];
    double r = *_vars[6];
    double dx = x2 - x1;
    double dy = y2 - y1;
    double line_len2 = dx * dx + dy * dy;

    if (line_len2 < 1e-10) {
        double dist = std::sqrt((cx - x1) * (cx - x1) + (cy - y1) * (cy - y1));
        return dist - r;
    }

    double t = ((cx - x1) * dx + (cy - y1) * dy) / line_len2;

    t = std::max(0.0, std::min(1.0, t));

    double px = x1 + t * dx;
    double py = y1 + t * dy;

    double dist = std::sqrt((cx - px) * (cx - px) + (cy - py) * (cy - py));

    return dist - r;
}


std::unordered_map<VAR, double> OurPaintDCM::Function::LineCircleDistanceFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double cx = *_vars[4];
    double cy = *_vars[5];
    double r = *_vars[6];

    double dx = x2 - x1;
    double dy = y2 - y1;
    double line_len2 = dx * dx + dy * dy;

    double px, py;
    double t;

    if (line_len2 < 1e-10) {
        px = x1;
        py = y1;
        t = 0.0;
    } else {
        t = ((cx - x1) * dx + (cy - y1) * dy) / line_len2;
        t = std::max(0.0, std::min(1.0, t));
        px = x1 + t * dx;
        py = y1 + t * dy;
    }

    double diff_x = px - cx;
    double diff_y = py - cy;
    double dist = std::sqrt(diff_x * diff_x + diff_y * diff_y);

    if (dist < 1e-10) {
        grad[_vars[0]] = 0.0;
        grad[_vars[1]] = 0.0;
        grad[_vars[2]] = 0.0;
        grad[_vars[3]] = 0.0;
        grad[_vars[4]] = 0.0;
        grad[_vars[5]] = 0.0;
        grad[_vars[6]] = 0.0;
        return grad;
    }

    double dfdpx = diff_x / dist;
    double dfdpy = diff_y / dist;

    double dt_dx1 = (t == 0.0) ? 0.0 : ((cx - x1) * (-1) - dx * t) / line_len2;
    double dt_dy1 = (t == 0.0) ? 0.0 : ((cy - y1) * (-1) - dy * t) / line_len2;
    double dt_dx2 = (t == 1.0) ? 0.0 : (dx * (1 - t)) / line_len2;
    double dt_dy2 = (t == 1.0) ? 0.0 : (dy * (1 - t)) / line_len2;

    double dpx_dx1 = (t < 1.0 && t > 0.0) ? 1.0 + dt_dx1 * dx - t : 1.0;
    double dpx_dy1 = dt_dy1 * dx;
    double dpx_dx2 = dt_dx2 * dx + t;
    double dpx_dy2 = dt_dy2 * dx;

    double dpy_dx1 = dt_dx1 * dy;
    double dpy_dy1 = 1.0 + dt_dy1 * dy - t;
    double dpy_dx2 = dt_dx2 * dy;
    double dpy_dy2 = dt_dy2 * dy + t;

    grad[_vars[0]] = dfdpx * dpx_dx1 + dfdpy * dpy_dx1; // L1x
    grad[_vars[1]] = dfdpx * dpx_dy1 + dfdpy * dpy_dy1; // L1y
    grad[_vars[2]] = dfdpx * dpx_dx2 + dfdpy * dpy_dx2; // L2x
    grad[_vars[3]] = dfdpx * dpx_dy2 + dfdpy * dpy_dy2; // L2y

    grad[_vars[4]] = -dfdpx; // Cx
    grad[_vars[5]] = -dfdpy; // Cy

    grad[_vars[6]] = -1.0; // df/dR = -1

    return grad;
}

size_t OurPaintDCM::Function::LineCircleDistanceFunction::getVarCount() const {
    return 7;
}

//LineOnCircleFunction Requirement
OurPaintDCM::Function::LineOnCircleFunction::LineOnCircleFunction(
    const std::vector<VAR> &vars) : RequirementFunction(
    Utils::RequirementType::ET_LINEONCIRCLE, vars) {
    if (vars.size() != 7) {
        throw std::invalid_argument("This function must have 7 variables");
    }
}

double OurPaintDCM::Function::LineOnCircleFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double cx = *_vars[4];
    double cy = *_vars[5];
    double r = *_vars[6];

    double dx1 = x1 - cx;
    double dy1 = y1 - cy;
    double dx2 = x2 - cx;
    double dy2 = y2 - cy;

    double dist1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    double dist2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

    return (dist1 - r) + (dist2 - r);
}

std::unordered_map<VAR, double> OurPaintDCM::Function::LineOnCircleFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double cx = *_vars[4];
    double cy = *_vars[5];
    double r = *_vars[6];

    double dx1 = x1 - cx;
    double dy1 = y1 - cy;
    double dist1 = std::sqrt(dx1 * dx1 + dy1 * dy1);

    double dx2 = x2 - cx;
    double dy2 = y2 - cy;
    double dist2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

    if (dist1 < 1e-10) dist1 = 1e-10;
    if (dist2 < 1e-10) dist2 = 1e-10;

    grad[_vars[0]] = dx1 / dist1; // d/dL1x
    grad[_vars[1]] = dy1 / dist1; // d/dL1y

    grad[_vars[2]] = dx2 / dist2; // d/dL2x
    grad[_vars[3]] = dy2 / dist2; // d/dL2y

    grad[_vars[4]] = -(dx1 / dist1 + dx2 / dist2); // d/dCx
    grad[_vars[5]] = -(dy1 / dist1 + dy2 / dist2); // d/dCy

    grad[_vars[6]] = -2.0;

    return grad;
}

size_t OurPaintDCM::Function::LineOnCircleFunction::getVarCount() const {
    return 7;
}

//LineLineParallelFunction Requirement
OurPaintDCM::Function::LineLineParallelFunction::LineLineParallelFunction(
    const std::vector<VAR> &vars) : RequirementFunction(
    Utils::RequirementType::ET_LINELINEPARALLEL, vars) {
    if (vars.size() != 8) {
        throw std::invalid_argument("This function must have 8 variables");
    }
}

double OurPaintDCM::Function::LineLineParallelFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double x3 = *_vars[4];
    double y3 = *_vars[5];
    double x4 = *_vars[6];
    double y4 = *_vars[7];

    double dx1 = x2 - x1;
    double dy1 = y2 - y1;
    double dx2 = x4 - x3;
    double dy2 = y4 - y3;

    double cross = dx1 * dy2 - dy1 * dx2;

    return cross;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::LineLineParallelFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double x3 = *_vars[4];
    double y3 = *_vars[5];
    double x4 = *_vars[6];
    double y4 = *_vars[7];

    double dx1 = x2 - x1;
    double dy1 = y2 - y1;
    double dx2 = x4 - x3;
    double dy2 = y4 - y3;

    grad[_vars[0]] = -dy2;
    grad[_vars[1]] = dx2;
    grad[_vars[2]] = dy2;
    grad[_vars[3]] = -dx2;

    grad[_vars[4]] = dy1;
    grad[_vars[5]] = -dx1;
    grad[_vars[6]] = -dy1;
    grad[_vars[7]] = dx1;

    return grad;
}

size_t OurPaintDCM::Function::LineLineParallelFunction::getVarCount() const {
    return 8;
}

// LineLinePerpendicularFunction Requirement
OurPaintDCM::Function::LineLinePerpendicularFunction::LineLinePerpendicularFunction(
    const std::vector<VAR> &vars) : RequirementFunction(
    Utils::RequirementType::ET_LINELINEPERPENDICULAR, vars) {
    if (vars.size() != 8) {
        throw std::invalid_argument("This function must have 8 variables");
    }
}

double OurPaintDCM::Function::LineLinePerpendicularFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double x3 = *_vars[4];
    double y3 = *_vars[5];
    double x4 = *_vars[6];
    double y4 = *_vars[7];

    double dx1 = x2 - x1;
    double dy1 = y2 - y1;
    double dx2 = x4 - x3;
    double dy2 = y4 - y3;

    return dx1 * dx2 + dy1 * dy2;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::LineLinePerpendicularFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double x3 = *_vars[4];
    double y3 = *_vars[5];
    double x4 = *_vars[6];
    double y4 = *_vars[7];

    double dx1 = x2 - x1;
    double dy1 = y2 - y1;
    double dx2 = x4 - x3;
    double dy2 = y4 - y3;

    grad[_vars[0]] = -dx2; // A1x
    grad[_vars[1]] = -dy2; // A1y
    grad[_vars[2]] = dx2; // A2x
    grad[_vars[3]] = dy2; // A2y

    grad[_vars[4]] = -dx1; // B1x
    grad[_vars[5]] = -dy1; // B1y
    grad[_vars[6]] = dx1; // B2x
    grad[_vars[7]] = dy1; // B2y

    return grad;
}

size_t OurPaintDCM::Function::LineLinePerpendicularFunction::getVarCount() const {
    return 8;
}

// LineLineAngleFunction Requirement
OurPaintDCM::Function::LineLineAngleFunction::LineLineAngleFunction(const std::vector<VAR> &vars,
                                                                    double angle) : RequirementFunction(
    Utils::RequirementType::ET_LINELINEANGLE, vars), _angle(angle) {
    if (vars.size() != 8) {
        throw std::invalid_argument("This function must have 8 variables");
    }
}

double OurPaintDCM::Function::LineLineAngleFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double x3 = *_vars[4];
    double y3 = *_vars[5];
    double x4 = *_vars[6];
    double y4 = *_vars[7];

    double dx1 = x2 - x1;
    double dy1 = y2 - y1;
    double dx2 = x4 - x3;
    double dy2 = y4 - y3;

    double dot = dx1 * dx2 + dy1 * dy2;
    double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

    if (len1 < 1e-10 || len2 < 1e-10) return 0.0;

    double cos_theta = dot / (len1 * len2);
    return cos_theta - std::cos(_angle);
}

std::unordered_map<VAR, double> OurPaintDCM::Function::LineLineAngleFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];
    double x3 = *_vars[4];
    double y3 = *_vars[5];
    double x4 = *_vars[6];
    double y4 = *_vars[7];

    double dx1 = x2 - x1;
    double dy1 = y2 - y1;
    double dx2 = x4 - x3;
    double dy2 = y4 - y3;

    double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

    if (len1 < 1e-10 || len2 < 1e-10) {
        for (VAR v: _vars) grad[v] = 0.0;
        return grad;
    }

    double dot = dx1 * dx2 + dy1 * dy2;
    double len1_3 = len1 * len1 * len1;
    double len2_3 = len2 * len2 * len2;

    // Gradient w.r.t v1 = (dx1, dy1)
    grad[_vars[0]] = dx2 / (len1 * len2) - dx1 * dot / (len1_3 * len2); // A1x
    grad[_vars[1]] = dy2 / (len1 * len2) - dy1 * dot / (len1_3 * len2); // A1y
    grad[_vars[2]] = -grad[_vars[0]]; // A2x
    grad[_vars[3]] = -grad[_vars[1]]; // A2y

    // Gradient w.r.t v2 = (dx2, dy2)
    grad[_vars[4]] = -(dx1 / (len1 * len2) - dx2 * dot / (len1 * len2_3)); // B1x
    grad[_vars[5]] = -(dy1 / (len1 * len2) - dy2 * dot / (len1 * len2_3)); // B1y
    grad[_vars[6]] = -grad[_vars[4]]; // B2x
    grad[_vars[7]] = -grad[_vars[5]]; // B2y

    return grad;
}
size_t OurPaintDCM::Function::LineLineAngleFunction::getVarCount() const {
    return 8;
}
// VerticalFunction Requirement
OurPaintDCM::Function::VerticalFunction::VerticalFunction(const std::vector<VAR> &vars) : RequirementFunction(
    Utils::RequirementType::ET_VERTICAL, vars) {
    if (vars.size() != 4) {
        throw std::invalid_argument("This function must have 4 variables");
    }
}

double OurPaintDCM::Function::VerticalFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];

    double dx = x2 - x1;
    double dy = y2 - y1;
    double len = std::sqrt(dx * dx + dy * dy); // normalize

    if (len < 1e-10)
        return 0.0;

    return dx / len;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::VerticalFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];

    double dx = x2 - x1;
    double dy = y2 - y1;
    double len2 = dx * dx + dy * dy;
    double len = std::sqrt(len2);

    if (len < 1e-10) {
        for (auto v: _vars) grad[v] = 0.0;
        return grad;
    }

    double len3 = len2 * len;

    grad[_vars[0]] = -1.0 / len + dx * dx / len3; // df/dx1
    grad[_vars[1]] = dx * dy / len3; // df/dy1
    grad[_vars[2]] = 1.0 / len - dx * dx / len3; // df/dx2
    grad[_vars[3]] = -dx * dy / len3; // df/dy2

    return grad;
}

size_t OurPaintDCM::Function::VerticalFunction::getVarCount() const {
    return 4;
}

// HorizontalFunction Requirement
OurPaintDCM::Function::HorizontalFunction::HorizontalFunction(const std::vector<VAR> &vars) : RequirementFunction(
    Utils::RequirementType::ET_HORIZONTAL, vars) {
    if (vars.size() != 4) {
        throw std::invalid_argument("This function must have 4 variables");
    }
}

double OurPaintDCM::Function::HorizontalFunction::evaluate() const {
    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];

    double dx = x2 - x1;
    double dy = y2 - y1;
    double len = std::sqrt(dx * dx + dy * dy); // normalize

    if (len < 1e-10)
        return 0.0;

    return dy / len;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::HorizontalFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double x1 = *_vars[0];
    double y1 = *_vars[1];
    double x2 = *_vars[2];
    double y2 = *_vars[3];

    double dx = x2 - x1;
    double dy = y2 - y1;
    double len2 = dx * dx + dy * dy;
    double len = std::sqrt(len2);

    if (len < 1e-10) {
        for (auto v: _vars) grad[v] = 0.0;
        return grad;
    }

    double len3 = len2 * len;

    // f = dy / len
    grad[_vars[0]] = dx * dy / len3; // df/dx1
    grad[_vars[1]] = -1.0 / len + dy * dy / len3; // df/dy1
    grad[_vars[2]] = -dx * dy / len3; // df/dx2
    grad[_vars[3]] = 1.0 / len - dy * dy / len3; // df/dy2

    return grad;
}


size_t OurPaintDCM::Function::HorizontalFunction::getVarCount() const {
    return 4;
}

// ArcCenterOnPerpendicularFunction Requirement
OurPaintDCM::Function::ArcCenterOnPerpendicularFunction::ArcCenterOnPerpendicularFunction(
    const std::vector<VAR> &vars) : RequirementFunction(
    Utils::RequirementType::ET_ARCCENTERONPERPENDICULAR, vars) {
    if (vars.size() != 6) {
        throw std::invalid_argument("This function must have 6 variables");
    }
}

double OurPaintDCM::Function::ArcCenterOnPerpendicularFunction::evaluate() const {
    double Ax = *_vars[0];
    double Ay = *_vars[1];
    double Bx = *_vars[2];
    double By = *_vars[3];
    double Cx = *_vars[4];
    double Cy = *_vars[5];

    // midpoint M
    double Mx = 0.5 * (Ax + Bx);
    double My = 0.5 * (Ay + By);

    // AB vector
    double dx = Bx - Ax;
    double dy = By - Ay;

    // MC vector
    double mx = Cx - Mx;
    double my = Cy - My;

    // Perpendicular condition: (AB · MC) = 0
    double dot = dx * mx + dy * my;

    return dot;
}

std::unordered_map<VAR, double> OurPaintDCM::Function::ArcCenterOnPerpendicularFunction::gradient() const {
    std::unordered_map<VAR, double> grad;

    double Ax = *_vars[0];
    double Ay = *_vars[1];
    double Bx = *_vars[2];
    double By = *_vars[3];
    double Cx = *_vars[4];
    double Cy = *_vars[5];

    double dx = Bx - Ax;
    double dy = By - Ay;
    double mx = Cx - 0.5 * (Ax + Bx);
    double my = Cy - 0.5 * (Ay + By);

    grad[_vars[0]] = -mx - 0.5 * dx; // df/dAx
    grad[_vars[1]] = -my - 0.5 * dy; // df/dAy
    grad[_vars[2]] = mx - 0.5 * dx; // df/dBx
    grad[_vars[3]] = my - 0.5 * dy; // df/dBy
    grad[_vars[4]] = dx; // df/dCx
    grad[_vars[5]] = dy; // df/dCy

    return grad;
}

size_t OurPaintDCM::Function::ArcCenterOnPerpendicularFunction::getVarCount() const {
    return 6;
}
