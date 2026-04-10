#include "Scene.h"

#include "Component.h"
#include "Enums.h"
#include "GeometricObjects.h"
#include "LMWithSparse.h"
#include "Objects.h"
#include "ISceneObserver.h"

#include "Scene_ID.h"

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
            SceneObjects::ID inner_id(id.id - 1);

            for (auto& observer : _observers) {
                //observer->onObjectUpserted(objData);
            }

            return SceneObjects::ID();
        }
        default:
            throw std::invalid_argument("Unknown object type");
    }
}

bool Scene::deleteObject(SceneObjects::ID objectID) {
    throw std::runtime_error("Scene error");
}

bool Scene::deletePoint(SceneObjects::ID pointID) {
    throw std::runtime_error("Scene error");
}

bool Scene::deleteSection(SceneObjects::ID sectionID) {
    throw std::runtime_error("Scene error");
}

bool Scene::deleteCircle(SceneObjects::ID circleID) {
    throw std::runtime_error("Scene error");
}

bool Scene::deleteArc(SceneObjects::ID arcID) {
    throw std::runtime_error("Scene error");
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
    throw std::runtime_error("Scene error");
}

bool Scene::hasRequirement(SceneObjects::ID id) const {
    throw std::runtime_error("Scene error");
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

#include "FigureDescriptor.h"

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
        od.id = SceneObjects::ID(f.id.value().id - 1);
        objects.push_back(od);
    }
    return objects;
}
std::vector<ObjectData> Scene::getPoints() const {

    const std::vector<OurPaintDCM::Figures::FigureRef<OurPaintDCM::Figures::Point2D>>& points = storage_->pointsWithIds();
    std::vector objs(points.size(), ObjectData(ObjType::ET_POINT));
    for (int i = 0; i < points.size(); i++) {
        objs[i].params = { points[i].ptr->x(), points[i].ptr->y() };
        objs[i].id = SceneObjects::ID(points[i].id.id - 1);
    }

    // std::vector<OurPaintDCM::Utils::FigureDescriptor> desc = DCM_manager.getAllPoints();
    // for (auto& f : desc) {
    //     ObjectData od;
    //     od.et = ObjType::ET_POINT;
    //     od.id = SceneObjects::ID(f.id.value().id - 1);
    //     od.params = {f.x.value(), f.y.value()};
    //     objs.push_back(od);
    // }

    // std::span<const OurPaintDCM::Figures::Point2D> points = storage_->allPoints();
    // objs.reserve(points.size());
    // for (auto& p : points) {
    //     ObjectData od;
    //     od.params = {p.x(), p.y()};
    //     od.et = ObjType::ET_POINT;
    //     objs.push_back(od);
    // }

    return objs;
}

std::vector<ObjectData> Scene::getLines() const {
    std::vector<ObjectData> objs;

    std::vector<OurPaintDCM::Utils::FigureDescriptor> desc = DCM_manager.getAllLines();
    for (auto& f : desc) {
        ObjectData od;
        od.et = ObjType::ET_LINE;
        od.id = SceneObjects::ID(f.id.value().id - 1);
        od.params = {f.coords[0], f.coords[1], f.coords[2], f.coords[3]};
        objs.push_back(od);
    }

    // std::span<const OurPaintDCM::Figures::Line2D> lines = storage_->allLines();
    // objs.reserve(lines.size());
    // for (auto& l : lines) {
    //     ObjectData od;
    //     od.params = {l.p1->x(), l.p1->y(), l.p2->x(), l.p2->y()};
    //     od.et = ObjType::ET_SECTION;
    //     objs.push_back(od);
    // }

    return objs;
}

std::vector<ObjectData> Scene::getCircles() const {
    const std::vector<OurPaintDCM::Figures::FigureRef<OurPaintDCM::Figures::Circle2D>>& circles = storage_->circlesWithIds();
    std::vector objs(circles.size(), ObjectData(ObjType::ET_CIRCLE));
    for (int i = 0; i < circles.size(); i++) {
        objs[i].params = { circles[i].ptr->center->x(), circles[i].ptr->center->y(), circles[i].ptr->radius };
        objs[i].id = SceneObjects::ID(circles[i].id.id - 1);
    }

    // std::vector<ObjectData> objs;
    // std::vector<OurPaintDCM::Utils::FigureDescriptor> desc = DCM_manager.getAllCircles();
    // for (auto& f : desc) {
    //     ObjectData od;
    //     od.et = ObjType::ET_CIRCLE;
    //     od.id = SceneObjects::ID(f.id.value().id - 1);
    //     od.params = {f.coords[0], f.coords[1], f.radius.value()};
    //     objs.push_back(od);
    // }

    // std::span<const OurPaintDCM::Figures::Circle2D> circles = storage_->allCircles();
    // objs.reserve(circles.size());
    // for (auto& c : circles) {
    //     ObjectData od;
    //     od.params = {c.center->x(), c.center->y(), c.radius};
    //     od.et = ObjType::ET_CIRCLE;
    //     objs.push_back(od);
    // }

    return objs;
}

std::vector<ObjectData> Scene::getArcs() const {
    std::vector<ObjectData> objs;

    std::vector<OurPaintDCM::Utils::FigureDescriptor> desc = DCM_manager.getAllFigures();
    for (auto& f : desc) {
        if (f.type == OurPaintDCM::Utils::FigureType::ET_ARC) {
            ObjectData od;
            od.et = ObjType::ET_ARC;
            od.id = SceneObjects::ID(f.id.value().id - 1);
            od.params = {f.coords[0], f.coords[1], f.coords[2], f.coords[3], f.coords[4], f.coords[5]};
            objs.push_back(od);
        }
    }

    // std::span<const OurPaintDCM::Figures::Arc2D> arcs = storage_->allArcs();
    // objs.reserve(arcs.size());
    // for (auto& a : arcs) {
    //     ObjectData od;
    //     od.params = {a.p1->x(), a.p1->y(), a.p2->x(), a.p2->y(), a.p_center->x(), a.p_center->y()};
    //     od.et = ObjType::ET_ARC;
    //     objs.push_back(od);
    // }

    return objs;
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

void Scene::moveObject(SceneObjects::ID id, double dx, double dy) {
    throw std::runtime_error("Scene error");
}

void Scene::movePoint(SceneObjects::ID pointID, double dx, double dy) {
    OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(pointID.get() + 1);

    OurPaintDCM::Utils::PointUpdateDescriptor d(id, dx, dy);
    DCM_manager.updatePoint(d);

    std::optional<OurPaintDCM::ComponentID> comp = DCM_manager.getComponentForFigure(OurPaintDCM::Utils::ID(pointID.get() + 1));
    std::vector<OurPaintDCM::Utils::ID> figures = DCM_manager.getFiguresInComponent(comp.value());

    ObjDescriptor desc = DCM_manager.getFigure(id).value();

    std::cout << "Point requested: " << d.newX.value() << ' ' << d.newY.value() << '\n';
    std::cout << "Point       got: " << desc.x.value() << ' ' << desc.y.value() << '\n';
    std::cout << '\n';

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }
}

void Scene::moveLine(SceneObjects::ID lineID, double dx, double dy) {
    OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(lineID.get() + 1);

    OurPaintDCM::Utils::FigureDescriptor desc = DCM_manager.getFigure(id).value();

    OurPaintDCM::Utils::PointUpdateDescriptor d1(desc.pointIds[0], desc.coords[0] + dx, desc.coords[1] + dy);
    OurPaintDCM::Utils::PointUpdateDescriptor d2(desc.pointIds[1], desc.coords[2] + dx, desc.coords[3] + dy);
    DCM_manager.updatePoint(d1);
    DCM_manager.updatePoint(d2);

    std::optional<OurPaintDCM::ComponentID> comp = DCM_manager.getComponentForFigure(OurPaintDCM::Utils::ID(lineID.get() + 1));
    std::vector<OurPaintDCM::Utils::ID> figures = DCM_manager.getFiguresInComponent(comp.value());

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }
}

void Scene::moveCircle(SceneObjects::ID circleID, double dx, double dy) {
    OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(circleID.get() + 1);

    OurPaintDCM::Utils::FigureDescriptor desc = DCM_manager.getFigure(id).value();

    OurPaintDCM::Utils::PointUpdateDescriptor p(desc.pointIds[0], desc.coords[0] + dx, desc.coords[1] + dy);
    DCM_manager.updatePoint(p);

    std::optional<OurPaintDCM::ComponentID> comp = DCM_manager.getComponentForFigure(OurPaintDCM::Utils::ID(circleID.get() + 1));
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

std::vector<const double*> Scene::getPointParams(SceneObjects::ID pointID) const {
    throw std::runtime_error("Scene error");
}

std::vector<const double*> Scene::getSectionParams(SceneObjects::ID sectionID) const {
    throw std::runtime_error("Scene error");
}

std::vector<const double*> Scene::getCircleParams(SceneObjects::ID circleID) const {
    throw std::runtime_error("Scene error");
}

std::vector<const double*> Scene::getArcParams(SceneObjects::ID arcID) const {
    throw std::runtime_error("Scene error");
}

SceneObjects::ID Scene::addRequirement(const Requirement& reqData, const bool updateRequirementFlag) {
    ReqDescriptor rd;
    switch (reqData.type) {
        case ReqType::ET_POINTPOINTDIST:
            rd.type = OurPaintDCM::Utils::RequirementType::ET_POINTPOINTDIST;
            break;
        default:
            break;
    }
    rd.objectIds = { ID(reqData.obj1.get()), ID(reqData.obj2.get()) };
    rd.param = reqData.param;

    DCM_manager.addRequirement(rd);

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




