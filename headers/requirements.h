//
// Created by Ardrass on 010, 10.07.2024.
//

#ifndef OURPAINT_REQUIREMENTS_H
#define OURPAINT_REQUIREMENTS_H

#define PARAMID double*
#include <vector>
#include "objects.h"
#include <cmath>
#include "enums.h"
#include "ErrorFunctions.h"

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
    point* m_p;
    section* m_s;
    double d;
public:
    ReqPointSecDist(point* p, section* s, double dist);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 2
class ReqPointOnSec : public IReq {
    point* m_p;
    section* m_s;
public:
    ReqPointOnSec(point* p, section* s);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 3
class ReqPointPointDist : public IReq {
    point* m_p1;
    point* m_p2;
    double v_dist;
public:
    ReqPointPointDist(point* p1, point* p2, double dist);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 4
class ReqPointOnPoint : public IReq {
    point* m_p1;
    point* m_p2;
public:
    ReqPointOnPoint(point* p1, point* p2);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 5
class ReqSecCircleDist : public IReq {
    section* m_s;
    circle* m_c;
    double v_dist;
public:
    ReqSecCircleDist(section* m_s, circle* m_c, double dist);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 6
class ReqSecOnCircle : public IReq {
    section* m_s;
    circle* m_c;
public:
    ReqSecOnCircle(section* m_s, circle* m_c);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 7
class ReqSecInCircle : public IReq {
    section* m_s;
    circle* m_c;
public:
    ReqSecInCircle(section* m_s, circle* m_c);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 8
class ReqSecSecParallel : public IReq {
    section* m_s1;
    section* m_s2;
public:
    ReqSecSecParallel(section* m_s1, section* m_s2);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 9
class ReqSecSecPerpendicular : public IReq {
    section* m_s1;
    section* m_s2;
public:
    ReqSecSecPerpendicular(section* m_s1, section* m_s2);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};

// 10
class ReqSecSecAngel : public IReq {
    section* m_s1;
    section* m_s2;
    double desired_angle;
public:
    ReqSecSecAngel(section* m_s1, section* m_s2, double desired_dist);
    ErrorFunctions* getFunction() override;
    std::vector<PARAMID> getParams() override;
    rectangle getRectangle() override;
};


#endif //OURPAINT_REQUIREMENTS_H
