#include "Requirements.h"

namespace OurPaintDCM::Requirements {
// ----------------------------------------------------------------
PointLineDist::PointLineDist(Figures::Point2D* p, Figures::Line<Figures::Point2D>* l, double dist)
    : Requirement(Utils::RequirementType::ET_POINTLINEDIST),
      _p(p),
      _l(l),
      _dist(dist) {
}

ErrorFunction* PointLineDist::toFunction() {
    std::vector<Variable*> x;
    auto*                  p_x      = new Variable(_p->ptrX());
    auto*                  p_y      = new Variable(_p->ptrY());
    auto*                  l_startx = new Variable(_l->p1->ptrX());
    auto*                  l_starty = new Variable(_l->p1->ptrY());
    auto*                  l_endx   = new Variable(_l->p2->ptrX());
    auto*                  l_endy   = new Variable(_l->p2->ptrY());
    x.push_back(p_x);
    x.push_back(p_y);
    x.push_back(l_startx);
    x.push_back(l_starty);
    x.push_back(l_endx);
    x.push_back(l_endy);
    return new PointSectionDistanceError(x, _dist);
}

//-----------------------------------------------------------------
PointOnLine::PointOnLine(Figures::Point2D* p, Figures::Line<Figures::Point2D>* l)
    : Requirement(Utils::RequirementType::ET_POINTONLINE),
      _p(p),
      _l(l) {
}

ErrorFunction* PointOnLine::toFunction() {
    std::vector<Variable*> x;
    auto*                  p_x      = new Variable(_p->ptrX());
    auto*                  p_y      = new Variable(_p->ptrY());
    auto*                  l_startx = new Variable(_l->p1->ptrX());
    auto*                  l_starty = new Variable(_l->p1->ptrY());
    auto*                  l_endx   = new Variable(_l->p2->ptrX());
    auto*                  l_endy   = new Variable(_l->p2->ptrY());
    x.push_back(p_x);
    x.push_back(p_y);
    x.push_back(l_startx);
    x.push_back(l_starty);
    x.push_back(l_endx);
    x.push_back(l_endy);
    return new PointOnSectionError(x);
}

//-----------------------------------------------------------------
PointPointDist::PointPointDist(Figures::Point2D* p1, Figures::Point2D* p2, double dist)
    : Requirement(Utils::RequirementType::ET_POINTPOINTDIST),
      _p1(p1),
      _p2(p2),
      _dist(dist) {
}

ErrorFunction* PointPointDist::toFunction() {
    std::vector<Variable*> x;
    auto*                  p1_x = new Variable(_p1->ptrX());
    auto*                  p1_y = new Variable(_p1->ptrY());
    auto*                  p2_x = new Variable(_p2->ptrX());
    auto*                  p2_y = new Variable(_p2->ptrY());
    x.push_back(p1_x);
    x.push_back(p1_y);
    x.push_back(p2_x);
    x.push_back(p2_y);
    return new PointPointDistanceError(x, _dist);
}

// -----------------------------------------------------------------
PointOnPoint::PointOnPoint(Figures::Point2D* p1, Figures::Point2D* p2)
    : Requirement(Utils::RequirementType::ET_POINTONPOINT),
      _p1(p1),
      _p2(p2) {
}

ErrorFunction* PointOnPoint::toFunction() {
    std::vector<Variable*> x;
    auto*                  p1_x = new Variable(_p1->ptrX());
    auto*                  p1_y = new Variable(_p1->ptrY());
    auto*                  p2_x = new Variable(_p2->ptrX());
    auto*                  p2_y = new Variable(_p2->ptrY());
    x.push_back(p1_x);
    x.push_back(p1_y);
    x.push_back(p2_x);
    x.push_back(p2_y);
    return new PointOnPointError(x);
}

// ----------------------------------------------------------------
LineCircleDist::LineCircleDist(Figures::Line<Figures::Point2D>*   line,
                               Figures::Circle<Figures::Point2D>* circle,
                               double                             dist)
    : Requirement(Utils::RequirementType::ET_LINECIRCLEDIST),
      _l(line),
      _c(circle),
      _dist(dist) {
}

ErrorFunction* LineCircleDist::toFunction() {
    std::vector<Variable*> x;
    auto*                  l_startx = new Variable(_l->p1->ptrX());
    auto*                  l_starty = new Variable(_l->p1->ptrY());
    auto*                  l_endx   = new Variable(_l->p2->ptrX());
    auto*                  l_endy   = new Variable(_l->p2->ptrY());
    auto*                  centerx  = new Variable(_c->center->ptrX());
    auto*                  centery  = new Variable(_c->center->ptrY());
    auto*                  radius   = new Variable(&_c->radius);
    x.push_back(l_startx);
    x.push_back(l_starty);
    x.push_back(l_endx);
    x.push_back(l_endy);
    x.push_back(centerx);
    x.push_back(centery);
    x.push_back(radius);
    return new SectionCircleDistanceError(x, _dist);
}

// ------------------------------------------------------------------
LineOnCircle::LineOnCircle(Figures::Line<Figures::Point2D>* line, Figures::Circle<Figures::Point2D>* circle)
    : Requirement(Utils::RequirementType::ET_LINEONCIRCLE),
      _l(line),
      _c(circle) {
}

ErrorFunction* LineOnCircle::toFunction() {
    std::vector<Variable*> x;
    auto*                  l_startx = new Variable(_l->p1->ptrX());
    auto*                  l_starty = new Variable(_l->p1->ptrY());
    auto*                  l_endx   = new Variable(_l->p2->ptrX());
    auto*                  l_endy   = new Variable(_l->p2->ptrY());
    auto*                  centerx  = new Variable(_c->center->ptrX());
    auto*                  centery  = new Variable(_c->center->ptrY());
    auto*                  radius   = new Variable(&_c->radius);
    x.push_back(l_startx);
    x.push_back(l_starty);
    x.push_back(l_endx);
    x.push_back(l_endy);
    x.push_back(centerx);
    x.push_back(centery);
    x.push_back(radius);
    return new SectionOnCircleError(x);
}

// ----------------------------------------------------------------
LineInCircle::LineInCircle(Figures::Line<Figures::Point2D>* line, Figures::Circle<Figures::Point2D>* circle)
    : Requirement(Utils::RequirementType::ET_LINEINCIRCLE),
      _l(line),
      _c(circle) {
}

ErrorFunction* LineInCircle::toFunction() {
    std::vector<Variable*> x;
    auto*                  l_startx = new Variable(_l->p1->ptrX());
    auto*                  l_starty = new Variable(_l->p1->ptrY());
    auto*                  l_endx   = new Variable(_l->p2->ptrX());
    auto*                  l_endy   = new Variable(_l->p2->ptrY());
    auto*                  centerx  = new Variable(_c->center->ptrX());
    auto*                  centery  = new Variable(_c->center->ptrY());
    auto*                  radius   = new Variable(&_c->radius);
    x.push_back(l_startx);
    x.push_back(l_starty);
    x.push_back(l_endx);
    x.push_back(l_endy);
    x.push_back(centerx);
    x.push_back(centery);
    x.push_back(radius);
    return new SectionInCircleError(x);
}

// -----------------------------------------------------------------

LineLineParallel::LineLineParallel(Figures::Line<Figures::Point2D>* l1, Figures::Line<Figures::Point2D>* l2)
    : Requirement(Utils::RequirementType::ET_LINELINEPARALLEL),
      _l1(l1),
      _l2(l2) {
}

ErrorFunction* LineLineParallel::toFunction() {
    std::vector<Variable*> x;
    auto*                  l1_startx = new Variable(_l1->p1->ptrX());
    auto*                  l1_starty = new Variable(_l1->p1->ptrY());
    auto*                  l1_endx   = new Variable(_l1->p2->ptrX());
    auto*                  l1_endy   = new Variable(_l1->p2->ptrY());
    auto*                  l2_startx = new Variable(_l2->p1->ptrX());
    auto*                  l2_starty = new Variable(_l2->p1->ptrY());
    auto*                  l2_endx   = new Variable(_l2->p2->ptrX());
    auto*                  l2_endy   = new Variable(_l2->p2->ptrY());
    x.push_back(l1_startx);
    x.push_back(l1_starty);
    x.push_back(l1_endx);
    x.push_back(l1_endy);
    x.push_back(l2_startx);
    x.push_back(l2_starty);
    x.push_back(l2_endx);
    x.push_back(l2_endy);
    return new SectionSectionParallelError(x);
}

// -------------------------------------------------------------------
LineLinePerpendicular::LineLinePerpendicular(Figures::Line<Figures::Point2D>* l1, Figures::Line<Figures::Point2D>* l2)
    : Requirement(Utils::RequirementType::ET_LINELINEPERPENDICULAR),
      _l1(l1),
      _l2(l2) {
}

ErrorFunction* LineLinePerpendicular::toFunction() {
    std::vector<Variable*> x;
    auto*                  l1_startx = new Variable(_l1->p1->ptrX());
    auto*                  l1_starty = new Variable(_l1->p1->ptrY());
    auto*                  l1_endx   = new Variable(_l1->p2->ptrX());
    auto*                  l1_endy   = new Variable(_l1->p2->ptrY());
    auto*                  l2_startx = new Variable(_l2->p1->ptrX());
    auto*                  l2_starty = new Variable(_l2->p1->ptrY());
    auto*                  l2_endx   = new Variable(_l2->p2->ptrX());
    auto*                  l2_endy   = new Variable(_l2->p2->ptrY());
    x.push_back(l1_startx);
    x.push_back(l1_starty);
    x.push_back(l1_endx);
    x.push_back(l1_endy);
    x.push_back(l2_startx);
    x.push_back(l2_starty);
    x.push_back(l2_endx);
    x.push_back(l2_endy);
    return new SectionSectionPerpendicularError(x);
}

// -----------------------------------------------------------------------
LineLineAngle::LineLineAngle(Figures::Line<Figures::Point2D>* l1, Figures::Line<Figures::Point2D>* l2, double angle)
    : Requirement(Utils::RequirementType::ET_LINELINEANGLE),
      _l1(l1),
      _l2(l2),
      _angle(angle) {
}

ErrorFunction* LineLineAngle::toFunction() {
    std::vector<Variable*> x;
    auto*                  l1_startx = new Variable(_l1->p1->ptrX());
    auto*                  l1_starty = new Variable(_l1->p1->ptrY());
    auto*                  l1_endx   = new Variable(_l1->p2->ptrX());
    auto*                  l1_endy   = new Variable(_l1->p2->ptrY());
    auto*                  l2_startx = new Variable(_l2->p1->ptrX());
    auto*                  l2_starty = new Variable(_l2->p1->ptrY());
    auto*                  l2_endx   = new Variable(_l2->p2->ptrX());
    auto*                  l2_endy   = new Variable(_l2->p2->ptrY());
    x.push_back(l1_startx);
    x.push_back(l1_starty);
    x.push_back(l1_endx);
    x.push_back(l1_endy);
    x.push_back(l2_startx);
    x.push_back(l2_starty);
    x.push_back(l2_endx);
    x.push_back(l2_endy);
    return new SectionSectionAngleError(x, _angle);
}

// ---------------------------------------------------------------------
LineHorizontal::LineHorizontal(Figures::Line<Figures::Point2D>* l) : Requirement(Utils::RequirementType::ET_HORIZONTAL),
_l(l){}
ErrorFunction* LineHorizontal::toFunction() {
    std::vector<Variable*> x;
    auto*                  l1_startx = new Variable(_l->p1->ptrX());
    auto*                  l1_starty = new Variable(_l->p1->ptrY());
    auto*                  l1_endx   = new Variable(_l->p2->ptrX());
    auto*                  l1_endy   = new Variable(_l->p2->ptrY());
    x.push_back(l1_startx);
    x.push_back(l1_starty);
    x.push_back(l1_endx);
    x.push_back(l1_endy);
    return new HorizontalError(x);
}
// ---------------------------------------------------------------------
LineVertical::LineVertical(Figures::Line<Figures::Point2D>* l) : Requirement(Utils::RequirementType::ET_VERTICAL),
_l(l){}
ErrorFunction* LineVertical::toFunction() {
    std::vector<Variable*> x;
    auto*                  l1_startx = new Variable(_l->p1->ptrX());
    auto*                  l1_starty = new Variable(_l->p1->ptrY());
    auto*                  l1_endx   = new Variable(_l->p2->ptrX());
    auto*                  l1_endy   = new Variable(_l->p2->ptrY());
    x.push_back(l1_startx);
    x.push_back(l1_starty);
    x.push_back(l1_endx);
    x.push_back(l1_endy);
    return new VerticalError(x);
}
}