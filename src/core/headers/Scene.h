#ifndef OURPAINT_HEADERS_SCENE_H_
#define OURPAINT_HEADERS_SCENE_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "BoundBox.h"
#include "Enums.h"
#include "ID.h"
#include "InheritanceGraph.h"
#include "Objects.h"

class ISceneObserver;
namespace SceneObjects {class ID;}
struct Point;
struct Section;
struct Circle;
struct Arc;
class Painter;
class Component;
class Variable;
class Function;
struct ObjectData;

#include "DCMManager.h"

class Scene {
private:
    OurPaintDCM::DCMManager DCM_manager;
    const OurPaintDCM::Figures::GeometryStorage* storage_ = nullptr;

private:
    mutable bool _isRectangleDirty;
    mutable BoundBox2D _allFiguresRectangle;


private:
    ObjType getObjType(SceneObjects::ID id) const;
    bool exists(SceneObjects::ID id, ObjType expected) const;
    bool isValid(const Requirement& req) const;
    std::vector<Variable*> getVariables(const Requirement& req) const;
    Function* getFunction(const Requirement& req);

    std::unordered_map<SceneObjects::ID, OurPaintDCM::Utils::ID> DCM_ids;

public:
    using ID = OurPaintDCM::Utils::ID;
    using ObjDescriptor = OurPaintDCM::Utils::FigureDescriptor;
    using ReqDescriptor = OurPaintDCM::Utils::RequirementDescriptor;
    using Type = OurPaintDCM::Utils::FigureType;
    using Storage = OurPaintDCM::Figures::GeometryStorage;

    static const SceneObjects::ID _errorID;
    static const SceneObjects::ID _connectionEdgeID;

public:
    Scene();
    Scene(const Scene&) = delete;
    Scene operator=(const Scene&) = delete;
    // Scene(Scene&&);
    // Scene& operator=(Scene&&);
    ~Scene();

    SceneObjects::ID addObject(const ObjectData&);
    bool deleteObject(SceneObjects::ID objectID);
    bool deletePoint(SceneObjects::ID pointID);
    bool deleteSection(SceneObjects::ID sectionID);
    bool deleteCircle(SceneObjects::ID circleID);
    bool deleteArc(SceneObjects::ID arcID);
    void clear();

    const BoundBox2D& getBoundingBox() const;
    void updateBoundingBox() const;

    void rebuildComponents();
    Component& findComponentByID(SceneObjects::ID id);

    ObjectData getObjectData(SceneObjects::ID objectID) const;
    ObjectData getRootObjectData(SceneObjects::ID objectID) const;
    Requirement getRequirementData(SceneObjects::ID object1, SceneObjects::ID object2) const;
    std::size_t objectsCount() const;
    std::size_t requirementsCount() const;
    std::vector<ObjectData> getObjects() const;
    std::vector<ObjectData> getPoints() const;
    std::vector<ObjectData> getLines() const;
    std::vector<ObjectData> getCircles() const;
    std::vector<ObjectData> getArcs() const;
    std::vector<Requirement> getRequirements() const;
    std::vector<Requirement> getObjectRequirements(SceneObjects::ID objectID) const;
    std::vector<Requirement> getObjectRequirementsWithConnectedObjects(SceneObjects::ID objectID) const;

    bool hasObject(SceneObjects::ID id) const;
    bool hasRequirement(SceneObjects::ID id) const;

    void moveObject(SceneObjects::ID objectID, double dx, double dy);
    void movePoint(SceneObjects::ID pointID, double dx, double dy);
    void moveLine(SceneObjects::ID lineID, double dx, double dy);
    void moveCircle(SceneObjects::ID circleID, double dx, double dy);
    void moveArc(SceneObjects::ID circleID, double dx, double dy);

    void setPoint(SceneObjects::ID pointID, double x, double y, const bool updateRequirementFlag = true);
    void setSection(SceneObjects::ID sectionID, double x1, double y1, double x2, double y2, const bool updateRequirementFlag = true);
    void setCircle(SceneObjects::ID pointID, double x, double y, double r, const bool updateRequirementFlag = true);
    void setArc(SceneObjects::ID arcID, double x0, double y0, double x1, double y1, double x2, double y2, double r, const bool updateRequirementFlag = true);

    std::vector<const double*> getPointParams(SceneObjects::ID pointID) const;
    std::vector<const double*> getSectionParams(SceneObjects::ID sectionID) const;
    std::vector<const double*> getCircleParams(SceneObjects::ID circleID) const;
    std::vector<const double*> getArcParams(SceneObjects::ID arcID) const;

    SceneObjects::ID addRequirement(const Requirement& reqData, const bool updateRequirementFlag = true);
    void updateRequirements(SceneObjects::ID objectID);
    Requirement getRequirementData(SceneObjects::ID reqID) const;
    std::vector<Requirement> getAllRequirementsData() const;
    bool deleteRequirement(SceneObjects::ID reqID);

    bool tryRestoreObject(const ObjectData&, SceneObjects::ID id);
    bool tryRestoreRequirement(const Requirement&, SceneObjects::ID id);

    void setObserver(ISceneObserver* o);

    void load(const std::vector<ObjectData>&, const std::vector<Requirement>&);

    ObjectContainer& getObjectContainer();

private:
    ObjectContainer _objectContainer;
    std::vector<ISceneObserver*> _observers;

    void addRequirement(const Requirement& reqData, SceneObjects::ID reqID);
};

#endif  // ! OURPAINT_HEADERS_SCENE_H_