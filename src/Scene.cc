#include "Scene.h"

Scene::Scene(Painter* p) :
        _painter(p),
        _points(),
        _sections(),
        _circles(),
        _arcs(),
        _requirements(),
        _components(),
        _idToComponent(),
        _isComponentsDirty(false),
        _isRectangleDirty(false),
        _allFiguresRectangle(),
        _graph()
        {
    _points.reserve(std::size_t(128));
    _sections.reserve(std::size_t(128));
    _circles.reserve(std::size_t(128));
    _arcs.reserve(std::size_t(128));
    _requirements.reserve(std::size_t(64));
    if (p) {
        p->initPoint(_points);
        p->initSection(_sections);
        p->initCircle(_circles);
        p->initArc(_arcs);
    }
}

const ID Scene::_errorID(-1);
const ID Scene::_connectionEdgeID(-2);
const ID Scene::_autoID (-3);

Scene::~Scene() {
    std::unordered_set<Point*> pointsWithoutDub;
    for (auto& pair: _points) {
        pointsWithoutDub.insert(pair.second);
    }
    for (auto& p : pointsWithoutDub) {
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
    for (auto& pair: _requirements) {
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

ID Scene::addObject(const ObjectData& objData, ID wantedID) {
    if (wantedID != Scene::_autoID && !_idDeletedGeometricObject.contains(wantedID)) {
        return Scene::_errorID;
    }
    switch (objData.et) {
        case ET_POINT: {
            if (objData.params.size() < 2) {
                throw std::invalid_argument("Point requires 2 coordinates");
            }

            ID newID;
            if (wantedID == Scene::_autoID) {
                newID = _idGeometricObjectsGenerator.generate();
            } else {
                newID = wantedID;
            }

            Point* p = new Point(objData.params[0], objData.params[1]);
            _points[newID] = p;
            _graph.addVertex(newID);

            _allFiguresRectangle = _allFiguresRectangle | p->getBox();
            _isComponentsDirty = true;
            return newID;
        }
        case ET_SECTION: {
            if (objData.params.size() < 4) {
                throw std::invalid_argument("Section requires 4 coordinates");
            }
            Point* p1 = new Point(objData.params[0], objData.params[1]);
            ID pID1 = _idGeometricObjectsGenerator.generate();
            _points[pID1] = p1;
            _graph.addVertex(pID1);

            Point* p2 = new Point(objData.params[2], objData.params[3]);
            ID pID2 = _idGeometricObjectsGenerator.generate();
            _points[pID2] = p2;
            _graph.addVertex(pID2);

            ID newID;
            if (wantedID == Scene::_autoID) {
                newID = _idGeometricObjectsGenerator.generate();
            } else {
                newID = wantedID;
            }

            Section* s = new Section(p1, p2);
            _sections[newID] = s;
            _graph.addVertex(newID);

            _graph.addEdge(pID1, newID, _connectionEdgeID);
            _graph.addEdge(pID2, newID, _connectionEdgeID);

            _allFiguresRectangle = _allFiguresRectangle | s->getBox();
            _isComponentsDirty = true;
            return newID;
        }
        case ET_CIRCLE: {
            if (objData.params.size() < 3) {
                throw std::invalid_argument("Circle requires center and radius");
            }
            Point* p = new Point(objData.params[0], objData.params[1]);
            ID pID = _idGeometricObjectsGenerator.generate();
            _points[pID] = p;
            _graph.addVertex(pID);

            ID newID;
            if (wantedID == Scene::_autoID) {
                newID = _idGeometricObjectsGenerator.generate();
            } else {
                newID = wantedID;
            }

            Circle* c = new Circle(p, objData.params[2]);
            _circles[newID] = c;
            _graph.addVertex(newID);

            _graph.addEdge(pID, newID, _connectionEdgeID);

            _allFiguresRectangle = _allFiguresRectangle | c->getBox();
            _isComponentsDirty = true;
            return newID;
        }
        case ET_ARC: {
            if (objData.params.size() < 5) {
                throw std::invalid_argument("Arc requires first point, second point, center and radius");
            }
            Point* p1 = new Point(objData.params[0], objData.params[1]);
            Point* p2 = new Point(objData.params[2], objData.params[3]);
            Point* p3 = new Point(objData.params[4], objData.params[5]);

            ID pID1 = _idGeometricObjectsGenerator.generate();
            _points[pID1] = p1;
            _graph.addVertex(pID1);

            ID pID2 = _idGeometricObjectsGenerator.generate();
            _points[pID2] = p2;
            _graph.addVertex(pID2);

            ID pID3 = _idGeometricObjectsGenerator.generate();
            _points[pID3] = p3;
            _graph.addVertex(pID3);

            ID newID;
            if (wantedID == Scene::_autoID) {
                newID = _idGeometricObjectsGenerator.generate();
            } else {
                newID = wantedID;
            }

            Arc* a = new Arc(p1, p2, p3, objData.params[6]);
            _arcs[newID] = a;
            _graph.addVertex(newID);

            _graph.addEdge(pID1, newID, _connectionEdgeID);
            _graph.addEdge(pID2, newID, _connectionEdgeID);
            _graph.addEdge(pID3, newID, _connectionEdgeID);
            _allFiguresRectangle = _allFiguresRectangle | a->getBox();
            _isComponentsDirty = true;
            return newID;
        }
        default:
            throw std::invalid_argument("Unknown object type");
    }
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

    bool onlyPoint = true;
    std::vector<Edge<ID, ID>> pointEdges = _graph.getVertexEdges(pointID);
    for (auto& pointEdge : pointEdges) {
        if (pointEdge.weight == Scene::_connectionEdgeID) {
            onlyPoint = false;
            if (pointEdge.from == pointID) {
                deleteObject(pointEdge.to);
            }
            else if (pointEdge.to == pointID) {
                deleteObject(pointEdge.from);
            }
            break;
        }
    }

    if (onlyPoint) {
        bool deleting = true;
        for (auto& pointEdge : pointEdges) {
            if (_requirements[pointEdge.weight]->getType() == ET_POINTONPOINT) {
                deleting = false;
            }
            _requirements.erase(pointEdge.weight);
        }
        if (deleting) {
            delete it_point->second;
        }
        _points.erase(it_point);
        _graph.removeVertex(pointID);
    }

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
            }
            else if (sectionEdge.to == sectionID) {
                pointID = sectionEdge.from;
            }

            bool deleting = true;
            std::vector<Edge<ID, ID>> pointEdges = _graph.getVertexEdges(pointID);
            for (auto& pointEdge : pointEdges) {
                if (pointEdge.weight != Scene::_connectionEdgeID) {
                    if (_requirements[pointEdge.weight]->getType() == ET_POINTONPOINT) {
                        deleting = false;
                    }
                    _requirements.erase(pointEdge.weight);
                }
            }

            if (deleting) {
                delete _points[pointID];
            }
            _points.erase(pointID);
            _graph.removeVertex(pointID);
            _idDeletedGeometricObject.insert(pointID);
        }
        else {
            _requirements.erase(sectionEdge.weight);
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
            }
            else if (circleEdge.to == circleID) {
                pointID = circleEdge.from;
            }

            bool deleting = true;
            std::vector<Edge<ID, ID>> pointEdges = _graph.getVertexEdges(pointID);
            for (auto& pointEdge : pointEdges) {
                if (pointEdge.weight != Scene::_connectionEdgeID) {
                    if (_requirements[pointEdge.weight]->getType() == ET_POINTONPOINT) {
                        deleting = false;
                    }
                    _requirements.erase(pointEdge.weight);
                }
            }

            if (deleting) {
                delete _points[pointID];
            }
            _points.erase(pointID);
            _graph.removeVertex(pointID);
            _idDeletedGeometricObject.insert(pointID);
        }
        else {
            _requirements.erase(circleEdge.weight);
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
            }
            else if (arcEdge.to == arcID) {
                pointID = arcEdge.from;
            }

            bool deleting = true;
            std::vector<Edge<ID, ID>> pointEdges = _graph.getVertexEdges(pointID);
            for (auto& pointEdge : pointEdges) {
                if (pointEdge.weight != Scene::_connectionEdgeID) {
                    if (_requirements[pointEdge.weight]->getType() == ET_POINTONPOINT) {
                        deleting = false;
                    }
                    _requirements.erase(pointEdge.weight);
                }
            }

            if (deleting) {
                delete _points[pointID];
            }
            _points.erase(pointID);
            _graph.removeVertex(pointID);
            _idDeletedGeometricObject.insert(pointID);
        }
        else {
            _requirements.erase(arcEdge.weight);
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
    for (auto& p : pointsWithoutDub) {
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
    _allFiguresRectangle = it->second->getBox();
    for (++it; it != _points.end(); ++it) {
        _allFiguresRectangle = _allFiguresRectangle | it->second->getBox();
    }
    _isRectangleDirty = false;
}

void Scene::paint() const {
    updateBoundingBox();
    _painter->getBoundBox(_allFiguresRectangle);
}

ObjectData Scene::getObjectData(ID id) const {
    ObjectData obj;
    if (auto it = _points.find(id); it != _points.end()) {
        obj.et = ET_POINT;
        obj.params = {
                it->second->x,
                it->second->y
        };
        return obj;
    }
    if (auto it = _sections.find(id); it != _sections.end()) {
        obj.et = ET_SECTION;
        obj.params = {
                it->second->beg->x,
                it->second->beg->y,
                it->second->end->x,
                it->second->end->y
        };
        return obj;
    }
    if (auto it = _circles.find(id); it != _circles.end()) {
        obj.et = ET_CIRCLE;
        obj.params = {
                it->second->center->x,
                it->second->center->y,
                it->second->r
        };
        return obj;
    }
    if (auto it = _arcs.find(id); it != _arcs.end()) {
        obj.et = ET_ARC;
        obj.params = {
                it->second->beg->x,
                it->second->beg->y,
                it->second->end->x,
                it->second->end->y,
                it->second->center->x,
                it->second->center->y,
                it->second->r
        };
        return obj;
    }
    throw std::invalid_argument("Object not found");
}

std::size_t Scene::edgeCount() const {
    return _graph.edgeCount();
}

std::size_t Scene::vertexCount() const {
    return _graph.vertexCount();
}

void Scene::setPainter(Painter* p) {
    _painter = p;
    _painter->initPoint(_points);
    _painter->initSection(_sections);
    _painter->initCircle(_circles);
    _painter->initArc(_arcs);
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

void Scene::setPoint(ID pointID, double x, double y) {
    if (!_points.contains(pointID)) {
        throw std::out_of_range("There is no point to change position");
    }
    Point* p = _points[pointID];
    if (p->x != x || p->y != y) {
        p->x = x;
        p->y = y;
        updateRequirements(pointID);
        _isRectangleDirty = true;
    }
}

void Scene::setSection(ID sectionID, double x1, double y1, double x2, double y2) {
    if (!_sections.contains(sectionID)) {
        throw std::out_of_range("There is no section to change");
    }
    Section* s = _sections[sectionID];
    if (s->beg->x != x1 || s->beg->y != y1 != s->beg->x != x2 || s->beg->x != y2) {
        s->beg->x = x1;
        s->beg->y = y1;
        s->end->x = x2;
        s->end->y = y2;
        updateRequirements(sectionID);
    }
}

void Scene::setCircle(ID circleID, double x, double y, double r) {
    if (!_circles.contains(circleID)) {
        throw std::out_of_range("There is no circle to change");
    }
    Circle* c = _circles[circleID];
    if (c->center->x != x || c->center->y != y || c->r != r) {
        c->center->x = x;
        c->center->y = y;
        c->r = r;
        updateRequirements(circleID);
    }
}

ID Scene::addRequirement(const RequirementData& reqData) {
    IReq* requirement = nullptr;
    if (reqData.objects.size() < 2) {
        throw std::invalid_argument("Insufficient data for requirement");
    }
    ID id1(reqData.objects[0]);
    ID id2(reqData.objects[1]);

    switch (reqData.req) {
        // 1
        case ET_POINTSECTIONDIST: {
            if (reqData.params.empty()) {
                throw std::invalid_argument("Insufficient data for requirement");
            }
            double dist = reqData.params[0];
            Point* p = nullptr;
            Section* s = nullptr;
            if (_points.contains(id1) && _sections.contains(id2)) {
                p = _points[id1];
                s = _sections[id2];
            } else if (_points.contains(id2) && _sections.contains(id1)) {
                p = _points[id2];
                s = _sections[id1];
            } else {
                throw std::invalid_argument("Objects must be a Point and a Section");
            }
            requirement = new ReqPointSecDist(p, s, dist);
            break;
        }

            // 2
        case ET_POINTONSECTION: {
            Point* p = nullptr;
            Section* s = nullptr;
            if (_points.contains(id1) && _sections.contains(id2)) {
                p = _points[id1];
                s = _sections[id2];
            } else if (_points.contains(id2) && _sections.contains(id1)) {
                p = _points[id2];
                s = _sections[id1];
            } else {
                throw std::invalid_argument("Objects must be a Point and a Section");
            }
            requirement = new ReqPointOnSec(p, s);
            break;
        }

            // 3
        case ET_POINTPOINTDIST: {
            if (reqData.params.empty()) {
                throw std::invalid_argument("Insufficient data for requirement");
            }
            double dist = reqData.params[0];
            Point* p1 = nullptr;
            Point* p2 = nullptr;
            if (_points.contains(id1) && _points.contains(id2)) {
                p1 = _points[id1];
                p2 = _points[id2];
            } else {
                throw std::invalid_argument("Objects must be two Point");
            }
            requirement = new ReqPointPointDist(p1, p2, dist);
            break;
        }

            // 4
        case ET_POINTONPOINT: {

            if (!_points.contains(id1) || !_points.contains(id2)) {
                throw std::invalid_argument("Objects must be two Point");
            }

            // Same points ptr
            if (_points[id1] == _points[id2]) {
                break;
            }

            Point* p1 = _points[id1];
            Point* p2 = _points[id2];

            delete p1;
            for (auto& [id, p] : _points) {
                if (p == p1) {
                    p = p2;
                }
            }

            // Update requirements
            for (auto& [id, req]: _requirements) {
                if (req->getType() == ET_POINTPOINTDIST) {
                    std::vector<IGeometricObject*> vec = req->getObjects();
                    if (vec.size() == 0) {
                        break;
                    }
                    Point* pt1 = static_cast<Point*>(vec[0]);
                    Point* pt2 = static_cast<Point*>(vec[1]);
                    if (pt1 != p1 && pt2 != p1) {
                        continue;
                    }
                    if (pt1 == p1) {
                        pt1 = p2;
                    }
                    if (pt2 == p1) {
                        pt2 = p2;
                    }
                    double v = *req->getParams()[4];
                    IReq* newReq = new ReqPointPointDist(pt1, pt2, v);
                    delete _requirements[id];
                    _requirements[id] = newReq;
                }
                else if (req->getType() == ET_SECTIONSECTIONPERPENDICULAR) {
                    std::vector<IGeometricObject*> vec = req->getObjects();
                    if (vec.size() == 0) {
                        break;
                    }
                    Section* s1 = static_cast<Section*>(vec[0]);
                    Section* s2 = static_cast<Section*>(vec[1]);
                    if (s1->beg != p1 && s1->end != p1 && s2->beg != p1 && s2->end != p1) {
                        continue;
                    }
                    if (s1->beg == p1) s1->beg = p2;
                    if (s1->end == p1) s1->end = p2;
                    if (s2->beg == p1) s2->beg = p2;
                    if (s2->end == p1) s2->end = p2;
                    IReq* newReq = new ReqSecSecPerpendicular(s1, s2);
                    delete _requirements[id];
                    _requirements[id] = newReq;
                }
                else if (req->getType() == ET_SECTIONSECTIONPARALLEL) {
                    std::vector<IGeometricObject*> vec = req->getObjects();
                    if (vec.size() == 0) {
                        break;
                    }
                    Section* s1 = static_cast<Section*>(vec[0]);
                    Section* s2 = static_cast<Section*>(vec[1]);
                    if (s1->beg != p1 && s1->end != p1 && s2->beg != p1 && s2->end != p1) {
                        continue;
                    }
                    if (s1->beg == p1) s1->beg = p2;
                    if (s1->end == p1) s1->end = p2;
                    if (s2->beg == p1) s2->beg = p2;
                    if (s2->end == p1) s2->end = p2;
                    IReq* newReq = new ReqSecSecParallel(s1, s2);
                    delete _requirements[id];
                    _requirements[id] = newReq;
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

            requirement = new ReqPointOnPoint(p1, p2);

            break;
        }

            // 5
        case ET_SECTIONCIRCLEDIST: {
            if (reqData.params.empty()) {
                throw std::invalid_argument("Insufficient data for requirement");
            }
            double dist = reqData.params[0];
            Section* s = nullptr;
            Circle* c = nullptr;
            if (_sections.contains(id1) && _circles.contains(id2)) {
                s = _sections[id1];
                c = _circles[id2];
            } else if (_sections.contains(id2) && _circles.contains(id1)) {
                s = _sections[id2];
                c = _circles[id1];
            } else {
                throw std::invalid_argument("Objects must be a Section and a Circle");
            }
            requirement = new ReqSecCircleDist(s, c, dist);
            break;
        }

            // 6
        case ET_SECTIONONCIRCLE: {
            Section* s = nullptr;
            Circle* c = nullptr;
            if (_sections.contains(id1) && _circles.contains(id2)) {
                s = _sections[id1];
                c = _circles[id2];
            } else if (_sections.contains(id2) && _circles.contains(id1)) {
                s = _sections[id2];
                c = _circles[id1];
            } else {
                throw std::invalid_argument("Objects must be a Section and a Circle");
            }
            requirement = new ReqSecOnCircle(s, c);
            break;
        }

            // 7
        case ET_SECTIONINCIRCLE: {
            Section* s = nullptr;
            Circle* c = nullptr;
            if (_sections.contains(id1) && _circles.contains(id2)) {
                s = _sections[id1];
                c = _circles[id2];
            } else if (_sections.contains(id2) && _circles.contains(id1)) {
                s = _sections[id2];
                c = _circles[id1];
            } else {
                throw std::invalid_argument("Objects must be a Section and a Circle");
            }
            requirement = new ReqSecInCircle(s, c);
            break;
        }

            // 8
        case ET_SECTIONSECTIONPARALLEL: {
            Section* s1 = nullptr;
            Section* s2 = nullptr;
            if (_sections.contains(id1) && _sections.contains(id2)) {
                s1 = _sections[id1];
                s2 = _sections[id2];
            } else {
                throw std::invalid_argument("Objects must be two Section");
            }
            requirement = new ReqSecSecParallel(s1, s2);
            break;
        }

            // 9
        case ET_SECTIONSECTIONPERPENDICULAR: {
            Section* s1 = nullptr;
            Section* s2 = nullptr;
            if (_sections.contains(id1) && _sections.contains(id2)) {
                s1 = _sections[id1];
                s2 = _sections[id2];
            } else {
                throw std::invalid_argument("Objects must be two Section");
            }
            requirement = new ReqSecSecPerpendicular(s1, s2);
            break;
        }

            // 10
        case ET_SECTIONSECTIONANGLE: {
            if (reqData.params.empty()) {
                throw std::invalid_argument("Insufficient data for requirement");
            }
            double angle = reqData.params[0];
            Section* s1 = nullptr;
            Section* s2 = nullptr;
            if (_sections.contains(id1) && _sections.contains(id2)) {
                s1 = _sections[id1];
                s2 = _sections[id2];
            } else {
                throw std::invalid_argument("Objects must be two Section");
            }
            requirement = new ReqSecSecAngel(s1, s2, angle);
            break;
        }

        default: {
            throw std::invalid_argument("Unknown requirement type");
        }
    }

    if (!requirement) {
        throw std::invalid_argument("Invalid requirement data");
    }

    ID newID = _idRequirementsGenerator.generate();
    _graph.addEdge(id1, id2, newID);
    _requirements[newID] = requirement;

    _isComponentsDirty = true;
    updateRequirements(id1);
    return newID;
}

const Component& Scene::findComponentByID(ID id) {
    if (_isComponentsDirty) {
        rebuildComponents();
    }
    auto it = _idToComponent.find(id);
    if (it == _idToComponent.end()) {
        throw std::runtime_error("ID not found in any component");
    }
    return _components[it->second];
}

void Scene::rebuildComponents() {
    _components.clear();
    _idToComponent.clear();
    std::vector<std::vector<ID>> allComponents = _graph.connectedComponents();
    for (std::size_t i = 0; auto& vec : allComponents) {
        for (auto& vertexID : vec) {
            _idToComponent[vertexID] = i;
        }
        ++i;
        Graph<ID, ID, UndirectedPolicy, WeightedPolicy> component = _graph.subGraph(vec);
        std::vector<Edge<ID, ID>> componentEdges = component.getAllEdges();
        std::vector<ID> componentVertices = component.getVertices();
        Component c;
        std::vector<Variable*> variables;
        std::unordered_set<double*> seen;
        for (auto& edge : componentEdges) {
            if (edge.weight != ID(-2) && _requirements[edge.weight]->getType() != ET_POINTONPOINT) {
                std::vector<Variable*> variablesWithDuplicates = _requirements[edge.weight]->getVariables();
                variables.reserve(variablesWithDuplicates.size());
                for (auto& var : variablesWithDuplicates) {
                    if (seen.insert(var->value).second) {
                        variables.push_back(var);
                    }
                }
                c._errorRequirementFunctions.push_back(_requirements[edge.weight]->getFunction());
            }
        }
        c._componentVars = variables;
        if (!componentVertices.empty()) {
            std::vector<ID> cC = _graph.findConnectedComponent(componentVertices[0]);
            c._objectIDs = std::unordered_set(cC.begin(), cC.end());
        }
        _components.push_back(std::move(c));
    }
    _isComponentsDirty = false;
}

void Scene::updateRequirements(ID id) {
    const Component& component = findComponentByID(id);

    std::size_t number_of_obj_in_component = component._objectIDs.size();
    std::cout << "Objects in component:  " << number_of_obj_in_component << '\n';

    if (component._errorRequirementFunctions.empty()) {
        return;
    }


    LSMFORLMTask* task = new LSMFORLMTask(component._errorRequirementFunctions, component._componentVars);
    LevenbergMarquardtSolver solver(10000, 0.5, 2, 4, 1e-07, 1e-07);
    solver.setTask(task);
    solver.optimize();

    // Check converging
    if (!solver.isConverged() || solver.getCurrentError() > 1e-6) {
        delete task;
        throw std::runtime_error("Not converged");
    }

    delete task;
}

// TODO
std::string Scene::to_string() const {
    FileOurP saver;
    for (auto& pair: _points) {
        std::pair<unsigned int, IGeometricObject*> res;
        res.first = pair.first.get();
        res.second = pair.second;
        saver.addObject(res);
    }
    for (auto& pair: _sections) {
        std::pair<unsigned int, IGeometricObject*> res;
        res.first = pair.first.get();
        res.second = pair.second;
        saver.addObject(res);
    }
    for (auto& pair: _circles) {
        std::pair<unsigned int, IGeometricObject*> res;
        res.first = pair.first.get();
        res.second = pair.second;
        saver.addObject(res);
    }
    for (auto& pair: _requirements) {
        std::pair<unsigned int, RequirementData> res;
        res.first = pair.first.get();
        RequirementData rd;
        rd.req = pair.second->getType();
        double d = pair.second->getDimension();
        rd.params.push_back(d);
        std::vector<Edge<ID, ID>> edges = _graph.getAllEdges();
        for (auto& edge: edges) {
            if (edge.weight == pair.first) {
                rd.objects.push_back(edge.from.get());
                rd.objects.push_back(edge.to.get());
                break;
            }
        }
        saver.addRequirement(res);
    }
    return saver.to_string();
}

void Scene::saveToFile(const char* filename) const {
    FileOurP saver;
    for (const auto& [id, obj]: _points) {
        std::pair<unsigned int, IGeometricObject*> m{id.get(), obj};
        saver.addObject(m);
    }
    for (const auto& [id, obj]: _sections) {
        std::pair<unsigned int, IGeometricObject*> m{id.get(), obj};
        saver.addObject(m);
    }
    for (const auto& [id, obj]: _circles) {
        std::pair<unsigned int, IGeometricObject*> m{id.get(), obj};
        saver.addObject(m);
    }
    for (const auto& [id, req]: _requirements) {
        RequirementData rd;
        rd.req = req->getType();
        double d = req->getDimension();
        rd.params.push_back(d);
        std::vector<Edge<ID, ID>> edges = _graph.getAllEdges();
        for (auto& edge: edges) {
            if (edge.weight == id) {
                rd.objects.push_back(edge.from.get());
                rd.objects.push_back(edge.to.get());
                break;
            }
        }
        std::pair<unsigned int, RequirementData> m{id.get(), rd};
        saver.addRequirement(m);
    }
    saver.saveToOurP(filename);
}

void Scene::loadFromFile(const char* filename) {
    FileOurP loader;

    loader.loadFromOurP(filename);
    clear();
    const std::vector<objectInFile>& vecObjectInFile = loader.getObjects();
    std::queue<ObjectData> pointObjectData;
    for (auto& objInFile: vecObjectInFile) {
        IGeometricObject* obj = objInFile.to_pair().second;
        ObjectData objData;
        if (obj->getType() == ET_POINT) {
            Point* p = static_cast<Point*>(obj);
            objData.et = ET_POINT;
            objData.params.push_back(p->x);
            objData.params.push_back(p->y);
            if (pointObjectData.size() > 2) {
                addObject(pointObjectData.front());
                pointObjectData.pop();
            }
            else {
                pointObjectData.push(objData);
            }
        }
        else if (obj->getType() == ET_SECTION) {
            Section* s = static_cast<Section*>(obj);
            objData.et = ET_SECTION;
            objData.params.push_back(s->beg->x);
            objData.params.push_back(s->beg->y);
            objData.params.push_back(s->end->x);
            objData.params.push_back(s->end->y);
            pointObjectData.pop();
            pointObjectData.pop();
            addObject(objData);
        }
        else if (obj->getType() == ET_CIRCLE) {
            Circle* c = static_cast<Circle*>(obj);
            objData.et = ET_CIRCLE;
            objData.params.push_back(c->center->x);
            objData.params.push_back(c->center->y);
            objData.params.push_back(c->r);
            pointObjectData.pop();
            addObject(objData);
        }
        else if (obj->getType() == ET_ARC) {
            Arc* a = static_cast<Arc*>(obj);
            objData.et = ET_ARC;
            objData.params.push_back(a->beg->x);
            objData.params.push_back(a->beg->y);
            objData.params.push_back(a->end->x);
            objData.params.push_back(a->end->y);
            objData.params.push_back(a->center->x);
            objData.params.push_back(a->center->y);
            objData.params.push_back(a->r);
            addObject(objData);
        }
    }


    const std::vector<requirementInFile> vecReqInFile = loader.getRequirements();
    for (auto& reqInFile : vecReqInFile) {
        RequirementData rd = reqInFile.to_pair().second;
        addRequirement(rd);
    }
}

