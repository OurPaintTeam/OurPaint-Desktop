#include "GeometricObjects.h"
#include "Scene.h"
#include "SceneObserver.h"
#include "Enums.h"
#include "Painter.h"
#include "Objects.h"
#include "LMWithSparse.h"
#include "Component.h"


const ID Scene::_errorID(-1);
const ID Scene::_connectionEdgeID(-2);

Scene::Scene(Painter* p) :
        _points(),
        _sections(),
        _circles(),
        _arcs(),
        _painter(p),
        _isRectangleDirty(false),
        _allFiguresRectangle(),
        _graph(),
        //_solver(100, 3, 1e-6, 1e-6),
        _requirements(),
        _components(),
        _idToComponent(),
        _isComponentsDirty(false) {
    _points.reserve(std::size_t(1024));
    _sections.reserve(std::size_t(1024));
    _circles.reserve(std::size_t(1024));
    _arcs.reserve(std::size_t(1024));
    _requirements.reserve(std::size_t(512));
    if (p) {
        p->initPointCase(_points);
        p->initSectionCase(_sections);
        p->initCircleCase(_circles);
        p->initArcCase(_arcs);
    }
}

//Scene::Scene(Scene &&) {
//}

//Scene& Scene::operator=(Scene &&) {
//}

Scene::~Scene() {
    std::unordered_set<Point*> pointsWithoutDub;
    for (auto& pair: _points) {
        pointsWithoutDub.insert(pair.second);
    }
    for (auto& p: pointsWithoutDub) {
        delete p;
    }
    pointsWithoutDub.clear();
    for (auto& pair: _sections) {
        delete pair.second;
    }
    for (auto& pair: _circles) {
        delete pair.second;
    }
    for (auto& pair: _arcs) {
        delete pair.second;
    }
    _points.clear();
    _sections.clear();
    _circles.clear();
    _arcs.clear();
    _requirements.clear();

    _graph = UndirectedWeightedGraph<ID, ID>{};

    _components.clear();
    _idToComponent.clear();
}

ID Scene::addObject(const ObjectData& objData) {
    switch (objData.et) {
        case ObjType::ET_POINT: {
            if (objData.params.size() < 2) {
                throw std::invalid_argument("Point requires 2 coordinates");
            }

            ID newID = _idGeometricObjectsGenerator.generate();

            addPoint(objData, newID);

            std::vector<const double*> vec = getPointParams(newID);
            if (_observer) {
                _observer->pointAdded(newID, vec[0], vec[1]);
            }

            return newID;
        }
        case ObjType::ET_SECTION: {
            if (objData.params.size() < 4) {
                throw std::invalid_argument("Section requires 4 coordinates");
            }

            ID pID1 = _idGeometricObjectsGenerator.generate();
            ID pID2 = _idGeometricObjectsGenerator.generate();
            ID newID = _idGeometricObjectsGenerator.generate();

            _objectSubObjects[newID].push_back(pID1);
            _objectSubObjects[newID].push_back(pID2);

            addSection(objData, pID1, pID2, newID);

            std::vector<const double*> vec = getSectionParams(newID);
            if (_observer) {
                _observer->sectionAdded(newID, vec[0], vec[1], vec[2], vec[3]);
            }

            return newID;
        }
        case ObjType::ET_CIRCLE: {
            if (objData.params.size() < 3) {
                throw std::invalid_argument("Circle requires center and radius");
            }

            ID pID = _idGeometricObjectsGenerator.generate();
            ID newID = _idGeometricObjectsGenerator.generate();

            _objectSubObjects[newID].push_back(pID);

            addCircle(objData, pID, newID);

            std::vector<const double*> vec = getCircleParams(newID);
            if (_observer) {
                _observer->circleAdded(newID, vec[0], vec[1], vec[2]);
            }

            return newID;
        }
        case ObjType::ET_ARC: {
            if (objData.params.size() < 5) {
                throw std::invalid_argument("Arc requires first point, second point, center and radius");
            }


            ID pID1 = _idGeometricObjectsGenerator.generate();
            ID pID2 = _idGeometricObjectsGenerator.generate();
            ID pID3 = _idGeometricObjectsGenerator.generate();
            ID newID = _idGeometricObjectsGenerator.generate();

            _objectSubObjects[newID].push_back(pID1);
            _objectSubObjects[newID].push_back(pID2);
            _objectSubObjects[newID].push_back(pID3);

            addArc(objData, pID1, pID2, pID3, newID);

            std::vector<const double*> vec = getArcParams(newID);
            if (_observer) {
                _observer->arcAdded(newID, vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);
            }

            return newID;
        }
        default:
            throw std::invalid_argument("Unknown object type");
    }
}

void Scene::addPoint(ObjectData data, ID id) {
    Point* p = new Point(data.params[0], data.params[1]);
    _points[id] = p;
    _graph.addVertex(id);
    _allFiguresRectangle = _allFiguresRectangle | getBox(*p);
    _isComponentsDirty = true;
}

void Scene::addSection(ObjectData data, ID pID1, ID pID2, ID sectionID) {
    Point* p1 = new Point(data.params[0], data.params[1]);
    _points[pID1] = p1;
    _graph.addVertex(pID1);


    Point* p2 = new Point(data.params[2], data.params[3]);
    _points[pID2] = p2;
    _graph.addVertex(pID2);


    Section* s = new Section(p1, p2);
    _sections[sectionID] = s;
    _graph.addVertex(sectionID);

    _graph.addEdge(pID1, sectionID, _connectionEdgeID);
    _graph.addEdge(pID2, sectionID, _connectionEdgeID);

    _allFiguresRectangle = _allFiguresRectangle | getBox(*s);
    _isComponentsDirty = true;
}

void Scene::addCircle(ObjectData data, ID pointID, ID circleID) {
    Point* p = new Point(data.params[0], data.params[1]);
    _points[pointID] = p;
    _graph.addVertex(pointID);

    Circle* c = new Circle(p, data.params[2]);
    _circles[circleID] = c;
    _graph.addVertex(circleID);

    _graph.addEdge(pointID, circleID, _connectionEdgeID);

    _allFiguresRectangle = _allFiguresRectangle | getBox(*c);
    _isComponentsDirty = true;
}

void Scene::addArc(ObjectData data, ID pointID1, ID pointID2, ID pointID3, ID arcID) {
    Point* p1 = new Point(data.params[0], data.params[1]);
    Point* p2 = new Point(data.params[2], data.params[3]);
    Point* p3 = new Point(data.params[4], data.params[5]);

    _points[pointID1] = p1;
    _graph.addVertex(pointID1);

    _points[pointID2] = p2;
    _graph.addVertex(pointID2);

    _points[pointID3] = p3;
    _graph.addVertex(pointID3);

    Arc* a = new Arc(p1, p2, p3);
    _arcs[arcID] = a;
    _graph.addVertex(arcID);

    _graph.addEdge(pointID1, arcID, _connectionEdgeID);
    _graph.addEdge(pointID2, arcID, _connectionEdgeID);
    _graph.addEdge(pointID3, arcID, _connectionEdgeID);
    _allFiguresRectangle = _allFiguresRectangle | getBox(*a);
    _isComponentsDirty = true;

    // Накладываем требование, что центр арки находится на серединном перпендикуляре
    Requirement reqData;
    reqData.type = ReqType::ET_ARCCENTERONPERPENDICULAR;
    reqData.obj1 = pointID1;
    reqData.obj2 = pointID2;
    reqData.obj3 = pointID3;
    addRequirement(reqData);
}

bool Scene::deleteObject(ID objectID) {
    return
            deletePoint(objectID)
            ||
            deleteSection(objectID)
            ||
            deleteCircle(objectID)
            ||
            deleteArc(objectID);
}

bool Scene::deletePoint(ID pointID) {
    auto it_point = _points.find(pointID);
    if (it_point == _points.end()) {
        return false;
    }

    std::vector<Edge<ID, ID>> pointEdges = _graph.getVertexEdges(pointID);
    for (auto& pointEdge: pointEdges) {
        if (pointEdge.weight == Scene::_connectionEdgeID) {
            if (pointEdge.from == pointID) {
                return deleteObject(pointEdge.to);
            } else if (pointEdge.to == pointID) {
                return deleteObject(pointEdge.from);
            }
        }
    }

    bool deleting = true;
    for (auto& pointEdge: pointEdges) {
        if (_requirements[pointEdge.weight].type == ReqType::ET_POINTONPOINT) {
            deleting = false;
        }
        _requirements.erase(pointEdge.weight);
        _idDeletedRequirements.insert(pointEdge.weight);
    }
    if (deleting) {
        delete it_point->second;
    }
    _points.erase(it_point);
    _graph.removeVertex(pointID);

    _idDeletedGeometricObject.insert(pointID);

    _isRectangleDirty = true;
    _isComponentsDirty = true;
    return true;
}

bool Scene::deleteSection(ID sectionID) {
    auto it_section = _sections.find(sectionID);
    if (it_section == _sections.end()) {
        return false;
    }

    std::vector<Edge<ID, ID>> sectionEdges = _graph.getVertexEdges(sectionID);
    for (auto& sectionEdge: sectionEdges) {
        if (sectionEdge.weight == Scene::_connectionEdgeID) {
            ID pointID;
            if (sectionEdge.from == sectionID) {
                pointID = sectionEdge.to;
            } else if (sectionEdge.to == sectionID) {
                pointID = sectionEdge.from;
            }

            bool deleting = true;
            std::vector<Edge<ID, ID>> pointEdges = _graph.getVertexEdges(pointID);
            for (auto& pointEdge: pointEdges) {
                if (pointEdge.weight != Scene::_connectionEdgeID) {
                    if (_requirements[pointEdge.weight].type == ReqType::ET_POINTONPOINT) {
                        deleting = false;
                    }
                    _requirements.erase(pointEdge.weight);
                    _idDeletedRequirements.insert(pointEdge.weight);
                }
            }

            if (deleting) {
                delete _points[pointID];
            }
            _points.erase(pointID);
            _graph.removeVertex(pointID);
            _idDeletedGeometricObject.insert(pointID);
        } else {
            _requirements.erase(sectionEdge.weight);
            _idDeletedRequirements.insert(sectionEdge.weight);
        }
    }

    delete it_section->second;
    _sections.erase(it_section);
    _graph.removeVertex(sectionID);

    _idDeletedGeometricObject.insert(sectionID);

    _isRectangleDirty = true;
    _isComponentsDirty = true;
    return true;
}

bool Scene::deleteCircle(ID circleID) {
    auto it_circle = _circles.find(circleID);
    if (it_circle == _circles.end()) {
        return false;
    }

    std::vector<Edge<ID, ID>> circleEdges = _graph.getVertexEdges(circleID);
    for (auto& circleEdge: circleEdges) {
        if (circleEdge.weight == Scene::_connectionEdgeID) {
            ID pointID;
            if (circleEdge.from == circleID) {
                pointID = circleEdge.to;
            } else if (circleEdge.to == circleID) {
                pointID = circleEdge.from;
            }

            bool deleting = true;
            std::vector<Edge<ID, ID>> pointEdges = _graph.getVertexEdges(pointID);
            for (auto& pointEdge: pointEdges) {
                if (pointEdge.weight != Scene::_connectionEdgeID) {
                    if (_requirements[pointEdge.weight].type == ReqType::ET_POINTONPOINT) {
                        deleting = false;
                    }
                    _requirements.erase(pointEdge.weight);
                    _idDeletedRequirements.insert(pointEdge.weight);
                }
            }

            if (deleting) {
                delete _points[pointID];
            }
            _points.erase(pointID);
            _graph.removeVertex(pointID);
            _idDeletedGeometricObject.insert(pointID);
        } else {
            _requirements.erase(circleEdge.weight);
            _idDeletedRequirements.insert(circleEdge.weight);
        }
    }

    delete it_circle->second;
    _circles.erase(it_circle);
    _graph.removeVertex(circleID);

    _idDeletedGeometricObject.insert(circleID);

    _isRectangleDirty = true;
    _isComponentsDirty = true;
    return true;
}

bool Scene::deleteArc(ID arcID) {
    auto it_arc = _arcs.find(arcID);
    if (it_arc != _arcs.end()) {
        return false;
    }

    std::vector<Edge<ID, ID>> arcEdges = _graph.getVertexEdges(arcID);
    for (auto& arcEdge: arcEdges) {
        if (arcEdge.weight == Scene::_connectionEdgeID) {
            ID pointID;
            if (arcEdge.from == arcID) {
                pointID = arcEdge.to;
            } else if (arcEdge.to == arcID) {
                pointID = arcEdge.from;
            }

            bool deleting = true;
            std::vector<Edge<ID, ID>> pointEdges = _graph.getVertexEdges(pointID);
            for (auto& pointEdge: pointEdges) {
                if (pointEdge.weight != Scene::_connectionEdgeID) {
                    if (_requirements[pointEdge.weight].type == ReqType::ET_POINTONPOINT) {
                        deleting = false;
                    }
                    _requirements.erase(pointEdge.weight);
                    _idDeletedRequirements.insert(pointEdge.weight);
                }
            }

            if (deleting) {
                delete _points[pointID];
            }
            _points.erase(pointID);
            _graph.removeVertex(pointID);
            _idDeletedGeometricObject.insert(pointID);
        } else {
            _requirements.erase(arcEdge.weight);
            _idDeletedRequirements.insert(arcEdge.weight);
        }
    }
    delete it_arc->second;
    _arcs.erase(it_arc);
    _graph.removeVertex(arcID);

    _idDeletedGeometricObject.insert(arcID);

    _isRectangleDirty = true;
    _isComponentsDirty = true;
    return true;
}

void Scene::clear() {
    std::unordered_set<Point*> pointsWithoutDub;
    for (auto& pair: _points) {
        pointsWithoutDub.insert(pair.second);
    }
    for (auto& p: pointsWithoutDub) {
        delete p;
    }
    for (auto& pair: _sections) {
        delete pair.second;
    }
    for (auto& pair: _circles) {
        delete pair.second;
    }
    for (auto& pair: _arcs) {
        delete pair.second;
    }
    _points.clear();
    _sections.clear();
    _circles.clear();
    _arcs.clear();

    _isComponentsDirty = false;
    _components.clear();
    _idToComponent.clear();

    _graph = UndirectedWeightedGraph<ID, ID>{};

    _idDeletedGeometricObject.clear();

    _allFiguresRectangle = BoundBox2D{};
    _isRectangleDirty = false;
    _idGeometricObjectsGenerator.reset();
    _idRequirementsGenerator.reset();
}

const BoundBox2D& Scene::getBoundingBox() const {
    if (_isRectangleDirty) {
        updateBoundingBox();
    }
    return _allFiguresRectangle;
}

void Scene::updateBoundingBox() const {
    _allFiguresRectangle = BoundBox2D{};
    if (_points.empty()) {
        _isRectangleDirty = false;
        return;
    }
    auto it = _points.begin();
    _allFiguresRectangle = getBox(*it->second);
    for (++it; it != _points.end(); ++it) {
        _allFiguresRectangle = _allFiguresRectangle | getBox(*it->second);
    }
    _isRectangleDirty = false;
}

void Scene::paint() const {
    updateBoundingBox();
    _painter->getBoundBox(_allFiguresRectangle);
    _painter->draw();
}

void Scene::clearImage() const {
    _painter->clear();
}

ObjectData Scene::getObjectData(ID id) const {
    ObjectData obj;
    if (auto it = _points.find(id); it != _points.end()) {
        obj.et = ObjType::ET_POINT;
        obj.params = {
                it->second->x,
                it->second->y
        };
        obj.id = id;
        return obj;
    }
    if (auto it = _sections.find(id); it != _sections.end()) {
        obj.et = ObjType::ET_SECTION;
        obj.params = {
                it->second->beg->x,
                it->second->beg->y,
                it->second->end->x,
                it->second->end->y
        };
        obj.subObjects = _objectSubObjects.at(id);
        obj.id = id;
        return obj;
    }
    if (auto it = _circles.find(id); it != _circles.end()) {
        obj.et = ObjType::ET_CIRCLE;
        obj.params = {
                it->second->center->x,
                it->second->center->y,
                it->second->r
        };
        obj.subObjects = _objectSubObjects.at(id);
        obj.id = id;
        return obj;
    }
    if (auto it = _arcs.find(id); it != _arcs.end()) {
        obj.et = ObjType::ET_ARC;
        obj.params = {
                it->second->beg->x,
                it->second->beg->y,
                it->second->end->x,
                it->second->end->y,
                it->second->center->x,
                it->second->center->y,
        };
        obj.subObjects = _objectSubObjects.at(id);
        obj.id = id;
        return obj;
    }
    throw std::invalid_argument("Object not found");
}

bool Scene::hasObject(ID id) const {
    if (auto it = _points.find(id); it != _points.end()) {
        return true;
    }
    if (auto it = _sections.find(id); it != _sections.end()) {
        return true;
    }
    if (auto it = _circles.find(id); it != _circles.end()) {
        return true;
    }
    if (auto it = _arcs.find(id); it != _arcs.end()) {
        return true;
    }
    return false;
}

bool Scene::hasRequirement(ID id) const {
    if (auto it = _requirements.find(id); it != _requirements.end()) {
        return true;
    }
    return false;
}

ObjectData Scene::getRootObjectData(ID id) const {
    if (_sections.contains(id) || _circles.contains(id) || _arcs.contains(id)) {
        return getObjectData(id);
    }
    if (!_points.contains(id)) {
        throw std::invalid_argument("Point not found");
    }
    std::vector<Edge<ID, ID>> objectEdges = _graph.getVertexEdges(id);
    for (auto& edge: objectEdges) {
        if (edge.weight == Scene::_connectionEdgeID) {
            if (edge.from == id) {
                return getObjectData(edge.to);
            } else if (edge.to == id) {
                return getObjectData(edge.from);
            }
        }
    }
    auto it = _points.find(id);
    ObjectData obj;
    obj.et = ObjType::ET_POINT;
    obj.params = {
            it->second->x,
            it->second->y
    };
    obj.id = id;
    return obj;
}

Requirement Scene::getRequirementData(ID object1, ID object2) const {
    if (!_points.contains(object1) && !_sections.contains(object1) && !_circles.contains(object1) &&
        !_arcs.contains(object1)) {
        throw std::invalid_argument("Objects not found");
    }
    if (!_points.contains(object2) && !_sections.contains(object2) && !_circles.contains(object2) &&
        !_arcs.contains(object2)) {
        throw std::invalid_argument("Objects not found");
    }
    ID reqID = _graph.getEdgeWeight(object1, object2);
    Requirement req = _requirements.at(reqID);
    return req;
}

std::size_t Scene::objectsCount() const {
    return _graph.vertexCount();
}

std::size_t Scene::requirementsCount() const {
    return _graph.edgeCount();
}

std::vector<ObjectData> Scene::getObjects() const {
    std::vector<ObjectData> objs;
    std::unordered_set<ID> alreadyPoints;
    for (auto& [id, s]: _sections) {
        ObjectData obj = getObjectData(id);
        objs.push_back(obj);
        alreadyPoints.insert(obj.subObjects.at(0));
        alreadyPoints.insert(obj.subObjects.at(1));
    }
    for (auto& [id, c]: _circles) {
        ObjectData obj = getObjectData(id);
        objs.push_back(obj);
        alreadyPoints.insert(obj.subObjects.at(0));
    }
    for (auto& [id, a]: _arcs) {
        ObjectData obj = getObjectData(id);
        objs.push_back(obj);
        alreadyPoints.insert(obj.subObjects.at(0));
        alreadyPoints.insert(obj.subObjects.at(1));
        alreadyPoints.insert(obj.subObjects.at(2));
    }
    for (auto& [id, _]: _points) {
        if (!alreadyPoints.contains(id)) {
            objs.push_back(getObjectData(id));
        }
    }
    return objs;
}

std::vector<Requirement> Scene::getRequirements() const {
    std::vector<Requirement> reqs;
    for (auto& [id, _]: _requirements) {
        reqs.push_back(getRequirementData(id));
    }
    return reqs;
}

std::vector<Requirement> Scene::getObjectRequirements(ID objectID) const {
    if (!_points.contains(objectID) && !_sections.contains(objectID) && !_circles.contains(objectID) &&
        !_arcs.contains(objectID)) {
        throw std::invalid_argument("Objects not found");
    }
    std::vector<Edge<ID, ID>> objReqsIds = _graph.getVertexEdges(objectID);
    std::vector<Requirement> objReqs;
    for (auto& edge: objReqsIds) {
        if (edge.weight != Scene::_connectionEdgeID) {
            objReqs.push_back(getRequirementData(edge.weight));
        }
    }
    return objReqs;
}

std::vector<Requirement> Scene::getObjectRequirementsWithConnectedObjects(ID objectID) const {
    std::unordered_set<Requirement> objectRequirements;
    if (_points.contains(objectID)) {
        std::vector<Edge<ID, ID>> objectEdges = _graph.getVertexEdges(objectID);
        for (auto& edge: objectEdges) {
            if (edge.weight == Scene::_connectionEdgeID) {
                if (edge.from == objectID) {
                    std::vector<Requirement> data = getObjectRequirementsWithConnectedObjects(edge.to);
                    return {data.begin(), data.end()};
                } else if (edge.to == objectID) {
                    std::vector<Requirement> data = getObjectRequirementsWithConnectedObjects(edge.from);
                    return {data.begin(), data.end()};
                }
            } else {
                objectRequirements.insert(getRequirementData(edge.weight));
            }
        }
    } else if (_sections.contains(objectID) || _circles.contains(objectID) || _arcs.contains(objectID)) {
        std::vector<Edge<ID, ID>> objectEdges = _graph.getVertexEdges(objectID);
        for (auto& edge: objectEdges) {
            if (edge.weight == Scene::_connectionEdgeID) {
                std::vector<Requirement> data;
                if (edge.from == objectID) {
                    data = getObjectRequirements(edge.to);
                    objectRequirements.insert(data.begin(), data.end());
                } else if (edge.to == objectID) {
                    data = getObjectRequirements(edge.from);
                    objectRequirements.insert(data.begin(), data.end());
                }
            } else {
                objectRequirements.insert(getRequirementData(edge.weight));
            }
        }
    } else {
        throw std::invalid_argument("Objects not found");
    }

    return {objectRequirements.begin(), objectRequirements.end()};
}

void Scene::setPainter(Painter* p) {
    _painter = p;
    _painter->initPointCase(_points);
    _painter->initSectionCase(_sections);
    _painter->initCircleCase(_circles);
    _painter->initArcCase(_arcs);
}

void Scene::moveObject(ID id, double dx, double dy) {
    if (_points.contains(id)) {
        Point* p = _points[id];
        p->x += dx;
        p->y += dy;
    } else if (_sections.contains(id)) {
        Section* s = _sections[id];
        s->beg->x += dx;
        s->beg->y += dy;
        s->end->x += dx;
        s->end->x += dy;
    } else if (_circles.contains(id)) {
        Circle* c = _circles[id];
        c->center->x += dx;
        c->center->y += dy;
    } else {
        throw std::runtime_error("Failed to move object");
    }
    _isRectangleDirty = true;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(id);
    std::cout << "Move object" << '\n';
}

void Scene::movePoint(ID pointID, double dx, double dy) {
    if (!_points.contains(pointID)) {
        throw std::invalid_argument("There is no point to move");
    }
    Point* p = _points[pointID];
    p->x += dx;
    p->y += dy;
    _isRectangleDirty = true;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(pointID);
}

void Scene::moveSection(ID sectionID, double dx, double dy) {
    if (dx == 0 && dy == 0) {
        return;
    }
    if (!_sections.contains(sectionID)) {
        throw std::invalid_argument("There is no section to move");
    }
    Section* s = _sections[sectionID];
    s->beg->x += dx;
    s->beg->y += dy;
    s->end->x += dx;
    s->end->y += dy;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(sectionID);
    _isRectangleDirty = true;
}

void Scene::moveCircle(ID circleID, double dx, double dy) {
    if (dx == 0 && dy == 0) {
        return;
    }
    if (!_circles.contains(circleID)) {
        throw std::invalid_argument("There is no circle to move");
    }
    Circle* c = _circles[circleID];
    c->center->x += dx;
    c->center->y += dy;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(circleID);
}

void Scene::moveArc(ID arcID, double dx, double dy) {
    if (_arcs.contains(arcID)) {
        Arc* a = _arcs[arcID];
        a->beg->x += dx;
        a->beg->y += dy;
        a->end->x += dx;
        a->end->y += dy;
        a->center->x += dx;
        a->center->y += dy;
    }
    _isRectangleDirty = true;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(arcID);
}

void Scene::setPoint(ID pointID, double x, double y, const bool updateRequirementFlag) {
    if (!_points.contains(pointID)) {
        throw std::out_of_range("There is no point to change position");
    }
    Point* p = _points[pointID];
    if (p->x != x || p->y != y) {
        p->x = x;
        p->y = y;
        if (updateRequirementFlag) {
            Component& c = findComponentByID(pointID);
            std::vector<Variable*> target;
            std::vector<Variable*>& vars = c._vars;
            for (auto it = vars.begin(); it != vars.end();) {
                Variable* var = *it;
                if (var->value == &p->x || var->value == &p->y) {
                    target.push_back(var);
                    it = vars.erase(it);
                } else {
                    ++it;
                }
            }
            updateRequirements(pointID);
            vars.insert(vars.end(), target.begin(), target.end());
        }
        _isRectangleDirty = true;
    }
}

void Scene::setSection(ID sectionID, double x1, double y1, double x2, double y2, const bool updateRequirementFlag) {
    if (!_sections.contains(sectionID)) {
        throw std::out_of_range("There is no section to change");
    }
    Section* s = _sections[sectionID];
    if (s->beg->x != x1 || s->beg->y != y1 || s->beg->x != x2 || s->beg->x != y2) {
        s->beg->x = x1;
        s->beg->y = y1;
        s->end->x = x2;
        s->end->y = y2;
        if (updateRequirementFlag) {
            updateRequirements(sectionID);
        }
    }
}

void Scene::setCircle(ID circleID, double x, double y, double r, const bool updateRequirementFlag) {
    if (!_circles.contains(circleID)) {
        throw std::out_of_range("There is no circle to change");
    }
    Circle* c = _circles[circleID];
    if (c->center->x != x || c->center->y != y || c->r != r) {
        c->center->x = x;
        c->center->y = y;
        c->r = r;
        if (updateRequirementFlag) {
            updateRequirements(circleID);
        }
    }
}

void Scene::setArc(ID arcID, double x0, double y0, double x1, double y1, double x2, double y2, double r,
                   const bool updateRequirementFlag) {
    if (!_arcs.contains(arcID)) {
        throw std::out_of_range("There is no arc to change");
    }
    Arc* a = _arcs[arcID];
    a->beg->x = x0;
    a->beg->y = y0;
    a->end->x = x1;
    a->end->y = y1;
    a->center->x = x2;
    a->center->y = y2;
    if (updateRequirementFlag) {
        updateRequirements(arcID);
    }
}

std::vector<const double*> Scene::getPointParams(ID pointID) const {
    if (!_points.contains(pointID)) {
        throw std::out_of_range("There is no point");
    }
    const Point* p = _points.at(pointID);
    return {&p->x, &p->y};
}

std::vector<const double*> Scene::getSectionParams(ID sectionID) const {
    if (!_sections.contains(sectionID)) {
        throw std::out_of_range("There is no section");
    }
    const Section* s = _sections.at(sectionID);
    return {&s->beg->x, &s->beg->y, &s->end->x, &s->end->y};
}

std::vector<const double*> Scene::getCircleParams(ID circleID) const {
    if (!_circles.contains(circleID)) {
        throw std::out_of_range("There is no circle");
    }
    const Circle* c = _circles.at(circleID);
    return {&c->center->x, &c->center->y, &c->r};
}

std::vector<const double*> Scene::getArcParams(ID arcID) const {
    if (!_arcs.contains(arcID)) {
        throw std::out_of_range("There is no arc");
    }
    const Arc* a = _arcs.at(arcID);
    return {
            &a->beg->x, &a->beg->y,
            &a->end->x, &a->end->y,
            &a->center->x, &a->center->y
    };
}

ID Scene::addRequirement(const Requirement& reqData, const bool updateRequirementFlag) {
    ID newID = _idRequirementsGenerator.generate();

    addRequirement(reqData, newID);

    if (updateRequirementFlag) {
        updateRequirements(reqData.obj1);
    }

    if (_observer) {
        // TODO
        //_observer->reqAdded(reqData);
    }

    return newID;
}

void Scene::addRequirement(const Requirement& reqData, ID reqID) {
    if (!isValid(reqData)) {
        throw std::invalid_argument("Invalid requirement data");
    }

    if (reqData.type == ReqType::ET_POINTONPOINT) {
        Point* p1 = _points[reqData.obj1];
        Point* p2 = _points[reqData.obj2];

        Requirement temp;
        temp.type = ReqType::ET_POINTPOINTDIST;
        temp.obj1 = reqData.obj1;
        temp.obj2 = reqData.obj2;
        temp.param = 0.;
        ID tempID(-5);

        _requirements[tempID] = temp;
        _graph.addEdge(reqData.obj1, reqData.obj2, tempID);
        updateRequirements(reqData.obj1);

        _graph.removeEdge(reqData.obj1, reqData.obj2);
        _requirements.erase(tempID);

        delete p1;
        for (auto& [id, p] : _points) {
            if (p == p1) {
                p = p2;
            }
        }
        for (auto& [id, sec] : _sections) {
            if (sec->beg == p1) {
                sec->beg = p2;
            }
            if (sec->end == p1) {
                sec->end = p2;
            }
        }
        for (auto& [id, circle] : _circles) {
            if (circle->center == p1) {
                circle->center = p2;
            }
        }
        for (auto& [id, arc] : _arcs) {
            if (arc->beg == p1) {
                arc->beg = p2;
            }
            if (arc->end == p1) {
                arc->end = p2;
            }
            if (arc->center == p1) {
                arc->center = p2;
            }
        }
    }

    Requirement req = reqData;
    req.id = reqID;
    _graph.addEdge(reqData.obj1, reqData.obj2, reqID);
    _requirements[reqID] = req;
    _requirementToObjects[reqID].insert(reqData.obj1);
    _requirementToObjects[reqID].insert(reqData.obj2);
    if (reqData.obj3 != ID(0)) {
        _requirementToObjects[reqID].insert(reqData.obj3);
    }
    _isComponentsDirty = true;
}

Component& Scene::findComponentByID(ID id) {
    if (_isComponentsDirty) {
        rebuildComponents();
    }
    auto it = _idToComponent.find(id);
    if (it == _idToComponent.end()) {
        throw std::runtime_error("ID not found in any component");
    }
    return _components[it->second];
}

std::vector<Variable*> Scene::getVariables(const Requirement& req) const {
    std::vector<Variable*> res;

    auto f = [&](std::vector<Variable*>& res, ID obj) {
        if (_points.contains(obj)) {
            res.emplace_back(new Variable(&_points.at(obj)->x));
            res.emplace_back(new Variable(&_points.at(obj)->y));
        }
        else if (_sections.contains(obj)) {
            res.emplace_back(new Variable(&_sections.at(obj)->beg->x));
            res.emplace_back(new Variable(&_sections.at(obj)->beg->y));
            res.emplace_back(new Variable(&_sections.at(obj)->end->x));
            res.emplace_back(new Variable(&_sections.at(obj)->end->y));
        }
        if (_circles.contains(obj)) {
            res.emplace_back(new Variable(&_circles.at(obj)->center->x));
            res.emplace_back(new Variable(&_circles.at(obj)->center->y));
            res.emplace_back(new Variable(&_circles.at(obj)->r));
        }
        if (_arcs.contains(obj)) {
            res.emplace_back(new Variable(&_arcs.at(obj)->beg->x));
            res.emplace_back(new Variable(&_arcs.at(obj)->beg->y));
            res.emplace_back(new Variable(&_arcs.at(obj)->end->x));
            res.emplace_back(new Variable(&_arcs.at(obj)->end->y));
            res.emplace_back(new Variable(&_arcs.at(obj)->center->x));
            res.emplace_back(new Variable(&_arcs.at(obj)->center->y));
        }
    };

    f(res, req.obj1);
    f(res, req.obj2);
    if (req.obj3 != ID(0)) {
        f(res, req.obj3);
    }

    return res;
}

Function* Scene::getFunction(const Requirement& req) {
    ID id1 = req.obj1;
    ID id2 = req.obj2;

    Function* err = nullptr;
    switch (req.type) {
        case ReqType::ET_POINTSECTIONDIST: {
            err = new PointSectionDistanceError(getVariables(req), req.param.value());
            break;
        }
        case ReqType::ET_POINTONSECTION: {
            err = new PointOnSectionError(getVariables(req));
            break;
        }
        case ReqType::ET_POINTPOINTDIST: {
            err = new PointPointDistanceError(getVariables(req), req.param.value());
            break;
        }
        case ReqType::ET_POINTONPOINT: {
            if (!_points.contains(id1) || !_points.contains(id2)) {
                throw std::invalid_argument("Objects must be two Point");
            }

            // Same points ptr
            if (_points[id1] == _points[id2]) {
                break;
            }

            Point* p1 = _points.at(id1);
            Point* p2 = _points.at(id2);

            delete p1;
            for (auto& [id, p]: _points) {
                if (p == p1) {
                    p = p2;
                }
            }

            err = new PointOnPointError(getVariables(req));
            break;
        }
        case ReqType::ET_SECTIONCIRCLEDIST: {
            err = new SectionCircleDistanceError(getVariables(req), req.param.value());
            break;
        }
        case ReqType::ET_SECTIONONCIRCLE: {
            err = new SectionOnCircleError(getVariables(req));
            break;
        }
        case ReqType::ET_SECTIONINCIRCLE: {
            err = new SectionInCircleError(getVariables(req));
            break;
        }
        case ReqType::ET_SECTIONSECTIONPARALLEL: {
            err = new SectionSectionParallelError(getVariables(req));
            break;
        }
        case ReqType::ET_SECTIONSECTIONPERPENDICULAR: {
            err = new SectionSectionPerpendicularError(getVariables(req));
            break;
        }
        case ReqType::ET_SECTIONSECTIONANGLE: {
            err = new SectionSectionAngleError(getVariables(req), req.param.value());
            break;
        }
        case ReqType::ET_ARCCENTERONPERPENDICULAR: {
            err = new ArcCenterOnPerpendicularError(getVariables(req));
            break;
        }
        case ReqType::ET_HORIZONTAL: {
            err = new HorizontalError(getVariables(req));
            break;
        }
        case ReqType::ET_VERTICAL: {
            err = new VerticalError(getVariables(req));
            break;
        }
        default: {
            throw std::invalid_argument("Unknown requirement type");
        }
    }
    return err;
}

void Scene::rebuildComponents() {
    _components.clear();
    _idToComponent.clear();
    std::vector<std::vector<ID>> allComponents = _graph.connectedComponents();
    for (std::size_t i = 0; auto& vec: allComponents) {
        for (auto& vertexID: vec) {
            _idToComponent[vertexID] = i;
        }
        ++i;
        Graph<ID, ID, UndirectedPolicy, WeightedPolicy> component = _graph.subGraph(vec);
        std::vector<Edge<ID, ID>> componentEdges = component.getAllEdges();
        std::vector<ID> componentVertices = component.getVertices();
        Component c;

        std::vector<Variable*> variables;
        std::unordered_set<double*> seen;

        for (auto& edge: componentEdges) {
            if (edge.weight != Scene::_connectionEdgeID && _requirements[edge.weight].type != ReqType::ET_POINTONPOINT) {
                std::vector<Variable*> vars = getVariables(_requirements[edge.weight]);
                for (Variable* var : vars) {
                    if (seen.insert(var->value).second) {
                        variables.push_back(var);
                    }
                    else {
                        delete var;
                    }
                }
                c._errorFunctions.push_back(getFunction(_requirements[edge.weight]));
            }
        }

        c._vars = variables;
        if (!componentVertices.empty()) {
            std::vector<ID> cC = _graph.findConnectedComponent(componentVertices[0]);
            c._objectIDs = std::unordered_set(cC.begin(), cC.end());
        }
        c._task = new LSMFORLMTask(c._errorFunctions, c._vars);
        _components.push_back(std::move(c));
    }
    _isComponentsDirty = false;
}

void Scene::updateRequirements(ID id) {
    Component& component = findComponentByID(id);

    std::cout << "Objects in component:  " << component._objectIDs.size() << '\n';

    if (component.noErrorFunction()) {
        return;
    }

    Task* task = new LSMFORLMTask(component._errorFunctions, component._vars);
    LMSparse _solver;
    _solver.setTask(task);
    _solver.optimize();

    // Check converging
    if (!_solver.isConverged() || _solver.getCurrentError() > 1e-3) {
        delete task;
        throw std::runtime_error("Not converged");
    }
    delete task;
}

Requirement Scene::getRequirementData(ID reqID) const {
    if (!_requirements.contains(reqID) || !_requirementToObjects.contains(reqID) || reqID == Scene::_connectionEdgeID) {
        throw std::invalid_argument("Requirement not found");
    }
    return _requirements.at(reqID);
}

std::vector<Requirement> Scene::getAllRequirementsData() const {
    std::vector<Requirement> vec;
    vec.reserve(_requirements.size());
    for (auto const& kv : _requirements) {
        vec.push_back(kv.second);
    }
    return vec;
}

bool Scene::deleteRequirement(ID reqID) {
    if (!_requirements.contains(reqID)) {
        return false;
    }

    std::unordered_set<ID> reqObjects = _requirementToObjects.at(reqID);
    auto it = reqObjects.begin();
    ID first = *it;
    ID second = *(++it);
    _graph.removeEdge(first, second);

    Requirement req = _requirements[reqID];
    _requirements.erase(reqID);
    if (req.type == ReqType::ET_POINTONPOINT) {
        Point* p1 = _points[first];
        Point* pNew = new Point(p1->x, p1->y);

        std::unordered_map<ID, Requirement> reqs = _requirements;
        std::vector<ID> component = _graph.findComponentByEdgeType(first, [&reqs](const Edge<ID, ID>& edge) {
            return edge.weight != ID(-2) && reqs[edge.weight].type == ReqType::ET_POINTONPOINT;
        });

        for (auto& obj: component) {
            // objects
            if (_points.contains(obj)) {
                _points[obj] = pNew;
            }
            for (auto& [id, vec]: _objectSubObjects) {
                for (auto& subID: vec) {
                    if (subID == obj) {
                        if (_sections.contains(id)) {
                            Section* s = _sections[id];
                            if (s->beg == p1) {
                                s->beg = pNew;
                            }
                            if (s->end == p1) {
                                s->end = pNew;
                            }
                        } else if (_circles.contains(id)) {
                            Circle* c = _circles[id];
                            if (c->center == p1) {
                                c->center = pNew;
                            }
                        } else if (_arcs.contains(id)) {
                            Arc* a = _arcs[id];
                            if (a->beg == p1) {
                                a->beg = pNew;
                            }
                            if (a->end == p1) {
                                a->end = pNew;
                            }
                            if (a->center == p1) {
                                a->center = pNew;
                            }
                        }
                    }
                }
            }
        }
    }

    _idDeletedRequirements.insert(reqID);
    _isComponentsDirty = true;
    _isRectangleDirty = true;
    return true;
}

bool Scene::tryRestoreObject(const ObjectData& data, ID id) {
    if (!_idDeletedGeometricObject.contains(id)) {
        return false;
    }
    if (data.et == ObjType::ET_POINT) {
        addPoint(data, id);
        _idDeletedGeometricObject.erase(id);
    } else if (data.et == ObjType::ET_SECTION) {
        if (data.subObjects.size() < 2) {
            return false;
        }
        addSection(data, data.subObjects[0], data.subObjects[1], id);
    } else if (data.et == ObjType::ET_CIRCLE) {
        if (data.subObjects.size() < 1) {
            return false;
        }
        addCircle(data, data.subObjects[0], id);
    } else if (data.et == ObjType::ET_ARC) {
        if (data.subObjects.size() < 3) {
            return false;
        }
        addArc(data, data.subObjects[0], data.subObjects[1], data.subObjects[2], id);
    }
    return true;
}

bool Scene::tryRestoreRequirement(const Requirement& data, ID id) {
    if (!_idDeletedRequirements.contains(id)) {
        return false;
    }
    addRequirement(data, id);
    _idDeletedRequirements.erase(id);
    return true;
}

ObjType Scene::getObjType(ID id) const {
    if (_points.contains(id))   return ObjType::ET_POINT;
    if (_sections.contains(id)) return ObjType::ET_SECTION;
    if (_circles.contains(id))  return ObjType::ET_CIRCLE;
    if (_arcs.contains(id))     return ObjType::ET_ARC;
    throw std::out_of_range("Object with given ID does not exist");
}

bool Scene::exists(ID id, ObjType expected) const {
    switch (expected) {
        case ObjType::ET_POINT:   return _points.contains(id);
        case ObjType::ET_SECTION: return _sections.contains(id);
        case ObjType::ET_CIRCLE:  return _circles.contains(id);
        case ObjType::ET_ARC:     return _arcs.contains(id);
    }
    return false;
}

bool Scene::isValid(const Requirement& req) const {
    using u8 = std::underlying_type_t<ReqType>;
    const auto& rule = ReqRules[static_cast<u8>(req.type)];
    const bool aOK = exists(req.obj1, rule.first);
    const bool bOK = exists(req.obj2, rule.second);

    const bool aOK = exists(req.obj1, rule.types[0]);
    const bool bOK = exists(req.obj2, rule.types[1]);

    if (aOK && bOK && (!rule.needsParam || req.param.has_value()) || (aOK && !bOK && (req.type == ReqType::ET_HORIZONTAL || req.type == ReqType::ET_VERTICAL))) {
        return true;
    }

    if (rule.symmetric) {
        const bool revA = exists(req.obj1, rule.types[1]);
        const bool revB = exists(req.obj2, rule.types[0]);
        if (revA && revB && (!rule.needsParam || req.param.has_value())) {
            return true;
        }
    }

    return false;
}

void Scene::setObserver(ISceneObserver* o) {
    _observer = o;
}

void Scene::load(const std::vector<ObjectData>& objs, const std::vector<Requirement>& reqs) {
    for (auto& obj : objs) {
        if (obj.et == ObjType::ET_POINT) {
            addPoint(obj, obj.id);
        }
        else if (obj.et == ObjType::ET_SECTION) {
            addSection(obj, obj.subObjects[0], obj.subObjects[1], obj.id);
        }
        else if (obj.et == ObjType::ET_CIRCLE) {
            addCircle(obj, obj.subObjects[0], obj.id);
        }
        else if (obj.et == ObjType::ET_ARC) {
            addArc(obj, obj.subObjects[0], obj.subObjects[1], obj.subObjects[2], obj.id);
        }
    }

    for (auto& req : reqs) {
        addRequirement(req, req.id);
    }
}




