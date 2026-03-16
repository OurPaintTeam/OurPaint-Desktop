#include "DCMManager.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <iomanip>

using namespace OurPaintDCM;
using namespace OurPaintDCM::Utils;

static const char* figureTypeName(FigureType t) {
    switch (t) {
        case FigureType::ET_POINT2D: return "Point";
        case FigureType::ET_LINE:    return "Line";
        case FigureType::ET_CIRCLE:  return "Circle";
        case FigureType::ET_ARC:     return "Arc";
    }
    return "?";
}

static const char* reqTypeName(RequirementType t) {
    switch (t) {
        case RequirementType::ET_POINTLINEDIST:          return "PointLineDist";
        case RequirementType::ET_POINTONLINE:            return "PointOnLine";
        case RequirementType::ET_POINTPOINTDIST:         return "PointPointDist";
        case RequirementType::ET_POINTONPOINT:           return "PointOnPoint";
        case RequirementType::ET_LINECIRCLEDIST:         return "LineCircleDist";
        case RequirementType::ET_LINEONCIRCLE:           return "LineOnCircle";
        case RequirementType::ET_LINEINCIRCLE:           return "LineInCircle";
        case RequirementType::ET_LINELINEPARALLEL:       return "Parallel";
        case RequirementType::ET_LINELINEPERPENDICULAR:  return "Perpendicular";
        case RequirementType::ET_LINELINEANGLE:          return "LineAngle";
        case RequirementType::ET_VERTICAL:               return "Vertical";
        case RequirementType::ET_HORIZONTAL:             return "Horizontal";
        case RequirementType::ET_ARCCENTERONPERPENDICULAR: return "ArcCenterPerp";
    }
    return "?";
}

static const char* modeName(SolveMode m) {
    switch (m) {
        case SolveMode::GLOBAL: return "GLOBAL";
        case SolveMode::LOCAL:  return "LOCAL";
        case SolveMode::DRAG:   return "DRAG";
    }
    return "?";
}

static const char* statusName(SystemStatus s) {
    switch (s) {
        case SystemStatus::WELL_CONSTRAINED:  return "Well-constrained";
        case SystemStatus::UNDER_CONSTRAINED: return "Under-constrained";
        case SystemStatus::OVER_CONSTRAINED:  return "Over-constrained";
        case SystemStatus::SINGULAR_SYSTEM:   return "Singular";
        case SystemStatus::EMPTY:             return "Empty";
        case SystemStatus::UNKNOWN:           return "Unknown";
    }
    return "?";
}

static void printHelp() {
    std::cout << R"(
=== DCM Console ===

Figures:
  add point <x> <y>                   - add 2D point
  add line <x1> <y1> <x2> <y2>        - add line by coordinates
  add circle <cx> <cy> <radius>       - add circle by center coordinates
  add arc <x1> <y1> <x2> <y2> <cx> <cy> - add arc by coordinates
  remove figure <id> [cascade]        - remove figure (cascade = force)
  update point <id> <x> <y>           - update point coords
  update circle <id> <radius>         - update circle radius

Requirements:
  req pp_dist <p1> <p2> <dist>        - point-point distance
  req pp_on <p1> <p2>                 - point on point
  req pl_dist <p> <l> <dist>          - point-line distance
  req pl_on <p> <l>                   - point on line
  req lc_dist <l> <c> <dist>          - line-circle distance
  req lc_on <l> <c>                   - line on circle
  req parallel <l1> <l2>              - parallel lines
  req perp <l1> <l2>                  - perpendicular lines
  req angle <l1> <l2> <angle>         - angle between lines
  req vertical <l>                    - vertical line
  req horizontal <l>                  - horizontal line
  remove req <id>                     - remove requirement
  update req <id> <param>             - update requirement param

Solver:
  mode global                         - set GLOBAL mode
  mode local                          - set LOCAL mode
  mode drag                           - set DRAG mode
  solve [component_id]                - solve (component_id for LOCAL)

Info:
  info                                - all figures + requirements + components
  status                              - system diagnosis
  help                                - this help
  quit / exit                         - exit
)";
}

static void printFigure(const FigureDescriptor& d) {
    std::cout << "  [" << d.id.value().id << "] " << figureTypeName(d.type);
    if (d.type == FigureType::ET_POINT2D) {
        std::cout << "  (" << std::fixed << std::setprecision(4)
                  << d.x.value() << ", " << d.y.value() << ")";
    } else if (d.type == FigureType::ET_CIRCLE) {
        std::cout << "  center=" << d.pointIds[0].id
                  << "  r=" << std::fixed << std::setprecision(4) << d.radius.value();
    } else if (d.type == FigureType::ET_LINE) {
        std::cout << "  pts=[" << d.pointIds[0].id << ", " << d.pointIds[1].id << "]";
    } else if (d.type == FigureType::ET_ARC) {
        std::cout << "  pts=[" << d.pointIds[0].id << ", "
                  << d.pointIds[1].id << ", " << d.pointIds[2].id << "]";
    }
    std::cout << "\n";
}

static void printReq(const RequirementDescriptor& d) {
    std::cout << "  [" << d.id.value().id << "] " << reqTypeName(d.type) << "  objs={";
    for (std::size_t i = 0; i < d.objectIds.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << d.objectIds[i].id;
    }
    std::cout << "}";
    if (d.param.has_value()) {
        std::cout << "  param=" << std::fixed << std::setprecision(4) << d.param.value();
    }
    std::cout << "\n";
}

static void printInfo(DCMManager& mgr) {
    std::cout << "\n--- Figures (" << mgr.figureCount() << ") ---\n";
    for (auto& f : mgr.getAllFigures()) printFigure(f);

    std::cout << "--- Requirements (" << mgr.requirementCount() << ") ---\n";
    for (auto& r : mgr.getAllRequirements()) printReq(r);

    std::cout << "--- Components (" << mgr.getComponentCount() << ") ---\n";
    auto comps = mgr.getAllComponents();
    for (std::size_t i = 0; i < comps.size(); ++i) {
        std::cout << "  comp " << i << ": {";
        for (std::size_t j = 0; j < comps[i].size(); ++j) {
            if (j) std::cout << ", ";
            std::cout << comps[i][j].id;
        }
        std::cout << "}\n";
    }

    std::cout << "--- Mode: " << modeName(mgr.getSolveMode()) << " ---\n\n";
}

int main() {
    DCMManager mgr;
    printHelp();

    std::string line;
    while (true) {
        std::cout << "dcm> ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd.empty()) continue;

        try {
            if (cmd == "quit" || cmd == "exit") {
                break;
            }
            else if (cmd == "help") {
                printHelp();
            }
            else if (cmd == "info") {
                printInfo(mgr);
            }
            else if (cmd == "status") {
                auto st = mgr.getRequirementSystem().diagnose();
                std::cout << "System: " << statusName(st) << "\n";
            }
            else if (cmd == "add") {
                std::string what;
                ss >> what;
                if (what == "point") {
                    double x, y;
                    ss >> x >> y;
                    auto id = mgr.addFigure(FigureDescriptor::point(x, y));
                    std::cout << "Point created, id=" << id.id << "\n";
                }
                else if (what == "line") {
                    double x1, y1, x2, y2;
                    ss >> x1 >> y1 >> x2 >> y2;
                    auto id = mgr.addFigure(FigureDescriptor::line(x1, y1, x2, y2));
                    auto desc = mgr.getFigure(id);
                    std::cout << "Line created, id=" << id.id
                              << " (pts: " << desc->pointIds[0].id
                              << ", " << desc->pointIds[1].id << ")\n";
                }
                else if (what == "circle") {
                    double cx, cy, r;
                    ss >> cx >> cy >> r;
                    auto id = mgr.addFigure(FigureDescriptor::circle(cx, cy, r));
                    auto desc = mgr.getFigure(id);
                    std::cout << "Circle created, id=" << id.id
                              << " (center: " << desc->pointIds[0].id << ")\n";
                }
                else if (what == "arc") {
                    double x1, y1, x2, y2, cx, cy;
                    ss >> x1 >> y1 >> x2 >> y2 >> cx >> cy;
                    auto id = mgr.addFigure(FigureDescriptor::arc(x1, y1, x2, y2, cx, cy));
                    auto desc = mgr.getFigure(id);
                    std::cout << "Arc created, id=" << id.id
                              << " (pts: " << desc->pointIds[0].id
                              << ", " << desc->pointIds[1].id
                              << ", center: " << desc->pointIds[2].id << ")\n";
                }
                else {
                    std::cout << "Unknown: add " << what << "\n";
                }
            }
            else if (cmd == "remove") {
                std::string what;
                ss >> what;
                if (what == "figure") {
                    unsigned long long fid;
                    ss >> fid;
                    std::string opt;
                    bool cascade = false;
                    if (ss >> opt && opt == "cascade") cascade = true;
                    mgr.removeFigure(ID(fid), cascade);
                    std::cout << "Figure " << fid << " removed\n";
                }
                else if (what == "req") {
                    unsigned long long rid;
                    ss >> rid;
                    mgr.removeRequirement(ID(rid));
                    std::cout << "Requirement " << rid << " removed\n";
                }
                else {
                    std::cout << "Unknown: remove " << what << "\n";
                }
            }
            else if (cmd == "update") {
                std::string what;
                ss >> what;
                if (what == "point") {
                    unsigned long long pid; double x, y;
                    ss >> pid >> x >> y;
                    mgr.updatePoint(PointUpdateDescriptor(ID(pid), x, y));
                    std::cout << "Point " << pid << " updated\n";
                }
                else if (what == "circle") {
                    unsigned long long cid; double r;
                    ss >> cid >> r;
                    mgr.updateCircle(CircleUpdateDescriptor(ID(cid), r));
                    std::cout << "Circle " << cid << " updated\n";
                }
                else if (what == "req") {
                    unsigned long long rid; double p;
                    ss >> rid >> p;
                    mgr.updateRequirementParam(ID(rid), p);
                    std::cout << "Requirement " << rid << " param updated\n";
                }
                else {
                    std::cout << "Unknown: update " << what << "\n";
                }
            }
            else if (cmd == "req") {
                std::string what;
                ss >> what;
                if (what == "pp_dist") {
                    unsigned long long a, b; double d;
                    ss >> a >> b >> d;
                    auto id = mgr.addRequirement(RequirementDescriptor::pointPointDist(ID(a), ID(b), d));
                    std::cout << "PointPointDist added, id=" << id.id << "\n";
                }
                else if (what == "pp_on") {
                    unsigned long long a, b;
                    ss >> a >> b;
                    auto id = mgr.addRequirement(RequirementDescriptor::pointOnPoint(ID(a), ID(b)));
                    std::cout << "PointOnPoint added, id=" << id.id << "\n";
                }
                else if (what == "pl_dist") {
                    unsigned long long p, l; double d;
                    ss >> p >> l >> d;
                    auto id = mgr.addRequirement(RequirementDescriptor::pointLineDist(ID(p), ID(l), d));
                    std::cout << "PointLineDist added, id=" << id.id << "\n";
                }
                else if (what == "pl_on") {
                    unsigned long long p, l;
                    ss >> p >> l;
                    auto id = mgr.addRequirement(RequirementDescriptor::pointOnLine(ID(p), ID(l)));
                    std::cout << "PointOnLine added, id=" << id.id << "\n";
                }
                else if (what == "lc_dist") {
                    unsigned long long l, c; double d;
                    ss >> l >> c >> d;
                    auto id = mgr.addRequirement(RequirementDescriptor::lineCircleDist(ID(l), ID(c), d));
                    std::cout << "LineCircleDist added, id=" << id.id << "\n";
                }
                else if (what == "lc_on") {
                    unsigned long long l, c;
                    ss >> l >> c;
                    auto id = mgr.addRequirement(RequirementDescriptor::lineOnCircle(ID(l), ID(c)));
                    std::cout << "LineOnCircle added, id=" << id.id << "\n";
                }
                else if (what == "parallel") {
                    unsigned long long a, b;
                    ss >> a >> b;
                    auto id = mgr.addRequirement(RequirementDescriptor::lineLineParallel(ID(a), ID(b)));
                    std::cout << "Parallel added, id=" << id.id << "\n";
                }
                else if (what == "perp") {
                    unsigned long long a, b;
                    ss >> a >> b;
                    auto id = mgr.addRequirement(RequirementDescriptor::lineLinePerpendicular(ID(a), ID(b)));
                    std::cout << "Perpendicular added, id=" << id.id << "\n";
                }
                else if (what == "angle") {
                    unsigned long long a, b; double ang;
                    ss >> a >> b >> ang;
                    auto id = mgr.addRequirement(RequirementDescriptor::lineLineAngle(ID(a), ID(b), ang));
                    std::cout << "LineAngle added, id=" << id.id << "\n";
                }
                else if (what == "vertical") {
                    unsigned long long l;
                    ss >> l;
                    auto id = mgr.addRequirement(RequirementDescriptor::vertical(ID(l)));
                    std::cout << "Vertical added, id=" << id.id << "\n";
                }
                else if (what == "horizontal") {
                    unsigned long long l;
                    ss >> l;
                    auto id = mgr.addRequirement(RequirementDescriptor::horizontal(ID(l)));
                    std::cout << "Horizontal added, id=" << id.id << "\n";
                }
                else {
                    std::cout << "Unknown requirement: " << what << "\n";
                }
            }
            else if (cmd == "mode") {
                std::string m;
                ss >> m;
                if (m == "global")     mgr.setSolveMode(SolveMode::GLOBAL);
                else if (m == "local") mgr.setSolveMode(SolveMode::LOCAL);
                else if (m == "drag")  mgr.setSolveMode(SolveMode::DRAG);
                else { std::cout << "Unknown mode: " << m << "\n"; continue; }
                std::cout << "Mode set to " << modeName(mgr.getSolveMode()) << "\n";
            }
            else if (cmd == "solve") {
                std::optional<ComponentID> compId;
                unsigned long long c;
                if (ss >> c) compId = static_cast<ComponentID>(c);

                bool ok = mgr.solve(compId);
                std::cout << (ok ? "Converged" : "Did NOT converge") << "\n";
            }
            else if (cmd == "clear") {
                mgr.clear();
                std::cout << "Cleared\n";
            }
            else {
                std::cout << "Unknown command. Type 'help'.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}
