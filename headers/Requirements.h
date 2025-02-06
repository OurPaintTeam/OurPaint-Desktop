#ifndef OURPAINT_HEADERS_REQUIREMENTS_H_
#define OURPAINT_HEADERS_REQUIREMENTS_H_

#define PARAMID double*

#include <cmath>
#include <vector>

#include "Enums.h"
#include "ErrorFunctions.h"
#include "Objects.h"

class VarsStorage{
    static std::map<PARAMID, Variable*> m_vars;
public:
    static std::vector<Variable*> getVars();
    static Variable* addVar(PARAMID id);
    static Variable* getVar(PARAMID id);
    static void clearVars();
};

struct RequirementData {
    Requirement req;
    std::vector<ID> objects;
    std::vector<double> params;
    RequirementData();

    bool operator==(const RequirementData& other) const {
    return req == other.req
        && objects == other.objects
        && params == other.params;
    }

    bool operator!=(const RequirementData& other) const {
    return !(*this == other);
    }
};

// Abstract class
struct IReq {
protected:
    ErrorFunctions* c_f;
    std::vector<ID> objects;
    Requirement req;
public:
    virtual ErrorFunctions* getFunction() = 0;
    virtual std::vector<PARAMID> getParams() = 0;
    virtual rectangle getRectangle() = 0;
};



// 1
class ReqPointSecDist : public IReq {
    Point* m_p;
    Section* m_s;
    double d;
public:
    ReqPointSecDist(Point* p, Section* s, double dist);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 2
class ReqPointOnSec : public IReq {
    Point* m_p;
    Section* m_s;
public:
    ReqPointOnSec(Point* p, Section* s);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 3
class ReqPointPointDist : public IReq {
    Point* m_p1;
    Point* m_p2;
    double v_dist;
public:
    ReqPointPointDist(Point* p1, Point* p2, double dist);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 4
class ReqPointOnPoint : public IReq {
    Point* m_p1;
    Point* m_p2;
public:
    ReqPointOnPoint(Point* p1, Point* p2);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 5
class ReqSecCircleDist : public IReq {
    Section* m_s;
    Circle* m_c;
    double v_dist;
public:
    ReqSecCircleDist(Section* m_s, Circle* m_c, double dist);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 6
class ReqSecOnCircle : public IReq {
    Section* m_s;
    Circle* m_c;
public:
    ReqSecOnCircle(Section* m_s, Circle* m_c);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 7
class ReqSecInCircle : public IReq {
    Section* m_s;
    Circle* m_c;
public:
    ReqSecInCircle(Section* m_s, Circle* m_c);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 8
class ReqSecSecParallel : public IReq {
    Section* m_s1;
    Section* m_s2;
public:
    ReqSecSecParallel(Section* m_s1, Section* m_s2);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 9
class ReqSecSecPerpendicular : public IReq {
    Section* m_s1;
    Section* m_s2;
public:
    ReqSecSecPerpendicular(Section* m_s1, Section* m_s2);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 10
class ReqSecSecAngel : public IReq {
    Section* m_s1;
    Section* m_s2;
    double desired_angle;
public:
    ReqSecSecAngel(Section* m_s1, Section* m_s2, double desired_dist);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};


#endif // ! OURPAINT_HEADERS_REQUIREMENTS_H_
