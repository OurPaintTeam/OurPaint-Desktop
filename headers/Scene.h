#ifndef OURPAINT_HEADERS_SCENE_H_
#define OURPAINT_HEADERS_SCENE_H_

#include "Painter.h"
#include "GeometricObjects.h"
#include "Objects.h"
#include "ID.h"
#include "Requirements.h"
#include "BoundBox.h"
#include "InheritanceGraph.h"
#include "LMForTest.h"
#include "FileOurP.h"
#include "objectInFile.h"
#include "Component.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
class Scene {
private:
    std::unordered_map<ID, Point*> _points;
    std::unordered_map<ID, Section*> _sections;
    std::unordered_map<ID, Circle*> _circles;
    std::unordered_map<ID, Arc*> _arcs;

    std::unordered_map<ID, IReq*> _requirements;

    std::vector<Component> _components;
    std::unordered_map<ID, std::size_t> _idToComponent;
    bool _isComponentsDirty;

    mutable Painter* _painter;

    mutable bool _isRectangleDirty;
    mutable BoundBox2D _allFiguresRectangle;

    IDGenerator _idGeometricObjectsGenerator;
    IDGenerator _idRequirementsGenerator;

    std::unordered_set<ID> _idDeletedGeometricObject;

    UndirectedWeightedGraph<ID, ID> _graph;

public:
    static const ID _errorID;
    static const ID _autoID;
    static const ID _connectionEdgeID;

public:
    Scene(Painter* p);
    ~Scene();

    ID addObject(const ObjectData&, ID id = Scene::_autoID);
    bool deleteObject(ID objectID);
    bool deletePoint(ID pointID);
    bool deleteSection(ID sectionID);
    bool deleteCircle(ID circleID);
    bool deleteArc(ID arcID);
    void clear();

    const BoundBox2D& getBoundingBox() const;
    void updateBoundingBox() const;

    void rebuildComponents();
    const Component& findComponentByID(ID id);

    void paint() const;

    ObjectData getObjectData(ID objectID) const;

    // TODO getAllObjects
    // TODO getPoints
    // TODO getSections
    // TODO getCircles

    std::size_t edgeCount() const;
    std::size_t vertexCount() const;

    void setPainter(Painter*);

    void moveObject(ID objectID, double dx, double dy);
    void movePoint(ID pointID, double dx, double dy);
    void moveSection(ID sectionID, double dx, double dy);
    void moveCircle(ID circleID, double dx, double dy);
    void moveArc(ID circleID, double dx, double dy);

    void setPoint(ID pointID, double x, double y);
    void setSection(ID sectionID, double x1, double y1, double x2, double y2);
    void setCircle(ID pointID, double x, double y, double r);
    // TODO setArc

    ID addRequirement(const RequirementData& reqData, const bool updateRequirementFlag = true);
    void updateRequirements(ID objectID);
    // TODO deleteRequirement
    // TODO getRequirement
    // TODO getAllRequirements

    std::string to_string() const;

    void saveToFile(const char* filename) const;
    void loadFromFile(const char* filename);
};

#endif // ! OURPAINT_HEADERS_SCENE_H_