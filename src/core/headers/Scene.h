#ifndef OURPAINT_HEADERS_SCENE_H_
#define OURPAINT_HEADERS_SCENE_H_

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "objects/BoundBox.h"
#include "objects/Enums.h"
#include "InheritanceGraph.h"
#include "objects/Objects.h"

class ISceneObserver;
struct Point;
struct Section;
struct Circle;
struct Arc;
class Painter;
class Component;
class Variable;
class Function;
struct ObjectData;
#include "objects/GeometricObjects.h"

#include "DCMManager.h"

#include "objects/ID.h"
using namespace core;

class Scene {
private:
    OurPaintDCM::DCMManager DCM_manager;
    const OurPaintDCM::Figures::GeometryStorage* storage_ = nullptr;

    struct bezier {
        CubicBezier b;
        ID start;
        ID end;
        ID control1;
        ID control2;
    };

    std::map<ID, bezier> beziers_;
    std::map<ID, ID> pointToBezier_;
    int lastBezierId = 0;

private:
    ObjType getObjType(ID id) const;
    bool exists(ID id, ObjType expected) const;
    bool isValid(const Requirement& req) const;
    std::vector<Variable*> getVariables(const Requirement& req) const;
    Function* getFunction(const Requirement& req);

    std::unordered_map<ID, OurPaintDCM::Utils::ID> DCM_ids;

public:
    using DCM_ID = OurPaintDCM::Utils::ID;
    using SCENE_ID = ID;
    using ObjDescriptor = OurPaintDCM::Utils::FigureDescriptor;
    using ReqDescriptor = OurPaintDCM::Utils::RequirementDescriptor;
    using Type = OurPaintDCM::Utils::FigureType;
    using Storage = OurPaintDCM::Figures::GeometryStorage;

    static const ID _errorID;
    static const ID _connectionEdgeID;

public:
    Scene();
    Scene(const Scene&) = delete;
    Scene operator=(const Scene&) = delete;
    // Scene(Scene&&);
    // Scene& operator=(Scene&&);
    ~Scene();

    ID addObject(const ObjectData&);
    bool deleteObject(ID objectID);
    bool deletePoint(ID pointID);
    bool deleteLine(ID sectionID);
    bool deleteCircle(ID circleID);
    bool deleteArc(ID arcID);
    void clear();

    const BoundBox2D& getBoundingBox() const;
    void updateBoundingBox() const;

    void rebuildComponents();
    Component& findComponentByID(ID id);

    ObjectData getObjectData(ID objectID) const;
    ObjectData getRootObjectData(ID objectID) const;
    Requirement getRequirementData(ID object1, ID object2) const;
    std::size_t objectsCount() const;
    std::size_t requirementsCount() const;
    std::vector<ObjectData> getObjects() const;
    std::vector<ObjectData> getPoints() const;
    std::vector<ObjectData> getLines() const;
    std::vector<ObjectData> getCircles() const;
    std::vector<ObjectData> getArcs() const;
    std::vector<ObjectData> getBeziers() const;
    std::vector<Requirement> getRequirements() const;
    std::vector<Requirement> getObjectRequirements(ID objectID) const;
    std::vector<Requirement> getObjectRequirementsWithConnectedObjects(ID objectID) const;

    bool hasObject(ID id) const;
    bool hasRequirement(ID id) const;

    void moveObject(ID objectID, double dx, double dy);
    void moveObjects(std::vector<ID> ids, double dx, double dy);
    void movePoint(ID pointID, double dx, double dy);
    void moveLine(ID lineID, double dx, double dy);
    void moveCircle(ID circleID, double dx, double dy);
    void moveArc(ID circleID, double dx, double dy);

    void setPoint(ID pointID, double x, double y, const bool updateRequirementFlag = true);
    void setSection(ID sectionID, double x1, double y1, double x2, double y2, const bool updateRequirementFlag = true);
    void setCircle(ID pointID, double x, double y, double r, const bool updateRequirementFlag = true);
    void setArc(ID arcID, double x0, double y0, double x1, double y1, double x2, double y2, double r, const bool updateRequirementFlag = true);

    ID addRequirement(const Requirement& reqData, const bool updateRequirementFlag = true);
    void updateRequirements(ID objectID);
    Requirement getRequirementData(ID reqID) const;
    std::vector<Requirement> getAllRequirementsData() const;
    bool deleteRequirement(ID reqID);

    bool tryRestoreObject(const ObjectData&, ID id);
    bool tryRestoreRequirement(const Requirement&, ID id);

    void setObserver(ISceneObserver* o);

    ClipboardData copyFragment(const std::vector<ID>& selected) const;
    std::vector<ID> pasteFragment(const ClipboardData& data, double targetPosX, double targetPosY);

    BoundBox2D makeBoundingBoxForObject(ID id) const;
    BoundBox2D makeBoundingBoxFromObjects(const std::vector<ID>& objects) const;

private:
    ObjectContainer _objectContainer;
    std::vector<ISceneObserver*> _observers;

    void addRequirement(const Requirement& reqData, ID reqID);

    static ReqType reqTypeMapper(OurPaintDCM::Utils::RequirementType type);
    static OurPaintDCM::Utils::RequirementType reqTypeMapper(ReqType type);
};

#endif  // ! OURPAINT_HEADERS_SCENE_H_