#ifndef OURPAINT_HEADERS_REQUIREMENTS_H_
#define OURPAINT_HEADERS_REQUIREMENTS_H_

#include <cmath>
#include <vector>

#include "Enums.h"
#include "ErrorFunction.h"
#include "GeometricObjects.h"
#include "BoundBox.h"
#include "Objects.h"


struct IReq {
public:
    virtual ~IReq() = default;

    virtual ErrorFunction* getFunction() = 0;
    virtual std::vector<PARAM_ID> getParams() = 0;
    virtual Requirement getType() const = 0;
    virtual std::vector<IGeometricObject*> getObjects() = 0;
    virtual std::vector<Variable*> getVariables() = 0;
    virtual double getDimension() const = 0;
};



// 1
class ReqPointSecDist : public IReq {
    Point* m_p;
    Section* m_s;
    double d;
public:
    ReqPointSecDist(Point* p, Section* s, double dist);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 2
class ReqPointOnSec : public IReq {
    Point* m_p;
    Section* m_s;
public:
    ReqPointOnSec(Point* p, Section* s);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 3
class ReqPointPointDist : public IReq {
    Point* m_p1;
    Point* m_p2;
    double v_dist;
public:
    ReqPointPointDist(Point* p1, Point* p2, double dist);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 4
class ReqPointOnPoint : public IReq {
    Point* m_p1;
    Point* m_p2;
public:
    ReqPointOnPoint(Point* p1, Point* p2);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 5
class ReqSecCircleDist : public IReq {
    Section* m_s;
    Circle* m_c;
    double v_dist;
public:
    ReqSecCircleDist(Section* m_s, Circle* m_c, double dist);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 6
class ReqSecOnCircle : public IReq {
    Section* m_s;
    Circle* m_c;
public:
    ReqSecOnCircle(Section* m_s, Circle* m_c);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 7
class ReqSecInCircle : public IReq {
    Section* m_s;
    Circle* m_c;
public:
    ReqSecInCircle(Section* m_s, Circle* m_c);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 8
class ReqSecSecParallel : public IReq {
    Section* m_s1;
    Section* m_s2;
public:
    ReqSecSecParallel(Section* m_s1, Section* m_s2);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 9
class ReqSecSecPerpendicular : public IReq {
    Section* m_s1;
    Section* m_s2;
public:
    ReqSecSecPerpendicular(Section* m_s1, Section* m_s2);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

// 10
class ReqSecSecAngel : public IReq {
    Section* m_s1;
    Section* m_s2;
    double desired_angle;
public:
    ReqSecSecAngel(Section* m_s1, Section* m_s2, double desired_dist);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;
};

class ReqArcCenterOnPerpendicular : public IReq {
public:
    ReqArcCenterOnPerpendicular(Point* p1, Point* p2, Point* p3);
    ErrorFunction* getFunction() override;
    std::vector<PARAM_ID> getParams() override;
    Requirement getType() const override;
    std::vector<IGeometricObject*> getObjects() override;
    std::vector<Variable*> getVariables() override;
    double getDimension() const override;

private:
    Point* m_p1;
    Point* m_p2;
    Point* m_p3;
};

#endif // ! OURPAINT_HEADERS_REQUIREMENTS_H_
