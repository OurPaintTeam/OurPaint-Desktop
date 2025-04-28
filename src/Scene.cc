#include "Scene.h"

Scene::Scene(Painter* p) :
        _painter(p),
        _points(),
        _arcs(),
        _sections(),
        _circles(),
        _isRectangleDirty(false),
        _allFiguresRectangle(),
        _graph() {
    if (p) {
        p->initPoint(_points);
        p->initSection(_sections);
        p->initCircle(_circles);
        p->initArc(_arcs);
    }
}

Scene::~Scene() {
    for (auto& pair: _points) {
        delete pair.second;
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
    for (auto& pair: _requirements) {
        delete pair.second;
    }
    _points.clear();
    _sections.clear();
    _circles.clear();
    _arcs.clear();
    _requirements.clear();

    _graph = UndirectedWeightedGraph<ID, ID>{};

    VarsStorage::clearVars();
}

ID Scene::addObject(const ObjectData& objData) {
    switch (objData.et) {
        case ET_POINT: {
            if (objData.params.size() < 2) {
                throw std::invalid_argument("Point requires 2 coordinates");
            }
            ID newID = _idGeometricObjectsGenerator.generate();
            Point* p = new Point(objData.params[0], objData.params[1]);
            _points[newID] = p;
            _allFiguresRectangle = _allFiguresRectangle | p->getBox();
            _graph.addVertex(newID);
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

            ID newID = _idGeometricObjectsGenerator.generate();
            Section* s = new Section(p1, p2);
            _sections[newID] = s;
            _graph.addVertex(newID);

            _graph.addEdge(pID1, newID, ID(-2));
            _graph.addEdge(pID2, newID, ID(-2));

            _allFiguresRectangle = _allFiguresRectangle | s->getBox();
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
            Circle* c = new Circle(p, objData.params[2]);
            ID newID = _idGeometricObjectsGenerator.generate();
            _circles[newID] = c;
            _graph.addVertex(newID);

            _graph.addEdge(pID, newID, ID(-2));

            _allFiguresRectangle = _allFiguresRectangle | c->getBox();
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

            Arc* a = new Arc(p1, p2, p3, objData.params[6]);
            ID newID = _idGeometricObjectsGenerator.generate();
            _arcs[newID] = a;
            _graph.addVertex(newID);

            _graph.addEdge(pID1, newID, ID(-2));
            _graph.addEdge(pID2, newID, ID(-2));
            _graph.addEdge(pID2, newID, ID(-2));

            _allFiguresRectangle = _allFiguresRectangle | a->getBox();
            return newID;
        }
        default:
            throw std::invalid_argument("Unknown object type");
    }
}

bool Scene::deleteObject(ID objectID) {
    auto it_point = _points.find(objectID);
    if (it_point != _points.end()) {
        if (it_point->second) {
            delete it_point->second;
        }
        _points.erase(it_point);
        _isRectangleDirty = true;
        return true;
    }

    auto it_section = _sections.find(objectID);
    if (it_section != _sections.end()) {
        if (it_section->second) {
            delete it_section->second;
        }
        _sections.erase(it_section);
        _isRectangleDirty = true;
        return true;
    }

    auto it_circle = _circles.find(objectID);
    if (it_circle != _circles.end()) {
        if (it_circle->second) {
            delete it_circle->second;
        }
        _circles.erase(it_circle);
        _isRectangleDirty = true;
        return true;
    }

    auto it_arc = _arcs.find(objectID);
    if (it_arc != _arcs.end()) {
        if (it_arc->second) {
            delete it_arc->second;
        }
        _arcs.erase(it_arc);
        _isRectangleDirty = true;
        return true;
    }

    return false;
}

bool Scene::deletePoint(ID pointID) {
    auto it_point = _points.find(pointID);
    if (it_point != _points.end()) {
        if (it_point->second) {
            delete it_point->second;
        }
        _points.erase(it_point);
        _isRectangleDirty = true;
        return true;
    }
    return false;
}

bool Scene::deleteSection(ID sectionID) {
    auto it_section = _sections.find(sectionID);
    if (it_section != _sections.end()) {
        if (it_section->second) {
            delete it_section->second;
        }
        _sections.erase(it_section);
        _isRectangleDirty = true;
        return true;
    }
    return false;
}

bool Scene::deleteCircle(ID circleID) {
    auto it_circle = _circles.find(circleID);
    if (it_circle != _circles.end()) {
        if (it_circle->second) {
            delete it_circle->second;
        }
        _circles.erase(it_circle);
        _isRectangleDirty = true;
        return true;
    }
    return false;
}

bool Scene::deleteArc(ID arcID) {
    auto it_arc = _arcs.find(arcID);
    if (it_arc != _arcs.end()) {
        if (it_arc->second) {
            delete it_arc->second;
        }
        _arcs.erase(it_arc);
        _isRectangleDirty = true;
        return true;
    }
    return false;
}

void Scene::clear() {
    for (auto& pair: _points) {
        delete pair.second;
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

    _graph = UndirectedWeightedGraph<ID, ID>{};

    _allFiguresRectangle = BoundBox2D{};
    _isRectangleDirty = false;
    _idGeometricObjectsGenerator.reset();
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

const IGeometricObject* Scene::getObject(ID id) const {
    if (auto it = _points.find(id); it != _points.end()) {
        return it->second;
    }
    if (auto it = _sections.find(id); it != _sections.end()) {
        return it->second;
    }
    if (auto it = _circles.find(id); it != _circles.end()) {
        return it->second;
    }
    if (auto it = _arcs.find(id); it != _arcs.end()) {
        return it->second;
    }
    return nullptr;
}

std::unordered_map<ID, const IGeometricObject*> Scene::getAllObjects() const {
    std::unordered_map<ID, const IGeometricObject*> result;

    for (const auto& [id, obj]: _points) {
        result[id] = obj;
    }
    for (const auto& [id, obj]: _sections) {
        result[id] = obj;
    }
    for (const auto& [id, obj]: _circles) {
        result[id] = obj;
    }
    for (const auto& [id, obj]: _arcs) {
        result[id] = obj;
    }
    return result;
}

std::unordered_map<ID, const Point*> Scene::getPoints() const {
    return {_points.begin(), _points.end()};
}

std::unordered_map<ID, const Section*> Scene::getSections() const {
    return {_sections.begin(), _sections.end()};
}

std::unordered_map<ID, const Circle*> Scene::getCircles() const {
    return {_circles.begin(), _circles.end()};
}

std::unordered_map<ID, const Arc*> Scene::getArcs() const {
    return {_arcs.begin(), _arcs.end()};
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
    if (!_sections.contains(sectionID)) {
        throw std::invalid_argument("There is no section to move");
    }
    Section* s = _sections[sectionID];
    s->beg->x += dx;
    s->beg->y += dy;
    s->end->x += dx;
    s->end->y += dy;
    _isRectangleDirty = true;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(ID(sectionID.get() - 1));
    //std::cout << "Move section" << '\n';
}

void Scene::moveCircle(ID circleID, double dx, double dy) {
    if (_circles.contains(circleID)) {
        Circle* c = _circles[circleID];
        c->center->x += dx;
        c->center->y += dy;
    }
    _isRectangleDirty = true;
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
    p->x = x;
    p->y = y;
    updateRequirements(pointID);
}

void Scene::setSection(ID sectionID, double x1, double y1, double x2, double y2) {
    if (!_sections.contains(sectionID)) {
        throw std::out_of_range("There is no section to change");
    }
    Section* s = _sections[sectionID];
    s->beg->x = x1;
    s->beg->y = y1;
    s->beg->x = x2;
    s->beg->x = y2;
    updateRequirements(sectionID);
}

void Scene::setCircle(ID circleID, double x, double y, double r) {
    if (!_circles.contains(circleID)) {
        throw std::out_of_range("There is no circle to change");
    }
    Circle* c = _circles[circleID];
    c->center->x = x;
    c->center->y = y;
    c->r = r;
    updateRequirements(circleID);
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

            delete _points[id1];
            _points[id1] = p2;

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
                } else if (req->getType() == ET_SECTIONSECTIONPERPENDICULAR) {
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
                } else if (req->getType() == ET_SECTIONSECTIONPARALLEL) {
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

            // Update req functions
            for (auto& func: _errorRequirementFunctions) {
                delete func;
            }
            _errorRequirementFunctions.clear();
            for (auto& [id, req]: _requirements) {
                if (req->getType() != ET_POINTONPOINT) {
                    _errorRequirementFunctions.push_back(req->getFunction());
                }
            }

            // Update VarsStorage
            VarsStorage::deleteVar(&p1->x);
            VarsStorage::deleteVar(&p1->y);

            for (auto& [id, sec]: _sections) {
                if (sec->beg == p1) {
                    sec->beg = p2;
                }
                if (sec->end == p1) {
                    sec->end = p2;
                }
            }

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

            // 11
        case ET_POINTINOBJECT: {
            break;
        }

        default: {
            std::invalid_argument("Unknown requirement type");
        }
    }

    if (!requirement && reqData.req != ET_POINTONPOINT) {
        throw std::invalid_argument("Invalid requirement data");
    }

    ID newID = _idRequirementsGenerator.generate();
    _graph.addEdge(id1, id2, newID);
    _requirements[newID] = requirement;

    if (reqData.req != ET_POINTONPOINT) {
        _errorRequirementFunctions.push_back(requirement->getFunction());
    }

    updateRequirements(id1);
    return newID;
}

void Scene::updateRequirements(ID objectID) {
    std::size_t number_of_obj_in_component = 0;

    std::vector<ID> connectedComponent = _graph.findConnectedComponent(objectID);

    number_of_obj_in_component = connectedComponent.size();
    if (_errorRequirementFunctions.size() == 0) {
        return;
    }
    std::cout << "Objects in component:  " << number_of_obj_in_component << std::endl;

    // TODO Решать только компоненту

    LSMFORLMTask* task = new LSMFORLMTask(_errorRequirementFunctions, VarsStorage::getVars());
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
        std::vector<double*> vec = pair.second->getParams();
        for (auto& it: vec) {
            rd.params.push_back(*it);
        }
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
        std::vector<double*> vec = req->getParams();
        for (auto& it: vec) {
            rd.params.push_back(*it);
        }
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
    const std::vector<objectInFile>& vecObjData = loader.getObjects();
    for (auto& objData: vecObjData) {
        std::pair<unsigned int, IGeometricObject*> pair = objData.to_pair();
        IGeometricObject* obj = pair.second;
        if (obj->getType() == ET_POINT) {
            Point* p = static_cast<Point*>(obj);
            Point* p2 = new Point();
            p2->x = p->x;
            p2->y = p->y;
            _points[ID(pair.first)] = p2;
            _graph.addVertex(ID(pair.first));
        } else if (obj->getType() == ET_SECTION) {
            Section* s = static_cast<Section*>(obj);
            Point* p1 = new Point();
            p1->x = s->beg->x;
            p1->y = s->beg->y;
            Point* p2 = new Point();
            p2->x = s->end->x;
            p2->y = s->end->y;
            Section* s1 = new Section();
            s1->beg = p1;
            s1->end = p2;
            _points[ID(pair.first - 1)] = p1;
            _points[ID(pair.first - 2)] = p2;
            _sections[ID(pair.first)] = s1;
            _graph.addVertex(ID(pair.first - 2));
            _graph.addVertex(ID(pair.first - 1));
            _graph.addVertex(ID(pair.first));
        } else if (obj->getType() == ET_CIRCLE) {
            Circle* c = static_cast<Circle*>(obj);
            Point* p1 = new Point();
            p1->x = c->center->x;
            p1->y = c->center->y;
            _points[ID(pair.first - 1)] = p1;
            _circles[ID(pair.first)] = c;
            _graph.addVertex(ID(pair.first - 1));
            _graph.addVertex(ID(pair.first));
        }
    }
    const std::vector<requirementInFile>& vecReqData = loader.getRequirements();
    for (auto& reqData: vecReqData) {
        std::pair<unsigned int, RequirementData> pair = reqData.to_pair();
        RequirementData rd = pair.second;
        ID id1(pair.second.objects[0]);
        ID id2(pair.second.objects[1]);
        IReq* requirement = nullptr;
        switch (pair.second.req) {
            // 1
            case ET_POINTSECTIONDIST: {
                if (rd.params.empty()) {
                    throw std::invalid_argument("Insufficient data for requirement");
                }
                double dist = rd.params[0];
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
                if (rd.params.empty()) {
                    throw std::invalid_argument("Insufficient data for requirement");
                }
                double dist = rd.params[0];
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

                delete _points[id1];
                _points.erase(id1);

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
                }

                // Update req functions
                for (auto& func: _errorRequirementFunctions) {
                    delete func;
                }
                _errorRequirementFunctions.clear();
                for (auto& [id, req]: _requirements) {
                    _errorRequirementFunctions.push_back(req->getFunction());
                }

                // Update VarsStorage
                VarsStorage::deleteVar(&p1->x);
                VarsStorage::deleteVar(&p1->y);

                for (auto& [id, sec]: _sections) {
                    if (sec->beg == p1) {
                        sec->beg = p2;
                    }
                    if (sec->end == p1) {
                        sec->end = p2;
                    }
                }

                break;
            }

                // 5
            case ET_SECTIONCIRCLEDIST: {
                if (rd.params.empty()) {
                    throw std::invalid_argument("Insufficient data for requirement");
                }
                double dist = rd.params[0];
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
                if (rd.params.empty()) {
                    throw std::invalid_argument("Insufficient data for requirement");
                }
                double angle = rd.params[0];
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
                std::invalid_argument("Unknown requirement type");
            }
        }
        _requirements[ID(pair.first)] = requirement;
        _graph.addEdge(id1, id2, ID(pair.first));
        _errorRequirementFunctions.push_back(requirement->getFunction());
    }
}

