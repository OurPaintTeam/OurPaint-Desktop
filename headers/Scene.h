#ifndef OURPAINT_HEADERS_SCENE_H_
#define OURPAINT_HEADERS_SCENE_H_

#include "Painter.h"
#include "objects/GeometricObjects.h"
#include "objects/Objects.h"
#include "objects/ID.h"
#include "Requirements.h"
#include "objects/BoundBox.h"
#include "InheritanceGraph.h"
#include "LMForTest.h"
#include "FileOurP.h"

#include <vector>
#include <unordered_map>

class Scene {
private:
    std::unordered_map<ID, Point*> _points;
    std::unordered_map<ID, Section*> _sections;
    std::unordered_map<ID, Circle*> _circles;

    std::unordered_map<ID, IReq*> _requirements;

    std::vector<Function*> _errorRequirementFunctions;

    mutable Painter* _painter;

    mutable bool _isRectangleDirty;
    mutable BoundBox2D _allFiguresRectangle;

    IDGenerator _idGeЫometricObjectsGenerator;
    IDGenerator _idRequirementsGenerator;

    UndirectedWeightedGraph<ID, ID> _graph;

public:
    Scene(Painter* p);
    ~Scene();

    ID addObject(const ObjectData& objData);
    bool deleteObject(ID objectID);

    void clear();

    const BoundBox2D& getBoundingBox() const;
    void updateBoundingBox() const;

    void paint() const;

    const IGeometricObject* getObject(ID objectID) const;
    std::unordered_map<ID, const IGeometricObject*> getAllObjects() const;
    std::unordered_map<ID, const Point*> getPoints() const;
    std::unordered_map<ID, const Section*> getSections() const;
    std::unordered_map<ID, const Circle*> getCircles() const;

    std::size_t edgeCount() const;
    std::size_t vertexCount() const;

    void setPainter(Painter*) const;

    void moveObject(ID objectID, double dx, double dy);
    void movePoint(ID pointID, double dx, double dy);
    void moveSection(ID sectionID, double dx, double dy);
    void moveCircle(ID circleID, double dx, double dy);

    void setPoint(ID pointID, double x, double y);
    void setSection(ID sectionID, double x1, double y1, double x2, double y2);
    void setCircle(ID pointID, double x, double y, double r);

    ID addRequirement(const RequirementData& reqData);
    void updateRequirements(ID objectID);

    std::string to_string() const;

    // TODO ---------------------------------------------------------------

    //IReq getRequirementInfo(ID id) const;
    //std::vector<std::pair<ID, IReq>> getAllRequirementsInfo() const;
    //void deleteRequirement(ID reqID);
};

#endif // ! OURPAINT_HEADERS_SCENE_H_