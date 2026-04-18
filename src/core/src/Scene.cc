#include "Scene.h"

#include "objects/Component.h"
#include "objects/Enums.h"
#include "objects/GeometricObjects.h"
#include "LMWithSparse.h"
#include "objects/Objects.h"
#include "ISceneObserver.h"

#include "objects/Scene_ID.h"

const SceneObjects::ID Scene::_errorID(-1);
const SceneObjects::ID Scene::_connectionEdgeID(-2);

Scene::Scene() :
    _isRectangleDirty(false),
    _objectContainer() {

    DCM_manager.setSolveMode(OurPaintDCM::Utils::SolveMode::DRAG);
}

Scene::~Scene() {}

SceneObjects::ID Scene::addObject(const ObjectData& objData) {
    switch (objData.et) {
        case ObjType::ET_POINT: {
            if (objData.params.size() < 2) {
                throw std::invalid_argument("Point requires 2 coordinates");
            }

            double x = objData.params[0];
            double y = objData.params[1];

            ObjDescriptor od = OurPaintDCM::Utils::FigureDescriptor::point(x, y);
            OurPaintDCM::Utils::ID id = DCM_manager.addFigure(od);

            for (auto& observer : _observers) {
                observer->onObjectAdded(od);
            }

            return SceneObjects::ID();
        }
        case ObjType::ET_LINE: {
            if (objData.params.size() < 4) {
                throw std::invalid_argument("Section requires 4 coordinates");
            }

            double x1 = objData.params[0];
            double y1 = objData.params[1];
            double x2 = objData.params[2];
            double y2 = objData.params[3];

            ObjDescriptor l = OurPaintDCM::Utils::FigureDescriptor::line(x1, y1, x2, y2);

            OurPaintDCM::Utils::ID id = DCM_manager.addFigure(l);

            for (auto& observer : _observers) {
                observer->onObjectAdded(l);
            }

            return SceneObjects::ID();
        }
        case ObjType::ET_CIRCLE: {
            if (objData.params.size() < 3) {
                throw std::invalid_argument("Circle requires center and radius");
            }

            double cx = objData.params[0];
            double cy = objData.params[1];
            double r = objData.params[2];

            ObjDescriptor c = OurPaintDCM::Utils::FigureDescriptor::circle(cx, cy, r);
            OurPaintDCM::Utils::ID id = DCM_manager.addFigure(c);

            for (auto& observer : _observers) {
                observer->onObjectAdded(c);
            }

            return SceneObjects::ID();
        }
        case ObjType::ET_ARC: {
            if (objData.params.size() < 5) {
                throw std::invalid_argument("Arc requires first point, second point, center and radius");
            }

            double beg_x = objData.params[0];
            double beg_y = objData.params[1];
            double end_x = objData.params[2];
            double end_y = objData.params[3];
            double cx = objData.params[4];
            double cy = objData.params[5];

            OurPaintDCM::Utils::ID id_p1 = DCM_manager.addFigure(OurPaintDCM::Utils::FigureDescriptor::point(beg_x, beg_y));
            OurPaintDCM::Utils::ID id_p2 = DCM_manager.addFigure(OurPaintDCM::Utils::FigureDescriptor::point(end_x, end_y));
            OurPaintDCM::Utils::ID id_p3 = DCM_manager.addFigure(OurPaintDCM::Utils::FigureDescriptor::point(cx, cy));

            OurPaintDCM::Utils::ID id = DCM_manager.addFigure(OurPaintDCM::Utils::FigureDescriptor::arc(id_p1, id_p2, id_p3));

            for (auto& observer : _observers) {
                //observer->onObjectUpserted(objData);
            }

            return SceneObjects::ID();
        }
        case ObjType::ET_CUBIC_BEZIER: {
            if (objData.params.size() < 4) {
                throw std::invalid_argument("Cubic bezier requires 4 points");
            }

            double x1 = objData.params[0];
            double y1 = objData.params[1];
            double x2 = objData.params[2];
            double y2 = objData.params[3];

            double x3 = x1 + (x2 - x1) / 3.0;
            double y3 = y1 + (y2 - y1) / 3.0;

            double x4 = x1 + 2.0 * (x2 - x1) / 3.0;
            double y4 = y1 + 2.0 * (y2 - y1) / 3.0;


            ObjDescriptor od1 = OurPaintDCM::Utils::FigureDescriptor::point(x1, y1);
            ObjDescriptor od2 = OurPaintDCM::Utils::FigureDescriptor::point(x2, y2);
            ObjDescriptor od3 = OurPaintDCM::Utils::FigureDescriptor::point(x3, y3);
            ObjDescriptor od4 = OurPaintDCM::Utils::FigureDescriptor::point(x4, y4);

            OurPaintDCM::Utils::ID id1 = DCM_manager.addFigure(od1);
            OurPaintDCM::Utils::ID id3 = DCM_manager.addFigure(od3);
            OurPaintDCM::Utils::ID id4 = DCM_manager.addFigure(od4);
            OurPaintDCM::Utils::ID id2 = DCM_manager.addFigure(od2);

            ++lastBezierId;

            pointToBezier_[SceneObjects::ID(id1.id)] = SceneObjects::ID(lastBezierId);
            pointToBezier_[SceneObjects::ID(id2.id)] = SceneObjects::ID(lastBezierId);
            pointToBezier_[SceneObjects::ID(id3.id)] = SceneObjects::ID(lastBezierId);
            pointToBezier_[SceneObjects::ID(id4.id)] = SceneObjects::ID(lastBezierId);

            CubicBezier b({
                Point(x1, y1),
                Point(x2, y2),
                Point(x3, y3),
                Point(x4, y4)
            });

            beziers_[SceneObjects::ID(lastBezierId)] = {
                b,
                SceneObjects::ID(id1.id),
                SceneObjects::ID(id2.id),
                SceneObjects::ID(id3.id),
                SceneObjects::ID(id4.id)
            };

            for (auto& observer : _observers) {
                //observer->onObjectAdded(l);
            }

            return SceneObjects::ID();
        }
        default:
            throw std::invalid_argument("Unknown object type");
    }
}

bool Scene::deleteObject(SceneObjects::ID objectID) {
    OurPaintDCM::Utils::ID id(objectID.get());
    try {
        DCM_manager.removeFigure(id, true);
        if (pointToBezier_.contains(SceneObjects::ID(id.id))) {
            SceneObjects::ID& b_id = pointToBezier_[SceneObjects::ID(id.id)];
            beziers_.erase(b_id);
        }
    }
    catch (...) {
        return false;
    }
    for (const auto& observer : _observers) {
        observer->onObjectRemoved(id);
    }
    return true;
}

bool Scene::deletePoint(SceneObjects::ID pointID) {
    return deleteObject(pointID);
}

bool Scene::deleteLine(SceneObjects::ID lineID) {
    return deleteObject(lineID);
}

bool Scene::deleteCircle(SceneObjects::ID circleID) {
    return deleteObject(circleID);
}

bool Scene::deleteArc(SceneObjects::ID arcID) {
    return deleteObject(arcID);
}

void Scene::clear() {
    throw std::runtime_error("Scene error");
}

const BoundBox2D& Scene::getBoundingBox() const {
    throw std::runtime_error("Scene error");
}

void Scene::updateBoundingBox() const {
    throw std::runtime_error("Scene error");
}

ObjectData Scene::getObjectData(SceneObjects::ID id) const {
    throw std::runtime_error("Scene error");
}

bool Scene::hasObject(SceneObjects::ID id) const {
    return DCM_manager.hasFigure(OurPaintDCM::Utils::ID(id.get()));
}

bool Scene::hasRequirement(SceneObjects::ID id) const {
    return DCM_manager.hasRequirement(OurPaintDCM::Utils::ID(id.get()));
}

ObjectData Scene::getRootObjectData(SceneObjects::ID id) const {
    throw std::runtime_error("Scene error");
}

Requirement Scene::getRequirementData(SceneObjects::ID object1, SceneObjects::ID object2) const {
    throw std::runtime_error("Scene error");
}

std::size_t Scene::objectsCount() const {
    return DCM_manager.figureCount();
}

std::size_t Scene::requirementsCount() const {
    throw std::runtime_error("Scene error");
}

std::vector<ObjectData> Scene::getObjects() const {
    std::vector<OurPaintDCM::Utils::FigureDescriptor> figures = DCM_manager.getAllFigures();
    std::vector<ObjectData> objects;
    objects.reserve(figures.size());
    for (auto& f : figures) {
        ObjectData od;
        switch (f.type) {
            case OurPaintDCM::Utils::FigureType::ET_POINT2D:
                od.et = ObjType::ET_POINT;
                od.params.push_back(f.coords[0]);
                od.params.push_back(f.coords[1]);
                break;
            case OurPaintDCM::Utils::FigureType::ET_LINE:
                od.et = ObjType::ET_LINE;
                od.params.push_back(f.coords[0]);
                od.params.push_back(f.coords[1]);
                od.params.push_back(f.coords[2]);
                od.params.push_back(f.coords[3]);
                break;
            case OurPaintDCM::Utils::FigureType::ET_CIRCLE:
                od.params.push_back(f.coords[0]);
                od.params.push_back(f.coords[1]);
                od.params.push_back(f.radius.value());
                od.et = ObjType::ET_CIRCLE;
                break;
            case OurPaintDCM::Utils::FigureType::ET_ARC:
                od.params.push_back(f.coords[0]);
                od.params.push_back(f.coords[1]);
                od.params.push_back(f.coords[2]);
                od.params.push_back(f.coords[3]);
                od.params.push_back(f.coords[4]);
                od.params.push_back(f.coords[5]);
                od.et = ObjType::ET_ARC;
                break;
        }
        od.id = SceneObjects::ID(f.id.value().id);
        objects.push_back(od);
    }
    return objects;
}
std::vector<ObjectData> Scene::getPoints() const {
    std::vector<ObjectData> objs;

    std::vector<OurPaintDCM::Utils::FigureDescriptor> points = DCM_manager.getAllPoints();
    objs.reserve(points.size());
    for (auto& d : points) {
        ObjectData od;
        od.et = ObjType::ET_POINT;
        od.id = SceneObjects::ID(d.id.value().id);
        od.params = {d.x.value(), d.y.value()};
        objs.push_back(od);
    }

    return objs;
}

std::vector<ObjectData> Scene::getLines() const {
    std::vector<ObjectData> objs;

    std::vector<OurPaintDCM::Utils::FigureDescriptor> desc = DCM_manager.getAllLines();
    for (auto& f : desc) {
        ObjectData od;
        od.et = ObjType::ET_LINE;
        od.id = SceneObjects::ID(f.id.value().id);
        od.params = {f.coords[0], f.coords[1], f.coords[2], f.coords[3]};
        od.subObjects = {SceneObjects::ID(f.pointIds[0].id), SceneObjects::ID(f.pointIds[1].id) };
        objs.push_back(od);
    }

    return objs;
}

std::vector<ObjectData> Scene::getCircles() const {
    std::vector<OurPaintDCM::Utils::FigureDescriptor> circles = DCM_manager.getAllCircles();
    std::vector<ObjectData> objs;
    objs.reserve(circles.size());
    for (auto c : circles) {
        ObjectData od;
        od.et = ObjType::ET_CIRCLE;
        od.params = { c.coords[0], c.coords[1], c.radius.value() };
        od.id = SceneObjects::ID(c.id.value().id);
        od.subObjects = { SceneObjects::ID(c.pointIds[0].id) };
        objs.push_back(od);
    }

    return objs;
}

std::vector<ObjectData> Scene::getArcs() const {
    std::vector<ObjectData> objs;

    std::vector<OurPaintDCM::Utils::FigureDescriptor> desc = DCM_manager.getAllFigures();
    for (auto& f : desc) {
        if (f.type == OurPaintDCM::Utils::FigureType::ET_ARC) {
            ObjectData od;
            od.et = ObjType::ET_ARC;
            od.id = SceneObjects::ID(f.id.value().id);
            od.params = {f.coords[0], f.coords[1], f.coords[2], f.coords[3], f.coords[4], f.coords[5]};
            objs.push_back(od);
        }
    }

    return objs;
}

std::vector<ObjectData> Scene::getBeziers() const {
    std::vector<ObjectData> beziers;
    for (const auto& [id, b] : beziers_) {
        ObjectData od;
        od.et = ObjType::ET_CUBIC_BEZIER;
        od.id = id;

        od.params.push_back(b.b.start.x);
        od.params.push_back(b.b.start.y);

        od.params.push_back(b.b.end.x);
        od.params.push_back(b.b.end.y);

        od.params.push_back(b.b.control1.x);
        od.params.push_back(b.b.control1.y);

        od.params.push_back(b.b.control2.x);
        od.params.push_back(b.b.control2.y);

        beziers.push_back(od);
    }
    return beziers;
}

std::vector<Requirement> Scene::getRequirements() const {
    throw std::runtime_error("Scene error");
}

std::vector<Requirement> Scene::getObjectRequirements(SceneObjects::ID objectID) const {
    throw std::runtime_error("Scene error");
}

std::vector<Requirement> Scene::getObjectRequirementsWithConnectedObjects(SceneObjects::ID objectID) const {
    throw std::runtime_error("Scene error");
}

void Scene::moveObject(SceneObjects::ID objId, double dx, double dy) {
    DCM_manager.setSolveMode(OurPaintDCM::Utils::SolveMode::LOCAL);

    OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(objId.get());

    std::optional<OurPaintDCM::Utils::FigureDescriptor> desc =  DCM_manager.getFigure(id);
    if (!desc.has_value()) {
        return;
    }

    switch (desc.value().type) {
        case OurPaintDCM::Utils::FigureType::ET_POINT2D: {
            OurPaintDCM::Utils::PointUpdateDescriptor d(id, desc.value().x.value() + dx, desc.value().y.value() + dy);
            DCM_manager.updatePoint(d);
            break;
        }
        case OurPaintDCM::Utils::FigureType::ET_LINE: {
            OurPaintDCM::Utils::PointUpdateDescriptor d1(desc.value().pointIds[0], desc.value().coords[0] + dx, desc.value().coords[1] + dy);
            OurPaintDCM::Utils::PointUpdateDescriptor d2(desc.value().pointIds[1], desc.value().coords[2] + dx, desc.value().coords[3] + dy);
            DCM_manager.updatePoint(d1);
            DCM_manager.updatePoint(d2);
            break;
        }
        case OurPaintDCM::Utils::FigureType::ET_CIRCLE: {
            OurPaintDCM::Utils::PointUpdateDescriptor p(desc.value().pointIds[0], desc.value().coords[0] + dx, desc.value().coords[1] + dy);
            DCM_manager.updatePoint(p);
            break;
        }
        case OurPaintDCM::Utils::FigureType::ET_ARC:
            default: break;
    }

    std::optional<OurPaintDCM::ComponentID> comp = DCM_manager.getComponentForFigure(id);
    std::vector<OurPaintDCM::Utils::ID> figures = DCM_manager.getFiguresInComponent(comp.value());

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }

    DCM_manager.setSolveMode(OurPaintDCM::Utils::SolveMode::DRAG);
    DCM_manager.solve();
}

void Scene::moveObjects(std::vector<SceneObjects::ID> ids, double dx, double dy) {
    std::unordered_set<OurPaintDCM::Utils::ID> figures;
    std::unordered_set<Scene::ID> points;

    DCM_manager.setSolveMode(OurPaintDCM::Utils::SolveMode::LOCAL);

    for (const auto& objId : ids) {
        OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(objId.get());

        std::optional<OurPaintDCM::Utils::FigureDescriptor> desc =  DCM_manager.getFigure(id);
        if (!desc.has_value()) {
            return;
        }

        switch (desc.value().type) {
            case OurPaintDCM::Utils::FigureType::ET_POINT2D: {
                points.insert(desc.value().id.value());
                break;
            }
            case OurPaintDCM::Utils::FigureType::ET_LINE: {
                points.insert(desc.value().pointIds[0]);
                points.insert(desc.value().pointIds[1]);
                break;
            }
            case OurPaintDCM::Utils::FigureType::ET_CIRCLE: {
                points.insert(desc.value().pointIds[0]);
                break;
            }
            case OurPaintDCM::Utils::FigureType::ET_ARC:
                break;
            default: break;
        }

        std::optional<OurPaintDCM::ComponentID> comp = DCM_manager.getComponentForFigure(id);
        std::vector<OurPaintDCM::Utils::ID> compFigures = DCM_manager.getFiguresInComponent(comp.value());
        figures.insert(compFigures.begin(), compFigures.end());
    }

    for (const auto& id : points) {
        std::optional<OurPaintDCM::Utils::FigureDescriptor> desc =  DCM_manager.getFigure(id);
        if (!desc.has_value()) {
            return;
        }

        OurPaintDCM::Utils::PointUpdateDescriptor d(id, desc.value().x.value() + dx, desc.value().y.value() + dy);
        DCM_manager.updatePoint(d);
    }

    for (const auto& p : DCM_manager.getAllPoints()) {
        auto id = SceneObjects::ID(p.id.value().id);
        if (pointToBezier_.contains(id)) {
            SceneObjects::ID& b_id = pointToBezier_[SceneObjects::ID(p.id.value().id)];
            bezier& b = beziers_[b_id];
            if (b.start == id) {
                b.b.start = {p.x.value(), p.y.value()};
            }
            else if (b.end == id) {
                b.b.end = {p.x.value(), p.y.value()};
            }
            else if (b.control1 == id) {
                b.b.control1 = {p.x.value(), p.y.value()};
            }
            else if (b.control2 == id) {
                b.b.control2 = {p.x.value(), p.y.value()};
            }
        }
    }

    DCM_manager.setSolveMode(OurPaintDCM::Utils::SolveMode::DRAG);
    DCM_manager.solve();

    for (auto& observer : _observers) {
        observer->onObjectUpdated({figures.begin(), figures.end()});
    }
}

void Scene::movePoint(SceneObjects::ID pointID, double dx, double dy) {
    OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(pointID.get());

    OurPaintDCM::Utils::FigureDescriptor desc = DCM_manager.getFigure(id).value();

    OurPaintDCM::Utils::PointUpdateDescriptor d(id, desc.x.value() + dx, desc.y.value() + dy);
    DCM_manager.updatePoint(d);

    std::optional<OurPaintDCM::ComponentID> comp = DCM_manager.getComponentForFigure(id);
    std::vector<OurPaintDCM::Utils::ID> figures = DCM_manager.getFiguresInComponent(comp.value());

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }
}

void Scene::moveLine(SceneObjects::ID lineID, double dx, double dy) {
    OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(lineID.get());

    OurPaintDCM::Utils::FigureDescriptor desc = DCM_manager.getFigure(id).value();

    OurPaintDCM::Utils::PointUpdateDescriptor d1(desc.pointIds[0], desc.coords[0] + dx, desc.coords[1] + dy);
    OurPaintDCM::Utils::PointUpdateDescriptor d2(desc.pointIds[1], desc.coords[2] + dx, desc.coords[3] + dy);
    DCM_manager.updatePoint(d1);
    DCM_manager.updatePoint(d2);

    std::optional<OurPaintDCM::ComponentID> comp = DCM_manager.getComponentForFigure(OurPaintDCM::Utils::ID(lineID.get()));
    std::vector<OurPaintDCM::Utils::ID> figures = DCM_manager.getFiguresInComponent(comp.value());

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }
}

void Scene::moveCircle(SceneObjects::ID circleID, double dx, double dy) {
    OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(circleID.get());

    OurPaintDCM::Utils::FigureDescriptor desc = DCM_manager.getFigure(id).value();

    OurPaintDCM::Utils::PointUpdateDescriptor p(desc.pointIds[0], desc.coords[0] + dx, desc.coords[1] + dy);
    DCM_manager.updatePoint(p);

    std::optional<OurPaintDCM::ComponentID> comp = DCM_manager.getComponentForFigure(OurPaintDCM::Utils::ID(circleID.get()));
    std::vector<OurPaintDCM::Utils::ID> figures = DCM_manager.getFiguresInComponent(comp.value());

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }
}

void Scene::moveArc(SceneObjects::ID arcID, double dx, double dy) {
    throw std::runtime_error("Scene error");
}

void Scene::setPoint(SceneObjects::ID pointID, double x, double y, const bool updateRequirementFlag) {


}

void Scene::setSection(SceneObjects::ID sectionID, double x1, double y1, double x2, double y2, const bool updateRequirementFlag) {
    throw std::runtime_error("Scene error");
}

void Scene::setCircle(SceneObjects::ID circleID, double x, double y, double r, const bool updateRequirementFlag) {
    throw std::runtime_error("Scene error");
}

void Scene::setArc(SceneObjects::ID arcID, double x0, double y0, double x1, double y1, double x2, double y2, double,
                   const bool updateRequirementFlag) {
    throw std::runtime_error("Scene error");
}

SceneObjects::ID Scene::addRequirement(const Requirement& reqData, const bool updateRequirementFlag) {
    ReqDescriptor rd;
    switch (reqData.type) {
        case ReqType::ET_POINTLINEDIST:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_POINTLINEDIST;
            break;
        case ReqType::ET_POINTONLINE:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_POINTONLINE;
            break;
        case ReqType::ET_POINTPOINTDIST:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_POINTPOINTDIST;
            break;
        case ReqType::ET_POINTONPOINT:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_POINTONPOINT;
            break;
        case ReqType::ET_LINECIRCLEDIST:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_LINECIRCLEDIST;
            break;
        case ReqType::ET_LINEONCIRCLE:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_LINEONCIRCLE;
            break;
        case ReqType::ET_LINEINCIRCLE:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_LINEINCIRCLE;
            break;
        case ReqType::ET_LINELINEPARALLEL:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_LINELINEPARALLEL;
            break;
        case ReqType::ET_LINELINEPERPENDICULAR:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_LINELINEPERPENDICULAR;
            break;
        case ReqType::ET_LINELINEANGLE:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_LINELINEANGLE;
            break;
        case ReqType::ET_ARCCENTERONPERPENDICULAR:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_ARCCENTERONPERPENDICULAR;
            break;
        case ReqType::ET_FIXPOINT:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_FIXPOINT;
            break;
        case ReqType::ET_FIXLINE:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_FIXLINE;
            break;
        case ReqType::ET_FIXCIRCLE:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_FIXCIRCLE;
            break;
        default:
            break;
    }

    std::vector<ID> ids;
    if (hasObject(reqData.obj1)) {
        ids.push_back(ID(reqData.obj1.get()));
        if (hasObject(reqData.obj2)) {
            ids.push_back(ID(reqData.obj2.get()));
            if (hasObject(reqData.obj3)) {
                ids.push_back(ID(reqData.obj3.get()));
            }
        }
    }
    rd.objectIds = ids;
    if (reqData.param.has_value()) {
        rd.param = reqData.param;
    }

    DCM_manager.addRequirement(rd);
    DCM_manager.solve();

    for (auto& observer : _observers) {
        observer->onRequirementAdded(rd);
    }

    return SceneObjects::ID();
}

void Scene::addRequirement(const Requirement& reqData, SceneObjects::ID reqID) {
    throw std::runtime_error("Scene error");
}

Component& Scene::findComponentByID(SceneObjects::ID id) {
    throw std::runtime_error("Scene error");
}

std::vector<Variable*> Scene::getVariables(const Requirement& req) const {
    throw std::runtime_error("Scene error");
}

Function* Scene::getFunction(const Requirement& req) {
    throw std::runtime_error("Scene error");
}

void Scene::rebuildComponents() {
    throw std::runtime_error("Scene error");
}

void Scene::updateRequirements(SceneObjects::ID id) {}

Requirement Scene::getRequirementData(SceneObjects::ID reqID) const {
    throw std::runtime_error("Scene error");
}

std::vector<Requirement> Scene::getAllRequirementsData() const {
    throw std::runtime_error("Scene error");
}

bool Scene::deleteRequirement(SceneObjects::ID reqID) {
    throw std::runtime_error("Scene error");
}

bool Scene::tryRestoreObject(const ObjectData& data, SceneObjects::ID id) {
    throw std::runtime_error("Scene error");
}

bool Scene::tryRestoreRequirement(const Requirement& data, SceneObjects::ID id) {
    throw std::runtime_error("Scene error");
}

ObjType Scene::getObjType(SceneObjects::ID id) const {
    throw std::runtime_error("Scene error");
}

bool Scene::exists(SceneObjects::ID id, ObjType expected) const {
    throw std::runtime_error("Scene error");
}

bool Scene::isValid(const Requirement& req) const {
    throw std::runtime_error("Scene error");
}

void Scene::setObserver(ISceneObserver* o) {
    _observers.push_back(o);
    const Storage& st = DCM_manager.getStorage();
    o->setStorage(&st);
    storage_ = &st;
}

void Scene::load(const std::vector<ObjectData>& objs, const std::vector<Requirement>& reqs) {
    throw std::runtime_error("Scene error");
}

ObjectContainer& Scene::getObjectContainer() {
    return _objectContainer;
}




