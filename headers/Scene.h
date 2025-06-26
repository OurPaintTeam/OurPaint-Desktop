#ifndef OURPAINT_HEADERS_SCENE_H_
#define OURPAINT_HEADERS_SCENE_H_

#include "Painter.h"
#include "Objects.h"
#include "InheritanceGraph.h"
#include "LMWithSparse.h"
#include "FileOurP.h"
#include "objectInFile.h"
#include "Component.h"
#include "Enums.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

class BoundBox2D;
class ID;
struct Point;
struct Section;
struct Circle;
struct Arc;

class Scene {
private:
    std::unordered_map<ID, Point*> _points;
    std::unordered_map<ID, Section*> _sections;
    std::unordered_map<ID, Circle*> _circles;
    std::unordered_map<ID, Arc*> _arcs;

    mutable Painter* _painter;

    mutable bool _isRectangleDirty;
    mutable BoundBox2D _allFiguresRectangle;

    IDGenerator _idGeometricObjectsGenerator;

    std::unordered_set<ID> _idDeletedGeometricObject;
    std::unordered_set<ID> _idDeletedRequirements;

    UndirectedWeightedGraph<ID, ID> _graph;

private:
    std::unordered_map<ID, Requirement> _requirements;

    std::unordered_map<ID, std::unordered_set<ID>> _requirementToObjects;
    std::unordered_map<ID, std::vector<ID>> _objectSubObjects;

    std::vector<Component> _components;
    std::unordered_map<ID, std::size_t> _idToComponent;
    bool _isComponentsDirty;

    //LMSparse _solver;

    IDGenerator _idRequirementsGenerator;

    ObjType getObjType(ID id) const;
    bool exists(ID id, ObjType expected) const;
    bool isValid(const Requirement& req) const;
    std::vector<Variable*> getVariables(const Requirement& req) const;
    Function* getFunction(const Requirement& req);

public:
    static const ID _errorID;
    static const ID _connectionEdgeID;

public:
    Scene(Painter* p);
    Scene(const Scene&) = delete;
    Scene operator=(const Scene&) = delete;
    //Scene(Scene&&);
    //Scene& operator=(Scene&&);
    ~Scene();

    ID addObject(const ObjectData&);
    bool deleteObject(ID objectID);
    bool deletePoint(ID pointID);
    bool deleteSection(ID sectionID);
    bool deleteCircle(ID circleID);
    bool deleteArc(ID arcID);
    void clear();

    const BoundBox2D& getBoundingBox() const;
    void updateBoundingBox() const;

    void rebuildComponents();
    Component& findComponentByID(ID id);

    void paint() const;
    void clearImage() const;

    ObjectData getObjectData(ID objectID) const;
    ObjectData getRootObjectData(ID objectID) const;
    Requirement getRequirementData(ID object1, ID object2) const;
    std::size_t objectsCount() const;
    std::size_t requirementsCount() const;
    std::vector<ObjectData> getObjects() const;
    std::vector<Requirement> getRequirements() const;
    std::vector<Requirement> getObjectRequirements(ID objectID) const;
    std::vector<Requirement> getObjectRequirementsWithConnectedObjects(ID objectID) const;

    bool hasObject(ID id) const;
    bool hasRequirement(ID id) const;

    void setPainter(Painter*);

    void moveObject(ID objectID, double dx, double dy);
    void movePoint(ID pointID, double dx, double dy);
    void moveSection(ID sectionID, double dx, double dy);
    void moveCircle(ID circleID, double dx, double dy);
    void moveArc(ID circleID, double dx, double dy);

    void setPoint(ID pointID, double x, double y, const bool updateRequirementFlag = true);
    void setSection(ID sectionID, double x1, double y1, double x2, double y2, const bool updateRequirementFlag = true);
    void setCircle(ID pointID, double x, double y, double r, const bool updateRequirementFlag = true);
    void setArc(ID arcID, double x0, double y0, double x1, double y1, double x2, double y2, double r, const bool updateRequirementFlag = true);

    std::vector<const double*> getPointParams(ID pointID) const;
    std::vector<const double*> getSectionParams(ID sectionID) const;
    std::vector<const double*> getCircleParams(ID circleID) const;
    std::vector<const double*> getArcParams(ID arcID) const;

    ID addRequirement(const Requirement& reqData, const bool updateRequirementFlag = true);
    void updateRequirements(ID objectID);
    Requirement getRequirementData(ID reqID) const;
    std::vector<Requirement> getAllRequirementsData() const;
    bool deleteRequirement(ID reqID);

    std::string to_string() const;
    void saveToFile(const char* filename) const;
    void loadFromFile(const char* filename);

    bool tryRestoreObject(const ObjectData&, ID id);
    bool tryRestoreRequirement(const Requirement&, ID id);

private:
    void addPoint(ObjectData data, ID id);
    void addSection(ObjectData data, ID pointID1, ID pointID2, ID sectionID);
    void addCircle(ObjectData data, ID pointID, ID circleID);
    void addArc(ObjectData data, ID pointID1, ID pointID2, ID pointID3, ID arcID);

    void addRequirement(const Requirement& reqData, ID reqID);
};

#endif // ! OURPAINT_HEADERS_SCENE_H_