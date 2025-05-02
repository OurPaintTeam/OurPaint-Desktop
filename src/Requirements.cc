#include "Requirements.h"


// 1
ErrorFunctions* ReqPointSecDist::getFunction() {
    return new PointSectionDistanceError(getVariables(), d);
}
std::vector<PARAM_ID> ReqPointSecDist::getParams() const {
    std::vector<PARAM_ID> res;
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
}
Requirement ReqPointSecDist::getType() const {
    return ET_POINTSECTIONDIST;
}
std::vector<IGeometricObject*> ReqPointSecDist::getObjects() {
    return {m_p, m_s};
}
std::vector<Variable*> ReqPointSecDist::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_p->x),
            new Variable(&m_p->y),
            new Variable(&m_s->beg->x),
            new Variable(&m_s->beg->y),
            new Variable(&m_s->end->x),
            new Variable(&m_s->end->y)
    };
    return variables;
}


// 2
ReqPointOnSec::ReqPointOnSec(Point* p, Section* s) {
    m_p = p;
    m_s = s;
}
ErrorFunctions* ReqPointOnSec::getFunction() {
    return new PointOnSectionError(getVariables());
}
std::vector<PARAM_ID> ReqPointOnSec::getParams() const {
    std::vector<PARAM_ID> res;
    res.push_back(&(m_p->x));
    res.push_back(&(m_p->y));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}
Requirement ReqPointOnSec::getType() const {
    return ET_POINTONSECTION;
}
std::vector<IGeometricObject*> ReqPointOnSec::getObjects() {
    return {m_p, m_s};
}
std::vector<Variable*> ReqPointOnSec::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_p->x),
            new Variable(&m_p->y),
            new Variable(&m_s->beg->x),
            new Variable(&m_s->beg->y),
            new Variable(&m_s->end->x),
            new Variable(&m_s->end->y)
    };
    return variables;
}


// 3
ReqPointPointDist::ReqPointPointDist(Point* p1, Point* p2, double dist) {
    m_p1 = p1;
    m_p2 = p2;
    v_dist = dist;
}
ErrorFunctions* ReqPointPointDist::getFunction() {
    return new PointPointDistanceError(getVariables(), v_dist);
}
std::vector<PARAM_ID> ReqPointPointDist::getParams() const {
    std::vector<PARAM_ID> res;
    res.push_back(&(m_p1->x));
    res.push_back(&(m_p1->y));
    res.push_back(&(m_p2->x));
    res.push_back(&(m_p2->y));
    double dist_copy = v_dist;
    res.push_back(&dist_copy);
    return res;
}
Requirement ReqPointPointDist::getType() const {
    return ET_POINTPOINTDIST;
}
std::vector<IGeometricObject*> ReqPointPointDist::getObjects() {
    return {m_p1, m_p2};
}
std::vector<Variable*> ReqPointPointDist::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_p1->x),
            new Variable(&m_p1->y),
            new Variable(&m_p2->x),
            new Variable(&m_p2->y)
    };
    return variables;
}


// 4
ReqPointOnPoint::ReqPointOnPoint(Point* p1, Point* p2) {
    m_p1 = p1;
    m_p2 = p2;
}
ErrorFunctions* ReqPointOnPoint::getFunction() {
    return new PointOnPointError(getVariables());
}
std::vector<PARAM_ID> ReqPointOnPoint::getParams() const {
    std::vector<PARAM_ID> res;
    res.push_back(&(m_p1->x));
    res.push_back(&(m_p1->y));
    res.push_back(&(m_p2->x));
    res.push_back(&(m_p2->y));
    return res;
}
Requirement ReqPointOnPoint::getType() const {
    return ET_POINTONPOINT;
}
std::vector<IGeometricObject*> ReqPointOnPoint::getObjects() {
    return {m_p1, m_p2};
}
std::vector<Variable*> ReqPointOnPoint::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_p1->x),
            new Variable(&m_p1->y),
            new Variable(&m_p2->x),
            new Variable(&m_p2->y)
    };
    return variables;
};




// 5
ReqSecCircleDist::ReqSecCircleDist(Section* s, Circle* c, double dist) {
    m_s = s;
    m_c = c;
    v_dist = dist;

}
ErrorFunctions* ReqSecCircleDist::getFunction() {
    return new SectionCircleDistanceError(getVariables(), v_dist);
}
std::vector<PARAM_ID> ReqSecCircleDist::getParams() const {
    std::vector<PARAM_ID> res;
    res.push_back(&(m_c->center->x));
    res.push_back(&(m_c->center->y));
    res.push_back(&(m_c->r));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}
Requirement ReqSecCircleDist::getType() const {
    return ET_SECTIONCIRCLEDIST;
}
std::vector<IGeometricObject*> ReqSecCircleDist::getObjects() {
    return {m_s, m_c};
}
std::vector<Variable*> ReqSecCircleDist::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_s->beg->x),
            new Variable(&m_s->beg->y),
            new Variable(&m_s->end->x),
            new Variable(&m_s->end->y),
            new Variable(&m_c->center->x),
            new Variable(&m_c->center->y),
            new Variable(&m_c->r)
    };
    return variables;
}


// 6
ReqSecOnCircle::ReqSecOnCircle(Section* s, Circle* c) {
    m_s = s;
    m_c = c;
}
ErrorFunctions* ReqSecOnCircle::getFunction() {
    return new SectionOnCircleError(getVariables());
}
std::vector<PARAM_ID> ReqSecOnCircle::getParams() const {
    std::vector<PARAM_ID> res;
    res.push_back(&(m_c->center->x));
    res.push_back(&(m_c->center->y));
    res.push_back(&(m_c->r));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}
Requirement ReqSecOnCircle::getType() const {
    return ET_SECTIONONCIRCLE;
}
std::vector<IGeometricObject*> ReqSecOnCircle::getObjects() {
    return {m_s, m_c};
}
std::vector<Variable*> ReqSecOnCircle::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_s->beg->x),
            new Variable(&m_s->beg->y),
            new Variable(&m_s->end->x),
            new Variable(&m_s->end->y),
            new Variable(&m_c->center->x),
            new Variable(&m_c->center->y),
            new Variable(&m_c->r)
    };
    return variables;
}


// 7
ReqSecInCircle::ReqSecInCircle(Section* s, Circle* c) {
    m_s = s;
    m_c = c;
    throw std::runtime_error("Not implemented");
}
ErrorFunctions* ReqSecInCircle::getFunction() {
    // TODO
    return nullptr;
}
std::vector<PARAM_ID> ReqSecInCircle::getParams() const {
    std::vector<PARAM_ID> res;
    res.push_back(&(m_c->center->x));
    res.push_back(&(m_c->center->y));
    res.push_back(&(m_c->r));
    res.push_back(&(m_s->beg->x));
    res.push_back(&(m_s->beg->y));
    res.push_back(&(m_s->end->x));
    res.push_back(&(m_s->end->y));
    return res;
}
Requirement ReqSecInCircle::getType() const {
    return ET_SECTIONINCIRCLE;
}
std::vector<IGeometricObject*> ReqSecInCircle::getObjects() {
    return {m_s, m_c};
}

std::vector<Variable*> ReqSecInCircle::getVariables() {
    return std::vector<Variable*>();
};



// 8
ReqSecSecParallel::ReqSecSecParallel(Section* s1, Section* s2) {
    m_s1 = s1;
    m_s2 = s2;
}
ErrorFunctions* ReqSecSecParallel::getFunction() {
    return new SectionSectionParallelError(getVariables());
}
std::vector<PARAM_ID> ReqSecSecParallel::getParams() const {
    std::vector<PARAM_ID> res;
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
Requirement ReqSecSecParallel::getType() const {
    return ET_SECTIONSECTIONPARALLEL;
}
std::vector<IGeometricObject*> ReqSecSecParallel::getObjects() {
    return {m_s1, m_s2};
}
std::vector<Variable*> ReqSecSecParallel::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_s1->beg->x),
            new Variable(&m_s1->beg->y),
            new Variable(&m_s1->end->x),
            new Variable(&m_s1->end->y),
            new Variable(&m_s2->beg->x),
            new Variable(&m_s2->beg->y),
            new Variable(&m_s2->end->x),
            new Variable(&m_s2->end->y)
    };
    return variables;
};


// 9
ReqSecSecPerpendicular::ReqSecSecPerpendicular(Section* s1, Section* s2) {
    m_s1 = s1;
    m_s2 = s2;
}
ErrorFunctions* ReqSecSecPerpendicular::getFunction() {
    return new SectionSectionPerpendicularError(getVariables());
}
std::vector<PARAM_ID> ReqSecSecPerpendicular::getParams() const {
    std::vector<PARAM_ID> res;
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
Requirement ReqSecSecPerpendicular::getType() const {
    return ET_SECTIONSECTIONPERPENDICULAR;
}
std::vector<IGeometricObject*> ReqSecSecPerpendicular::getObjects() {
    return {m_s1, m_s2};
}
std::vector<Variable*> ReqSecSecPerpendicular::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_s1->beg->x),
            new Variable(&m_s1->beg->y),
            new Variable(&m_s1->end->x),
            new Variable(&m_s1->end->y),
            new Variable(&m_s2->beg->x),
            new Variable(&m_s2->beg->y),
            new Variable(&m_s2->end->x),
            new Variable(&m_s2->end->y)
    };
    return variables;
}


// 10
ReqSecSecAngel::ReqSecSecAngel(Section* s1, Section* s2, double angle) {
    m_s1 = s1;
    m_s2 = s2;
    desired_angle = angle;
}
ErrorFunctions* ReqSecSecAngel::getFunction() {
    return new SectionSectionAngleError(getVariables(), desired_angle);
}
std::vector<PARAM_ID> ReqSecSecAngel::getParams() const {
    std::vector<PARAM_ID> res;
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
Requirement ReqSecSecAngel::getType() const {
    return ET_SECTIONSECTIONANGLE;
}
std::vector<IGeometricObject*> ReqSecSecAngel::getObjects() {
    return {m_s1, m_s2};
}
std::vector<Variable*> ReqSecSecAngel::getVariables() {
    std::vector<Variable*> variables = {
            new Variable(&m_s1->beg->x),
            new Variable(&m_s1->beg->y),
            new Variable(&m_s1->end->x),
            new Variable(&m_s1->end->y),
            new Variable(&m_s2->beg->x),
            new Variable(&m_s2->beg->y),
            new Variable(&m_s2->end->x),
            new Variable(&m_s2->end->y)
    };
    return variables;
}

