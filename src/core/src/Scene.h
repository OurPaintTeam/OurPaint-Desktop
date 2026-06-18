#ifndef OURPAINT_CORE_SCENE_H_
#define OURPAINT_CORE_SCENE_H_

#include <vector>
#include "objects/ID.h"
#include "objects/GeometricObjects.h"
#include "DCMManager.h"
#include "objects/Enums.h"
class ISceneObserver;

namespace core {
class Requirement;
class ClipboardData;
class ObjectData;

using namespace OurPaintDCM;

class Scene {
private:
    DCMManager DCM_;

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

public:
    using DCM_ID = Utils::ID;
    using DCM_FigDesc = Utils::FigureDescriptor;
    using DCM_ReqDesc = Utils::RequirementDescriptor;
    using DCM_FigType = Utils::FigureType;
    using DCM_ReqType = Utils::RequirementType;
    using DCM_PointUpdateDesc = Utils::PointUpdateDescriptor;

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

    ObjectData getObjectData(ID objectID) const;
    ObjectData getRootObjectData(ID objectID) const;
    Requirement getRequirementData(ID object1, ID object2) const;
    std::size_t objectsCount() const;
    std::size_t requirementsCount() const;
    std::vector<ObjectData> getObjects() const;

    std::vector<ObjectData> getPoints() const;
    void appendPickedPointsInRect(double rx1, double ry1, double rx2, double ry2, std::vector<ID>& out) const;

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

    void resizeCircle(ID circleID, double radius);

    void setPoint(ID pointID, double x, double y, const bool updateRequirementFlag = true);
    void setSection(ID sectionID, double x1, double y1, double x2, double y2, const bool updateRequirementFlag = true);
    void setCircle(ID pointID, double x, double y, double r, const bool updateRequirementFlag = true);
    void setArc(ID arcID, double x0, double y0, double x1, double y1, double x2, double y2, double r, const bool updateRequirementFlag = true);

    ID addRequirement(const Requirement& reqData, const bool updateRequirementFlag = true);
    void updateRequirements(ID objectID);
    Requirement getRequirementData(ID reqID) const;
    std::vector<Requirement> getAllRequirementsData() const;
    bool deleteRequirement(ID reqID);

    void setObserver(ISceneObserver* o);

    ClipboardData copyFragment(const std::vector<ID>& selected) const;
    std::vector<ID> pasteFragment(const ClipboardData& data, double targetPosX, double targetPosY);

    BoundBox2D makeBoundingBoxForObject(ID id) const;
    BoundBox2D makeBoundingBoxFromObjects(const std::vector<ID>& objects) const;

    bool pointIsFixed(ID pointID) const;

    struct PointGroup {
        std::vector<ObjectData> points;
    };

    std::vector<PointGroup> getPointOnPointGroups() const;

    bool restoreObject(const ObjectData& object);
    bool restoreRequirement(const Requirement& requirement);
    bool setObjectData(const ObjectData& object);

private:
    std::vector<ISceneObserver*> _observers;

    void addRequirement(const Requirement& reqData, ID reqID);

    static ReqType reqTypeMapper(DCM_ReqType type);
    static DCM_ReqType reqTypeMapper(ReqType type);
};

}

#endif  // ! OURPAINT_CORE_SCENE_H_