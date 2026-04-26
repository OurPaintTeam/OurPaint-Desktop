#include "Scene.h"

#include "objects/Component.h"
#include "objects/Enums.h"
#include "objects/GeometricObjects.h"
#include "LMWithSparse.h"
#include "objects/Objects.h"
#include "ISceneObserver.h"

Scene::Scene() :
    _objectContainer() {

    DCM_manager.setSolveMode(OurPaintDCM::Utils::SolveMode::DRAG);
}

Scene::~Scene() {}

ID Scene::addObject(const ObjectData& objData) {
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

            return ID(id.id);
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

            return ID(id.id);
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

            return ID(id.id);
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

            return ID(id.id);
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

            pointToBezier_[ID(id1.id)] = ID(lastBezierId);
            pointToBezier_[ID(id2.id)] = ID(lastBezierId);
            pointToBezier_[ID(id3.id)] = ID(lastBezierId);
            pointToBezier_[ID(id4.id)] = ID(lastBezierId);

            CubicBezier b({
                Point(x1, y1),
                Point(x2, y2),
                Point(x3, y3),
                Point(x4, y4)
            });

            beziers_[ID(lastBezierId)] = {
                b,
                ID(id1.id),
                ID(id2.id),
                ID(id3.id),
                ID(id4.id)
            };

            for (auto& observer : _observers) {
                //observer->onObjectAdded(l);
            }

            return ID();
        }
        default:
            throw std::invalid_argument("Unknown object type");
    }
}

bool Scene::deleteObject(ID objectID) {
    OurPaintDCM::Utils::ID id(objectID.get());
    try {
        DCM_manager.removeFigure(id, true);
        if (pointToBezier_.contains(ID(id.id))) {
            ID& b_id = pointToBezier_[ID(id.id)];
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

bool Scene::deletePoint(ID pointID) {
    return deleteObject(pointID);
}

bool Scene::deleteLine(ID lineID) {
    return deleteObject(lineID);
}

bool Scene::deleteCircle(ID circleID) {
    return deleteObject(circleID);
}

bool Scene::deleteArc(ID arcID) {
    return deleteObject(arcID);
}

void Scene::clear() {
    DCM_manager.clear();
    beziers_.clear();
    pointToBezier_.clear();
}

const BoundBox2D& Scene::getBoundingBox() const {
    throw std::runtime_error("Scene error");
}

void Scene::updateBoundingBox() const {
    throw std::runtime_error("Scene error: update Bounding box");
}

ObjectData Scene::getObjectData(SCENE_ID id) const {
    ObjectData od;
    OurPaintDCM::Utils::FigureDescriptor desc = DCM_manager.getFigure(DCM_ID(id.get())).value();
    od.id = ID(desc.id.value().id);
    switch (desc.type) {
        case OurPaintDCM::Utils::FigureType::ET_POINT2D:
            od.et = ObjType::ET_POINT;
            od.params.push_back(desc.coords[0]);
            od.params.push_back(desc.coords[1]);
            break;
        case OurPaintDCM::Utils::FigureType::ET_LINE:
            od.et = ObjType::ET_LINE;
            od.params = {desc.coords[0], desc.coords[1], desc.coords[2], desc.coords[3]};
            od.subObjects = {SCENE_ID(desc.pointIds[0].id), SCENE_ID(desc.pointIds[1].id)};
            break;
        case OurPaintDCM::Utils::FigureType::ET_CIRCLE:
            od.params.push_back(desc.coords[0]);
            od.params.push_back(desc.coords[1]);
            od.params.push_back(desc.radius.value());
            od.subObjects = {SCENE_ID(desc.pointIds[0].id)};
            od.et = ObjType::ET_CIRCLE;
            break;
        case OurPaintDCM::Utils::FigureType::ET_ARC:
            od.params.push_back(desc.coords[0]);
            od.params.push_back(desc.coords[1]);
            od.params.push_back(desc.coords[2]);
            od.params.push_back(desc.coords[3]);
            od.params.push_back(desc.coords[4]);
            od.params.push_back(desc.coords[5]);
            od.subObjects = {SCENE_ID(desc.pointIds[0].id), SCENE_ID(desc.pointIds[1].id), SCENE_ID(desc.pointIds[1].id)};
            od.et = ObjType::ET_ARC;
            break;
    }
    return od;
}

bool Scene::hasObject(ID id) const {
    return DCM_manager.hasFigure(OurPaintDCM::Utils::ID(id.get()));
}

bool Scene::hasRequirement(ID id) const {
    return DCM_manager.hasRequirement(OurPaintDCM::Utils::ID(id.get()));
}

ObjectData Scene::getRootObjectData(ID id) const {
    throw std::runtime_error("Scene error");
}

Requirement Scene::getRequirementData(ID object1, ID object2) const {
    throw std::runtime_error("Scene error");
}

std::size_t Scene::objectsCount() const {
    return DCM_manager.figureCount();
}

std::size_t Scene::requirementsCount() const {
    return DCM_manager.requirementCount();
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
        od.id = ID(f.id.value().id);
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
        od.id = ID(d.id.value().id);
        od.params = {d.x.value(), d.y.value()};
        objs.push_back(od);
    }

    return objs;
}

void Scene::appendPickedPointsInRect(double rx1, double ry1, double rx2, double ry2, std::vector<ID>& out) const {
    const auto& points = DCM_manager.getStorage().pointsWithIds();

    for (const auto& ref : points) {
        const double x = ref.ptr->x();
        const double y = ref.ptr->y();

        if (x >= rx1 && x <= rx2 && y >= ry1 && y <= ry2) {
            out.push_back(ID(ref.id.id));
        }
    }
}

std::vector<ObjectData> Scene::getLines() const {
    std::vector<ObjectData> objs;

    std::vector<OurPaintDCM::Utils::FigureDescriptor> desc = DCM_manager.getAllLines();
    for (auto& f : desc) {
        ObjectData od;
        od.et = ObjType::ET_LINE;
        od.id = ID(f.id.value().id);
        od.params = {f.coords[0], f.coords[1], f.coords[2], f.coords[3]};
        od.subObjects = {ID(f.pointIds[0].id), ID(f.pointIds[1].id) };
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
        od.id = ID(c.id.value().id);
        od.subObjects = { ID(c.pointIds[0].id) };
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
            od.id = ID(f.id.value().id);
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

std::vector<Requirement> Scene::getObjectRequirements(ID objectID) const {
    throw std::runtime_error("Scene error");
}

std::vector<Requirement> Scene::getObjectRequirementsWithConnectedObjects(ID objectID) const {
    throw std::runtime_error("Scene error");
}

void Scene::moveObject(ID objId, double dx, double dy) {
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

void Scene::moveObjects(std::vector<ID> ids, double dx, double dy) {
    std::unordered_set<OurPaintDCM::Utils::ID> figures;
    std::unordered_set<DCM_ID> points;

    for (const auto& objId : ids) {
        OurPaintDCM::Utils::ID id = OurPaintDCM::Utils::ID(objId.get());

        std::optional<OurPaintDCM::Utils::FigureDescriptor> desc =  DCM_manager.getFigure(id);
        if (!desc.has_value()) {
            return;
        }

        switch (desc.value().type) {
            case OurPaintDCM::Utils::FigureType::ET_POINT2D: {
                points.insert(id);
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
    }

    std::vector<OurPaintDCM::Utils::PointUpdateDescriptor> descs;
    descs.reserve(points.size());
    for (const auto& id : points) {
        std::optional<OurPaintDCM::Utils::FigureDescriptor> desc =  DCM_manager.getFigure(id);
        if (!desc.has_value()) {
            return;
        }

        OurPaintDCM::Utils::PointUpdateDescriptor d(id, desc.value().x.value() + dx, desc.value().y.value() + dy);
        descs.push_back(d);
    }
    DCM_manager.updatePoints(descs);

    for (const auto& p : DCM_manager.getAllPoints()) {
        auto id = ID(p.id.value().id);
        if (pointToBezier_.contains(id)) {
            ID& b_id = pointToBezier_[ID(p.id.value().id)];
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

    for (auto& observer : _observers) {
        observer->onObjectUpdated({figures.begin(), figures.end()});
    }
}

void Scene::movePoint(ID pointID, double dx, double dy) {
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

void Scene::moveLine(ID lineID, double dx, double dy) {
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

void Scene::moveCircle(ID circleID, double dx, double dy) {
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

void Scene::moveArc(ID arcID, double dx, double dy) {
    throw std::runtime_error("Scene error");
}

void Scene::resizeCircle(ID circleId, double radius) {
    if (radius <= 0.0) {
        return;
    }

    OurPaintDCM::Utils::CircleUpdateDescriptor c(DCM_ID(circleId.get()), radius);
    DCM_manager.updateCircle(c);
}
void Scene::setPoint(ID pointID, double x, double y, const bool updateRequirementFlag) {


}

void Scene::setSection(ID sectionID, double x1, double y1, double x2, double y2, const bool updateRequirementFlag) {
    throw std::runtime_error("Scene error");
}

void Scene::setCircle(ID circleID, double x, double y, double r, const bool updateRequirementFlag) {
    throw std::runtime_error("Scene error");
}

void Scene::setArc(ID arcID, double x0, double y0, double x1, double y1, double x2, double y2, double,
                   const bool updateRequirementFlag) {
    throw std::runtime_error("Scene error");
}

ID Scene::addRequirement(const Requirement& reqData, const bool updateRequirementFlag) {
    ReqDescriptor rd;

    rd.type = reqTypeMapper(reqData.type);

    std::vector<DCM_ID> ids;
    if (hasObject(reqData.obj1)) {
        ids.push_back(DCM_ID(reqData.obj1.get()));
        if (hasObject(reqData.obj2)) {
            ids.push_back(DCM_ID(reqData.obj2.get()));
            if (hasObject(reqData.obj3)) {
                ids.push_back(DCM_ID(reqData.obj3.get()));
            }
        }
    }
    rd.objectIds = ids;
    if (reqData.param.has_value()) {
        rd.param = reqData.param;
    }

    DCM_manager.addRequirement(rd);
    if (updateRequirementFlag) {
        DCM_manager.solve();
    }

    for (auto& observer : _observers) {
        observer->onRequirementAdded(rd);
    }

    return ID();
}

static bool requirementFullyInside(const Requirement& r, const std::unordered_set<ID>& copiedIds) {
    auto insideIfUsed = [&](ID id) -> bool {
        return id == ID{0} || copiedIds.contains(id);
    };

    return insideIfUsed(r.obj1)
        && insideIfUsed(r.obj2)
        && insideIfUsed(r.obj3);
}

ClipboardData Scene::copyFragment(const std::vector<ID>& selected) const {
    ClipboardData data;

    BoundBox2D b = makeBoundingBoxFromObjects(selected);
    if (b.isValid()) {
        data.centerX = (b.min_x + b.max_x) * 0.5;
        data.centerY = (b.min_y + b.max_y) * 0.5;
    }

    std::unordered_set<ID> includedPoints;
    for (const auto id : selected) {
        ObjectData od = getObjectData(id);
        if (od.et != ObjType::ET_POINT) {
            includedPoints.insert(od.subObjects.begin(), od.subObjects.end());
        }
    }

    std::unordered_set<ID> copiedIds;

    for (const auto id : selected) {
        ObjectData od = getObjectData(id);

        if (od.et == ObjType::ET_POINT) {
            if (!includedPoints.contains(id)) {
                data.objects.push_back(od);
                copiedIds.insert(od.id);
            }
        } else {
            data.objects.push_back(od);
            copiedIds.insert(od.id);

            for (const auto subId : od.subObjects) {
                copiedIds.insert(subId);
            }
        }
    }

    std::unordered_set<DCM_ID> reqs;
    for (const auto id : selected) {
        auto compOpt = DCM_manager.getComponentForFigure(DCM_ID(id.get()));
        if (!compOpt.has_value()) continue;

        std::vector<DCM_ID> reqIds = DCM_manager.getRequirementsInComponent(compOpt.value());
        reqs.insert(reqIds.begin(), reqIds.end());
    }

    for (const auto reqId : reqs) {
        Requirement r = getRequirementData(ID(reqId.id));
        if (requirementFullyInside(r, copiedIds)) {
            data.requirements.push_back(r);
        }
    }

    return data;
}

std::vector<ID> Scene::pasteFragment(const ClipboardData& data, double targetPosX, double targetPosY) {
    std::vector<ID> copiedObjectsNewIDs;
    std::unordered_map<ID, ID> oldToNewObjectIds;

    for (const auto& od : data.objects) {
        auto newID = addObject(od);
        ObjectData newOd = getObjectData(newID);

        copiedObjectsNewIDs.push_back(newID);
        oldToNewObjectIds[od.id] = newID;

        for (size_t i = 0; i < od.subObjects.size() && i < newOd.subObjects.size(); ++i) {
            oldToNewObjectIds[od.subObjects[i]] = newOd.subObjects[i];
        }
    }

    moveObjects(copiedObjectsNewIDs, targetPosX - data.centerX, targetPosY - data.centerY);

    auto remapIfValid = [&](ID& id) -> bool {
        if (id == ID{0}) return true;
        auto it = oldToNewObjectIds.find(id);
        if (it == oldToNewObjectIds.end()) return false;
        id = it->second;
        return true;
    };

    for (const auto& r : data.requirements) {
        Requirement req = r;

        if (!remapIfValid(req.obj1)) continue;
        if (!remapIfValid(req.obj2)) continue;
        if (!remapIfValid(req.obj3)) continue;

        addRequirement(req, false);
    }
    DCM_manager.solve();

    return copiedObjectsNewIDs;
}

BoundBox2D Scene::makeBoundingBoxForObject(ID id) const {
    BoundBox2D b;
    ObjectData od = getObjectData(id);
    switch (od.et) {
        case ObjType::ET_POINT: {
            b.expand(od.params[0], od.params[1]);
            break;
        }
        case ObjType::ET_LINE: {
            b.expand(od.params[0], od.params[1]);
            b.expand(od.params[2], od.params[3]);
            break;
        }
        case ObjType::ET_CIRCLE: {
            double cx = od.params[0];
            double cy = od.params[1];
            double r = od.params[2];
            b.expand(cx - r, cy - r);
            b.expand(cx + r, cy + r);
            break;
        }
        default:
            break;
    }
    return b;
}

BoundBox2D Scene::makeBoundingBoxFromObjects(const std::vector<ID>& objects) const {
    BoundBox2D total;
    for (const auto& id : objects) {
        total = total | makeBoundingBoxForObject(id);
    }
    return total;
}

void Scene::addRequirement(const Requirement& reqData, ID reqID) {
    throw std::runtime_error("Scene error");
}

ReqType Scene::reqTypeMapper(OurPaintDCM::Utils::RequirementType type) {
    switch (type) {
        case OurPaintDCM::Utils::RequirementType::ET_POINTLINEDIST:
            return ReqType::ET_POINTLINEDIST;
        case OurPaintDCM::Utils::RequirementType::ET_POINTONLINE:
            return ReqType::ET_POINTONLINE;
        case OurPaintDCM::Utils::RequirementType::ET_POINTPOINTDIST:
            return ReqType::ET_POINTPOINTDIST;
        case OurPaintDCM::Utils::RequirementType::ET_POINTONPOINT:
            return ReqType::ET_POINTONPOINT;
        case OurPaintDCM::Utils::RequirementType::ET_LINECIRCLEDIST:
            return ReqType::ET_LINECIRCLEDIST;
        case OurPaintDCM::Utils::RequirementType::ET_LINEONCIRCLE:
            return ReqType::ET_LINEONCIRCLE;
        case OurPaintDCM::Utils::RequirementType::ET_LINEINCIRCLE:
            return ReqType::ET_LINEINCIRCLE;
        case OurPaintDCM::Utils::RequirementType::ET_LINELINEPARALLEL:
            return ReqType::ET_LINELINEPARALLEL;
        case OurPaintDCM::Utils::RequirementType::ET_LINELINEPERPENDICULAR:
            return ReqType::ET_LINELINEPERPENDICULAR;
        case OurPaintDCM::Utils::RequirementType::ET_LINELINEANGLE:
            return ReqType::ET_LINELINEANGLE;
        case OurPaintDCM::Utils::RequirementType::ET_ARCCENTERONPERPENDICULAR:
            return ReqType::ET_ARCCENTERONPERPENDICULAR;
        case OurPaintDCM::Utils::RequirementType::ET_FIXPOINT:
            return ReqType::ET_FIXPOINT;
        case OurPaintDCM::Utils::RequirementType::ET_FIXLINE:
            return ReqType::ET_FIXLINE;
        case OurPaintDCM::Utils::RequirementType::ET_FIXCIRCLE:
            return ReqType::ET_FIXCIRCLE;
        default:
            return {};
    }
    return {};
}

OurPaintDCM::Utils::RequirementType Scene::reqTypeMapper(ReqType type) {
    switch (type) {
        case ReqType::ET_POINTLINEDIST:
            return OurPaintDCM::Utils::RequirementType::ET_POINTLINEDIST;
        case ReqType::ET_POINTONLINE:
            return OurPaintDCM::Utils::RequirementType::ET_POINTONLINE;
        case ReqType::ET_POINTPOINTDIST:
            return OurPaintDCM::Utils::RequirementType::ET_POINTPOINTDIST;
        case ReqType::ET_POINTONPOINT:
            return OurPaintDCM::Utils::RequirementType::ET_POINTONPOINT;
        case ReqType::ET_LINECIRCLEDIST:
            return OurPaintDCM::Utils::RequirementType::ET_LINECIRCLEDIST;
        case ReqType::ET_LINEONCIRCLE:
            return OurPaintDCM::Utils::RequirementType::ET_LINEONCIRCLE;
        case ReqType::ET_LINEINCIRCLE:
            return OurPaintDCM::Utils::RequirementType::ET_LINEINCIRCLE;
        case ReqType::ET_LINELINEPARALLEL:
            return OurPaintDCM::Utils::RequirementType::ET_LINELINEPARALLEL;
        case ReqType::ET_LINELINEPERPENDICULAR:
            return OurPaintDCM::Utils::RequirementType::ET_LINELINEPERPENDICULAR;
        case ReqType::ET_LINELINEANGLE:
            return OurPaintDCM::Utils::RequirementType::ET_LINELINEANGLE;
        case ReqType::ET_ARCCENTERONPERPENDICULAR:
            return OurPaintDCM::Utils::RequirementType::ET_ARCCENTERONPERPENDICULAR;
        case ReqType::ET_FIXPOINT:
            return OurPaintDCM::Utils::RequirementType::ET_FIXPOINT;
        case ReqType::ET_FIXLINE:
            return OurPaintDCM::Utils::RequirementType::ET_FIXLINE;
        case ReqType::ET_FIXCIRCLE:
            return OurPaintDCM::Utils::RequirementType::ET_FIXCIRCLE;
        default:
            return {};
    }
    return {};
}

Component& Scene::findComponentByID(ID id) {
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

void Scene::updateRequirements(ID id) {}

Requirement Scene::getRequirementData(ID reqID) const {
    OurPaintDCM::Utils::RequirementDescriptor desc = DCM_manager.getRequirement(DCM_ID(reqID.get())).value();
    Requirement r;
    r.type = reqTypeMapper(desc.type);
    auto ids = desc.objectIds;
    if (ids.size() > 0) {
        r.obj1 = ID(ids[0].id);
    }
    if (ids.size() > 1) {
        r.obj2 = ID(ids[1].id);
    }
    if (ids.size() > 2) {
        r.obj3 = ID(ids[2].id);
    }
    if (desc.param.has_value()) {
        r.param = desc.param.value();
    }
    return r;
}

std::vector<Requirement> Scene::getAllRequirementsData() const {
    throw std::runtime_error("Scene error");
}

bool Scene::deleteRequirement(ID reqID) {
    throw std::runtime_error("Scene error");
}

bool Scene::tryRestoreObject(const ObjectData& data, ID id) {
    throw std::runtime_error("Scene error");
}

bool Scene::tryRestoreRequirement(const Requirement& data, ID id) {
    throw std::runtime_error("Scene error");
}

ObjType Scene::getObjType(ID id) const {
    throw std::runtime_error("Scene error");
}

bool Scene::exists(ID id, ObjType expected) const {
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





