#include "Scene.h"
#include "objects/Objects.h"
#include "ISceneObserver.h"
#include "DSU.h"

using namespace core;

Scene::Scene() {
    DCM_.setSolveMode(Utils::SolveMode::DRAG);
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

            DCM_FigDesc od = DCM_FigDesc::point(x, y);
            DCM_ID id = DCM_.addFigure(od);

            // for (auto& observer : _observers) {
            //     observer->onObjectAdded(od);
            // }

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

            DCM_FigDesc l = DCM_FigDesc::line(x1, y1, x2, y2);

            DCM_ID id = DCM_.addFigure(l);

            // for (auto& observer : _observers) {
            //     observer->onObjectAdded(l);
            // }

            return ID(id.id);
        }
        case ObjType::ET_CIRCLE: {
            if (objData.params.size() < 3) {
                throw std::invalid_argument("Circle requires center and radius");
            }

            double cx = objData.params[0];
            double cy = objData.params[1];
            double r = objData.params[2];

            DCM_FigDesc c = DCM_FigDesc::circle(cx, cy, r);
            DCM_ID id = DCM_.addFigure(c);

            // for (auto& observer : _observers) {
            //     observer->onObjectAdded(c);
            // }

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

            DCM_ID id_p1 = DCM_.addFigure(DCM_FigDesc::point(beg_x, beg_y));
            DCM_ID id_p2 = DCM_.addFigure(DCM_FigDesc::point(end_x, end_y));
            DCM_ID id_p3 = DCM_.addFigure(DCM_FigDesc::point(cx, cy));

            DCM_ID id = DCM_.addFigure(DCM_FigDesc::arc(id_p1, id_p2, id_p3));

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


            DCM_FigDesc od1 = DCM_FigDesc::point(x1, y1);
            DCM_FigDesc od2 = DCM_FigDesc::point(x2, y2);
            DCM_FigDesc od3 = DCM_FigDesc::point(x3, y3);
            DCM_FigDesc od4 = DCM_FigDesc::point(x4, y4);

            DCM_ID id1 = DCM_.addFigure(od1);
            DCM_ID id3 = DCM_.addFigure(od3);
            DCM_ID id4 = DCM_.addFigure(od4);
            DCM_ID id2 = DCM_.addFigure(od2);

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
    DCM_ID id(objectID.get());
    try {
        DCM_.removeFigure(id, true);
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
    DCM_.clear();
    beziers_.clear();
    pointToBezier_.clear();
}

const BoundBox2D& Scene::getBoundingBox() const {
    throw std::runtime_error("Scene error");
}

void Scene::updateBoundingBox() const {
    throw std::runtime_error("Scene error: update Bounding box");
}

ObjectData Scene::getObjectData(ID id) const {
    ObjectData od;
    DCM_FigDesc desc = DCM_.getFigure(DCM_ID(id.get())).value();
    od.id = ID(desc.id.value().id);
    switch (desc.type) {
        case DCM_FigType::ET_POINT2D:
            od.et = ObjType::ET_POINT;
            od.params.push_back(desc.coords[0]);
            od.params.push_back(desc.coords[1]);
            break;
        case DCM_FigType::ET_LINE:
            od.et = ObjType::ET_LINE;
            od.params = {desc.coords[0], desc.coords[1], desc.coords[2], desc.coords[3]};
            od.subObjects = {ID(desc.pointIds[0].id), ID(desc.pointIds[1].id)};
            break;
        case DCM_FigType::ET_CIRCLE:
            od.params.push_back(desc.coords[0]);
            od.params.push_back(desc.coords[1]);
            od.params.push_back(desc.radius.value());
            od.subObjects = {ID(desc.pointIds[0].id)};
            od.et = ObjType::ET_CIRCLE;
            break;
        case DCM_FigType::ET_ARC:
            od.params.push_back(desc.coords[0]);
            od.params.push_back(desc.coords[1]);
            od.params.push_back(desc.coords[2]);
            od.params.push_back(desc.coords[3]);
            od.params.push_back(desc.coords[4]);
            od.params.push_back(desc.coords[5]);
            od.subObjects = {ID(desc.pointIds[0].id), ID(desc.pointIds[1].id), ID(desc.pointIds[1].id)};
            od.et = ObjType::ET_ARC;
            break;
    }
    return od;
}

bool Scene::hasObject(ID id) const {
    return DCM_.hasFigure(DCM_ID(id.get()));
}

bool Scene::hasRequirement(ID id) const {
    return DCM_.hasRequirement(DCM_ID(id.get()));
}

ObjectData Scene::getRootObjectData(ID id) const {
    throw std::runtime_error("Scene error");
}

Requirement Scene::getRequirementData(ID object1, ID object2) const {
    throw std::runtime_error("Scene error");
}

std::size_t Scene::objectsCount() const {
    return DCM_.figureCount();
}

std::size_t Scene::requirementsCount() const {
    return DCM_.requirementCount();
}

std::vector<ObjectData> Scene::getObjects() const {
    std::vector<DCM_FigDesc> figures = DCM_.getAllFigures();
    std::vector<ObjectData> objects;
    objects.reserve(figures.size());
    for (auto& f : figures) {
        ObjectData od;
        switch (f.type) {
            case DCM_FigType::ET_POINT2D:
                od.et = ObjType::ET_POINT;
                od.params.push_back(f.coords[0]);
                od.params.push_back(f.coords[1]);
                break;
            case DCM_FigType::ET_LINE:
                od.et = ObjType::ET_LINE;
                od.params.push_back(f.coords[0]);
                od.params.push_back(f.coords[1]);
                od.params.push_back(f.coords[2]);
                od.params.push_back(f.coords[3]);
                break;
            case DCM_FigType::ET_CIRCLE:
                od.params.push_back(f.coords[0]);
                od.params.push_back(f.coords[1]);
                od.params.push_back(f.radius.value());
                od.et = ObjType::ET_CIRCLE;
                break;
            case DCM_FigType::ET_ARC:
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

    std::vector<DCM_FigDesc> points = DCM_.getAllPoints();
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
    const auto& points = DCM_.getStorage().pointsWithIds();

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

    std::vector<DCM_FigDesc> desc = DCM_.getAllLines();
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
    std::vector<DCM_FigDesc> circles = DCM_.getAllCircles();
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

    std::vector<DCM_FigDesc> desc = DCM_.getAllFigures();
    for (auto& f : desc) {
        if (f.type == DCM_FigType::ET_ARC) {
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
    DCM_ID id = DCM_ID(objId.get());

    std::optional<DCM_FigDesc> desc =  DCM_.getFigure(id);
    if (!desc.has_value()) {
        return;
    }

    switch (desc.value().type) {
        case DCM_FigType::ET_POINT2D: {
            DCM_PointUpdateDesc d(id, desc.value().x.value() + dx, desc.value().y.value() + dy);
            DCM_.updatePoint(d);
            break;
        }
        case DCM_FigType::ET_LINE: {
            DCM_PointUpdateDesc d1(desc.value().pointIds[0], desc.value().coords[0] + dx, desc.value().coords[1] + dy);
            DCM_PointUpdateDesc d2(desc.value().pointIds[1], desc.value().coords[2] + dx, desc.value().coords[3] + dy);
            DCM_.updatePoint(d1);
            DCM_.updatePoint(d2);
            break;
        }
        case DCM_FigType::ET_CIRCLE: {
            DCM_PointUpdateDesc p(desc.value().pointIds[0], desc.value().coords[0] + dx, desc.value().coords[1] + dy);
            DCM_.updatePoint(p);
            break;
        }
        case DCM_FigType::ET_ARC:
            default: break;
    }

    std::optional<ComponentID> comp = DCM_.getComponentForFigure(id);
    std::vector<DCM_ID> figures = DCM_.getFiguresInComponent(comp.value());

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }

    DCM_.setSolveMode(Utils::SolveMode::DRAG);
    DCM_.solve();
}

void Scene::moveObjects(std::vector<ID> ids, double dx, double dy) {
    std::unordered_set<DCM_ID> figures;
    std::unordered_set<DCM_ID> points;

    for (const auto& objId : ids) {
        DCM_ID id = DCM_ID(objId.get());

        std::optional<DCM_FigDesc> desc =  DCM_.getFigure(id);
        if (!desc.has_value()) {
            return;
        }

        switch (desc.value().type) {
            case DCM_FigType::ET_POINT2D: {
                points.insert(id);
                break;
            }
            case DCM_FigType::ET_LINE: {
                points.insert(desc.value().pointIds[0]);
                points.insert(desc.value().pointIds[1]);
                break;
            }
            case DCM_FigType::ET_CIRCLE: {
                points.insert(desc.value().pointIds[0]);
                break;
            }
            case DCM_FigType::ET_ARC:
                break;
            default: break;
        }
    }

    std::vector<DCM_PointUpdateDesc> descs;
    descs.reserve(points.size());
    for (const auto& id : points) {
        std::optional<DCM_FigDesc> desc =  DCM_.getFigure(id);
        if (!desc.has_value()) {
            return;
        }

        DCM_PointUpdateDesc d(id, desc.value().x.value() + dx, desc.value().y.value() + dy);
        descs.push_back(d);
    }
    DCM_.updatePoints(descs);

    for (const auto& p : DCM_.getAllPoints()) {
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
    DCM_ID id = DCM_ID(pointID.get());

    DCM_FigDesc desc = DCM_.getFigure(id).value();

    DCM_PointUpdateDesc d(id, desc.x.value() + dx, desc.y.value() + dy);
    DCM_.updatePoint(d);

    std::optional<ComponentID> comp = DCM_.getComponentForFigure(id);
    std::vector<DCM_ID> figures = DCM_.getFiguresInComponent(comp.value());

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }
}

void Scene::moveLine(ID lineID, double dx, double dy) {
    DCM_ID id = DCM_ID(lineID.get());

    DCM_FigDesc desc = DCM_.getFigure(id).value();

    DCM_PointUpdateDesc d1(desc.pointIds[0], desc.coords[0] + dx, desc.coords[1] + dy);
    DCM_PointUpdateDesc d2(desc.pointIds[1], desc.coords[2] + dx, desc.coords[3] + dy);
    DCM_.updatePoint(d1);
    DCM_.updatePoint(d2);

    std::optional<ComponentID> comp = DCM_.getComponentForFigure(DCM_ID(lineID.get()));
    std::vector<DCM_ID> figures = DCM_.getFiguresInComponent(comp.value());

    for (auto& observer : _observers) {
        observer->onObjectUpdated(figures);
    }
}

void Scene::moveCircle(ID circleID, double dx, double dy) {
    DCM_ID id = DCM_ID(circleID.get());

    DCM_FigDesc desc = DCM_.getFigure(id).value();

    DCM_PointUpdateDesc p(desc.pointIds[0], desc.coords[0] + dx, desc.coords[1] + dy);
    DCM_.updatePoint(p);

    std::optional<ComponentID> comp = DCM_.getComponentForFigure(DCM_ID(circleID.get()));
    std::vector<DCM_ID> figures = DCM_.getFiguresInComponent(comp.value());

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

    Utils::CircleUpdateDescriptor c(DCM_ID(circleId.get()), radius);
    DCM_.updateCircle(c);
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
    if (reqData.type == ReqType::ET_POINTONPOINT) {

        // Retrieve current coordinates of both points.
        ID id1 = reqData.obj1;
        ID id2 = reqData.obj2;

        ObjectData leftPoint = getObjectData(id1);
        ObjectData rightPoint = getObjectData(id2);

        double x1 = leftPoint.params[0];
        double y1 = leftPoint.params[1];
        double x2 = rightPoint.params[0];
        double y2 = rightPoint.params[1];

        // Compute midpoint between the two points.
        double midX = (x1 + x2) * 0.5;
        double midY = (y1 + y2) * 0.5;


        // Move both points toward the midpoint before adding
        // the coincidence constraint. This provides a better
        // initial configuration for the solver and produces
        // more intuitive visual behavior in the sketch.
        //
        // Fixed points will remain unchanged because movePoint()
        // ignores movement of constrained points.
        movePoint(leftPoint.id, midX - x1,  midY - y1);
        movePoint(rightPoint.id, midX - x2, midY - y2);
    }

    DCM_ReqDesc rd;

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

    DCM_.addRequirement(rd);
    if (updateRequirementFlag) {
        DCM_.solve();
    }

    // for (auto& observer : _observers) {
    //     observer->onRequirementAdded(rd);
    // }

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
        auto compOpt = DCM_.getComponentForFigure(DCM_ID(id.get()));
        if (!compOpt.has_value()) continue;

        std::vector<DCM_ID> reqIds = DCM_.getRequirementsInComponent(compOpt.value());
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
    DCM_.solve();

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

bool Scene::pointIsFixed(ID pointID) const {
    ObjectData od = getObjectData(pointID);
    if (od.et != ObjType::ET_POINT) {
        return false;
    }

    std::vector<DCM_ReqDesc> reqs = DCM_.getAllRequirements();
    for (const auto& desc : reqs) {
        if (desc.type == DCM_ReqType::ET_FIXPOINT) {
            for (const auto& id : desc.objectIds) {
                if (id.id == pointID.get()) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::vector<Scene::PointGroup> Scene::getPointOnPointGroups() const {
    std::vector<ObjectData> points = getPoints();

    if (points.empty()) {
        return {};
    }

    DSU<ID> dsu;

    for (const auto& p : points) {
        dsu.makeSet(p.id);
    }

    std::vector<DCM_ReqDesc> reqs = DCM_.getAllRequirements();

    for (const auto& req : reqs) {
        if (req.type != DCM_ReqType::ET_POINTONPOINT) {
            continue;
        }

        if (req.objectIds.size() < 2) {
            continue;
        }

        ID p1(req.objectIds[0].id);
        ID p2(req.objectIds[1].id);

        dsu.unite(p1, p2);
    }

    std::unordered_map<ID, PointGroup> groups;

    for (const auto& p : points) {
        ID root = dsu.find(p.id);
        groups[root].points.push_back(p);
    }

    std::vector<PointGroup> result;
    result.reserve(groups.size());

    for (auto& [root, group] : groups) {
        result.push_back(std::move(group));
    }

    return result;
}

void Scene::addRequirement(const Requirement& reqData, ID reqID) {
    throw std::runtime_error("Scene error");
}

ReqType Scene::reqTypeMapper(DCM_ReqType type) {
    switch (type) {
        case DCM_ReqType::ET_POINTLINEDIST:
            return ReqType::ET_POINTLINEDIST;
        case DCM_ReqType::ET_POINTONLINE:
            return ReqType::ET_POINTONLINE;
        case DCM_ReqType::ET_POINTPOINTDIST:
            return ReqType::ET_POINTPOINTDIST;
        case DCM_ReqType::ET_POINTONPOINT:
            return ReqType::ET_POINTONPOINT;
        case DCM_ReqType::ET_LINECIRCLEDIST:
            return ReqType::ET_LINECIRCLEDIST;
        case DCM_ReqType::ET_LINEONCIRCLE:
            return ReqType::ET_LINEONCIRCLE;
        case DCM_ReqType::ET_LINEINCIRCLE:
            return ReqType::ET_LINEINCIRCLE;
        case DCM_ReqType::ET_LINELINEPARALLEL:
            return ReqType::ET_LINELINEPARALLEL;
        case DCM_ReqType::ET_LINELINEPERPENDICULAR:
            return ReqType::ET_LINELINEPERPENDICULAR;
        case DCM_ReqType::ET_LINELINEANGLE:
            return ReqType::ET_LINELINEANGLE;
        case DCM_ReqType::ET_ARCCENTERONPERPENDICULAR:
            return ReqType::ET_ARCCENTERONPERPENDICULAR;
        case DCM_ReqType::ET_FIXPOINT:
            return ReqType::ET_FIXPOINT;
        case DCM_ReqType::ET_FIXLINE:
            return ReqType::ET_FIXLINE;
        case DCM_ReqType::ET_FIXCIRCLE:
            return ReqType::ET_FIXCIRCLE;
        case DCM_ReqType::ET_HORIZONTAL:
            return ReqType::ET_HORIZONTAL;
        case DCM_ReqType::ET_VERTICAL:
            return ReqType::ET_VERTICAL;
        default:
            return {};
    }
    return {};
}

Scene::DCM_ReqType Scene::reqTypeMapper(ReqType type) {
    switch (type) {
        case ReqType::ET_POINTLINEDIST:
            return DCM_ReqType::ET_POINTLINEDIST;
        case ReqType::ET_POINTONLINE:
            return DCM_ReqType::ET_POINTONLINE;
        case ReqType::ET_POINTPOINTDIST:
            return DCM_ReqType::ET_POINTPOINTDIST;
        case ReqType::ET_POINTONPOINT:
            return DCM_ReqType::ET_POINTONPOINT;
        case ReqType::ET_LINECIRCLEDIST:
            return DCM_ReqType::ET_LINECIRCLEDIST;
        case ReqType::ET_LINEONCIRCLE:
            return DCM_ReqType::ET_LINEONCIRCLE;
        case ReqType::ET_LINEINCIRCLE:
            return DCM_ReqType::ET_LINEINCIRCLE;
        case ReqType::ET_LINELINEPARALLEL:
            return DCM_ReqType::ET_LINELINEPARALLEL;
        case ReqType::ET_LINELINEPERPENDICULAR:
            return DCM_ReqType::ET_LINELINEPERPENDICULAR;
        case ReqType::ET_LINELINEANGLE:
            return DCM_ReqType::ET_LINELINEANGLE;
        case ReqType::ET_ARCCENTERONPERPENDICULAR:
            return DCM_ReqType::ET_ARCCENTERONPERPENDICULAR;
        case ReqType::ET_FIXPOINT:
            return DCM_ReqType::ET_FIXPOINT;
        case ReqType::ET_FIXLINE:
            return DCM_ReqType::ET_FIXLINE;
        case ReqType::ET_FIXCIRCLE:
            return DCM_ReqType::ET_FIXCIRCLE;
        case ReqType::ET_HORIZONTAL:
            return DCM_ReqType::ET_HORIZONTAL;
        case ReqType::ET_VERTICAL:
            return DCM_ReqType::ET_VERTICAL;
        default:
            return {};
    }
    return {};
}

void Scene::updateRequirements(ID id) {}

Requirement Scene::getRequirementData(ID reqID) const {
    Utils::RequirementDescriptor desc = DCM_.getRequirement(DCM_ID(reqID.get())).value();
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

void Scene::setObserver(ISceneObserver* o) {
    _observers.push_back(o);
}





