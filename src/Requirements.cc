#include "Requirements.h"
std::map<PARAMID, Variable*> VarsStorage::m_vars;
std::vector<Variable *> VarsStorage::getVars() {
    std::vector<Variable *> vars;
    for (auto var : m_vars) {
        vars.push_back(var.second);
    }
    return vars;
}
Variable* VarsStorage::addVar(PARAMID id){
    if (m_vars.find(id) == m_vars.end()) {
        m_vars[id] = new Variable(id);
    }
    return m_vars[id];
}
Variable* VarsStorage::getVar(PARAMID id) {
    if (m_vars.find(id) == m_vars.end()) {
        throw std::runtime_error("Variable not found");
    }
    return m_vars[id];
}

void VarsStorage::clearVars() {
    for (auto var : m_vars) {
        delete var.second;
    }
    m_vars.clear();
}
/*
RequirementData::RequirementData() {
    objects = std::vector<ID>();
    params = std::vector<double>();
}*/

// 1
ErrorFunctions* ReqPointSecDist::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqPointSecDist::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_p->x));
    res.push_back(&(m_p->y));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}
ReqPointSecDist::ReqPointSecDist(Point* p, Section* s, double dist) {
    m_p = p;
    m_s = s;
    d = dist;
    Variable* x1 = VarsStorage::addVar(&p->x);
    Variable* y1 = VarsStorage::addVar(&p->y);
    Variable* x2 = VarsStorage::addVar(&s->beg->x);
    Variable* y2 = VarsStorage::addVar(&s->beg->y);
    Variable* x3 = VarsStorage::addVar(&s->end->x);
    Variable* y3 = VarsStorage::addVar(&s->end->y);
    c_f = new PointSectionDistanceError({x1, y1, x2, y2, x3, y3}, d);
}

BoundBox2D ReqPointSecDist::getRectangle() const {
    return m_s->getBox() | m_p->getBox();
}

Requirement ReqPointSecDist::getType() const {
    return ET_POINTSECTIONDIST;
};



// 2
ReqPointOnSec::ReqPointOnSec(Point* p, Section* s) {
    m_p = p;
    m_s = s;
    Variable* x1 = VarsStorage::addVar(&p->x);
    Variable* y1 = VarsStorage::addVar(&p->y);
    Variable* x2 = VarsStorage::addVar(&s->beg->x);
    Variable* y2 = VarsStorage::addVar(&s->beg->y);
    Variable* x3 = VarsStorage::addVar(&s->end->x);
    Variable* y3 = VarsStorage::addVar(&s->end->y);
    c_f = new PointOnSectionError({x1, y1, x2, y2, x3, y3});
}
ErrorFunctions* ReqPointOnSec::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqPointOnSec::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_p->x));
    res.push_back(&(m_p->y));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}

BoundBox2D ReqPointOnSec::getRectangle() const {
    return m_s->getBox() | m_p->getBox();
}

Requirement ReqPointOnSec::getType() const {
    return ET_POINTONSECTION;
}


// 3
ReqPointPointDist::ReqPointPointDist(Point* p1, Point* p2, double dist) {
    m_p1 = p1;
    m_p2 = p2;
    v_dist = dist;
    Variable* x1 = VarsStorage::addVar(&p1->x);
    Variable* y1 = VarsStorage::addVar(&p1->y);
    Variable* x2 = VarsStorage::addVar(&p2->x);
    Variable* y2 = VarsStorage::addVar(&p2->y);
    c_f = new PointPointDistanceError({x1, y1, x2, y2}, v_dist);
}
ErrorFunctions* ReqPointPointDist::getFunction() const {
    return static_cast<ErrorFunctions*>(c_f->clone());
}
std::vector<PARAMID> ReqPointPointDist::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_p1->x));
    res.push_back(&(m_p1->y));
    res.push_back(&(m_p2->x));
    res.push_back(&(m_p2->y));
    return res;
}

BoundBox2D ReqPointPointDist::getRectangle() const {
    return m_p1->getBox() | m_p2->getBox();
}

Requirement ReqPointPointDist::getType() const {
    return ET_POINTPOINTDIST;
}


// 4
ReqPointOnPoint::ReqPointOnPoint(Point* p1, Point* p2) {
    m_p1 = p1;
    m_p2 = p2;
    Variable* x1 = VarsStorage::addVar(&p1->x);
    Variable* y1 = VarsStorage::addVar(&p1->y);
    Variable* x2 = VarsStorage::addVar(&p2->x);
    Variable* y2 = VarsStorage::addVar(&p2->y);
    c_f = new PointOnPointError({x1, y1, x2, y2});
}
ErrorFunctions* ReqPointOnPoint::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqPointOnPoint::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_p1->x));
    res.push_back(&(m_p1->y));
    res.push_back(&(m_p2->x));
    res.push_back(&(m_p2->y));
    return res;
}

BoundBox2D ReqPointOnPoint::getRectangle() const {
    return m_p2->getBox() | m_p1->getBox();
}

Requirement ReqPointOnPoint::getType() const {
    return ET_POINTONPOINT;
}


// 5
ReqSecCircleDist::ReqSecCircleDist(Section* s, Circle* c, double dist) {
    m_s = s;
    m_c = c;
    v_dist = dist;
    Variable* x1 = VarsStorage::addVar(&s->beg->x);
    Variable* y1 = VarsStorage::addVar(&s->beg->y);
    Variable* x2 = VarsStorage::addVar(&s->end->x);
    Variable* y2 = VarsStorage::addVar(&s->end->y);
    Variable* xc = VarsStorage::addVar(&c->center->x);
    Variable* yc = VarsStorage::addVar(&c->center->y);
    Variable* r = VarsStorage::addVar(&c->r);
    c_f = new SectionCircleDistanceError({x1, y1, x2, y2, xc, yc, r}, v_dist);
}
ErrorFunctions* ReqSecCircleDist::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqSecCircleDist::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_c->center->x));
    res.push_back(&(m_c->center->y));
    res.push_back(&(m_c->r));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}

BoundBox2D ReqSecCircleDist::getRectangle() const {
    return m_s->getBox() | m_c->getBox();
}

Requirement ReqSecCircleDist::getType() const {
    return ET_SECTIONCIRCLEDIST;
}


// 6
ReqSecOnCircle::ReqSecOnCircle(Section* s, Circle* c) {
    m_s = s;
    m_c = c;

    Variable* x1 = VarsStorage::addVar(&s->beg->x);
    Variable* y1 = VarsStorage::addVar(&s->beg->y);
    Variable* x2 = VarsStorage::addVar(&s->end->x);
    Variable* y2 = VarsStorage::addVar(&s->end->y);
    Variable* xc = VarsStorage::addVar(&c->center->x);
    Variable* yc = VarsStorage::addVar(&c->center->y);
    Variable* r = VarsStorage::addVar(&c->r);
    c_f = new SectionOnCircleError({x1, y1, x2, y2, xc, yc,r });
}
ErrorFunctions* ReqSecOnCircle::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqSecOnCircle::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_c->center->x));
    res.push_back(&(m_c->center->y));
    res.push_back(&(m_c->r));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}

BoundBox2D ReqSecOnCircle::getRectangle() const {
    return m_s->getBox() | m_c->getBox();
}

Requirement ReqSecOnCircle::getType() const {
    return ET_SECTIONONCIRCLE;
}


// 7
ReqSecInCircle::ReqSecInCircle(Section* s, Circle* c) {
    m_s = s;
    m_c = c;
    throw std::runtime_error("Not implemented");
}
ErrorFunctions* ReqSecInCircle::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqSecInCircle::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_c->center->x));
    res.push_back(&(m_c->center->y));
    res.push_back(&(m_c->r));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}

BoundBox2D ReqSecInCircle::getRectangle() const {
    return m_s->getBox() | m_c->getBox();
}

Requirement ReqSecInCircle::getType() const {
    return ET_SECTIONINCIRCLE;
}


// 8
ReqSecSecParallel::ReqSecSecParallel(Section* s1, Section* s2) {
    m_s1 = s1;
    m_s2 = s2;
    Variable* x1 = VarsStorage::addVar(&s1->beg->x);
    Variable* y1 = VarsStorage::addVar(&s1->beg->y);
    Variable* x2 = VarsStorage::addVar(&s1->end->x);
    Variable* y2 = VarsStorage::addVar(&s1->end->y);
    Variable* x3 = VarsStorage::addVar(&s2->beg->x);
    Variable* y3 = VarsStorage::addVar(&s2->beg->y);
    Variable* x4 = VarsStorage::addVar(&s2->end->x);
    Variable* y4 = VarsStorage::addVar(&s2->end->y);
    c_f = new SectionSectionParallelError({x1, y1, x2, y2, x3, y3, x4, y4});
}
// ad-bc. Normal vectors (x2-x1, y2-y1) and (x4-x3, y4-y3)
ErrorFunctions* ReqSecSecParallel::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqSecSecParallel::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_s1->beg->x));
    res.push_back(&(m_s1->beg->y));
    res.push_back(&(m_s1->end->x));
    res.push_back(&(m_s1->end->y));
    res.push_back(&(m_s2->beg->x));
    res.push_back(&(m_s2->beg->y));
    res.push_back(&(m_s2->end->x));
    res.push_back(&(m_s2->end->y));
    return res;
}
//  (x2-x1)*(y4-y3)-(y2-y1)*(x4-x3)

BoundBox2D ReqSecSecParallel::getRectangle() const {
    return m_s1->getBox() | m_s2->getBox();
}

Requirement ReqSecSecParallel::getType() const {
    return ET_SECTIONSECTIONPARALLEL;
}


// 9
ReqSecSecPerpendicular::ReqSecSecPerpendicular(Section* s1, Section* s2) {
    m_s1 = s1;
    m_s2 = s2;
    Variable* x1 = VarsStorage::addVar(&s1->beg->x);
    Variable* y1 = VarsStorage::addVar(&s1->beg->y);
    Variable* x2 = VarsStorage::addVar(&s1->end->x);
    Variable* y2 = VarsStorage::addVar(&s1->end->y);
    Variable* x3 = VarsStorage::addVar(&s2->beg->x);
    Variable* y3 = VarsStorage::addVar(&s2->beg->y);
    Variable* x4 = VarsStorage::addVar(&s2->end->x);
    Variable* y4 = VarsStorage::addVar(&s2->end->y);
    c_f = new SectionSectionPerpendicularError({x1, y1, x2, y2, x3, y3, x4, y4});
}
ErrorFunctions* ReqSecSecPerpendicular::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqSecSecPerpendicular::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_s1->beg->x));
    res.push_back(&(m_s1->beg->y));
    res.push_back(&(m_s1->end->x));
    res.push_back(&(m_s1->end->y));
    res.push_back(&(m_s2->beg->x));
    res.push_back(&(m_s2->beg->y));
    res.push_back(&(m_s2->end->x));
    res.push_back(&(m_s2->end->y));
    return res;
}

BoundBox2D ReqSecSecPerpendicular::getRectangle() const {
    return m_s1->getBox() | m_s2->getBox();
}

Requirement ReqSecSecPerpendicular::getType() const {
    return ET_SECTIONSECTIONPERPENDICULAR;
}


// 10
ReqSecSecAngel::ReqSecSecAngel(Section* s1, Section* s2, double angle) {
    m_s1 = s1;
    m_s2 = s2;
    desired_angle = angle;
    Variable* x1 = VarsStorage::addVar(&s1->beg->x);
    Variable* y1 = VarsStorage::addVar(&s1->beg->y);
    Variable* x2 = VarsStorage::addVar(&s1->end->x);
    Variable* y2 = VarsStorage::addVar(&s1->end->y);
    Variable* x3 = VarsStorage::addVar(&s2->beg->x);
    Variable* y3 = VarsStorage::addVar(&s2->beg->y);
    Variable* x4 = VarsStorage::addVar(&s2->end->x);
    Variable* y4 = VarsStorage::addVar(&s2->end->y);
    c_f = new SectionSectionAngleError({x1, y1, x2, y2, x3, y3, x4, y4}, desired_angle);
}
ErrorFunctions* ReqSecSecAngel::getFunction() const {
    return c_f;
}
std::vector<PARAMID> ReqSecSecAngel::getParams() const {
    std::vector<PARAMID> res;
    res.push_back(&(m_s1->beg->x));
    res.push_back(&(m_s1->beg->y));
    res.push_back(&(m_s1->end->x));
    res.push_back(&(m_s1->end->y));
    res.push_back(&(m_s2->beg->x));
    res.push_back(&(m_s2->beg->y));
    res.push_back(&(m_s2->end->x));
    res.push_back(&(m_s2->end->y));
    return res;
}

BoundBox2D ReqSecSecAngel::getRectangle() const {
    return m_s1->getBox() | m_s2->getBox();
}

Requirement ReqSecSecAngel::getType() const {
    return ET_SECTIONSECTIONANGLE;
}

