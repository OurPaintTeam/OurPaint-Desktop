#include "Scene.h"

Scene::Scene(Painter *p) :
        _painter(p),
        _points(),
        _sections(),
        _circles(),
        _isRectangleDirty(false),
        _allFiguresRectangle(),
        _graph() {
    if (p) {
        p->initPoint(_points);
        p->initSection(_sections);
        p->initCircle(_circles);
    }
}

Scene::~Scene() {
    for (auto &pair: _points) {
        delete pair.second;
    }
    for (auto &pair: _sections) {
        delete pair.second;
    }
    for (auto &pair: _circles) {
        delete pair.second;
    }
    for (auto &pair: _requirements) {
        delete pair.second;
    }
    _points.clear();
    _sections.clear();
    _circles.clear();
    _requirements.clear();
}

ID Scene::addObject(const ObjectData &objData) {
    switch (objData.et) {
        case ET_POINT: {
            if (objData.params.size() < 2) {
                throw std::invalid_argument("Point requires 2 coordinates");
            }
            ID newID = _idGeometricObjectsGenerator.generate();
            Point *p = new Point(objData.params[0], objData.params[1]);
            _points[newID] = p;
            _allFiguresRectangle = _allFiguresRectangle | p->getBox();
            _graph.addVertex(newID);
            return newID;
        }
        case ET_SECTION: {
            if (objData.params.size() < 4) {
                throw std::invalid_argument("Section requires 4 coordinates");
            }
            Point *p1 = new Point(objData.params[0], objData.params[1]);
            ID p1ID = _idGeometricObjectsGenerator.generate();
            _points[p1ID] = p1;
            _graph.addVertex(p1ID);

            Point *p2 = new Point(objData.params[2], objData.params[3]);
            ID p2ID = _idGeometricObjectsGenerator.generate();
            _points[p2ID] = p2;
            _graph.addVertex(p2ID);

            ID newID = _idGeometricObjectsGenerator.generate();
            Section *s = new Section(p1, p2);
            _sections[newID] = s;
            _graph.addVertex(newID);

            _allFiguresRectangle = _allFiguresRectangle | s->getBox();
            return newID;
        }
        case ET_CIRCLE: {
            if (objData.params.size() < 3) {
                throw std::invalid_argument("Circle requires center and radius");
            }
            Point *p = new Point(objData.params[0], objData.params[1]);
            ID pID = _idGeometricObjectsGenerator.generate();
            _points[pID] = p;
            _graph.addVertex(pID);;
            Circle *c = new Circle(p, objData.params[2]);
            ID newID = _idGeometricObjectsGenerator.generate();
            _circles[newID] = c;
            _graph.addVertex(newID);

            _allFiguresRectangle = _allFiguresRectangle | c->getBox();
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

void Scene::clear() {
    for (auto &pair: _points) {
        delete pair.second;
    }
    for (auto &pair: _sections) {
        delete pair.second;
    }
    for (auto &pair: _circles) {
        delete pair.second;
    }
    _points.clear();
    _sections.clear();
    _circles.clear();

    _allFiguresRectangle = BoundBox2D{};
    _isRectangleDirty = false;
    _idGeometricObjectsGenerator.reset();
}

const BoundBox2D &Scene::getBoundingBox() const {
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
    return nullptr;
}

std::unordered_map<ID, const IGeometricObject*> Scene::getAllObjects() const {
    std::unordered_map<ID, const IGeometricObject*> result;

    for (const auto &[id, obj]: _points) {
        result[id] = obj;
    }
    for (const auto &[id, obj]: _sections) {
        result[id] = obj;
    }
    for (const auto &[id, obj]: _circles) {
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

std::size_t Scene::edgeCount() const {
    return _graph.edgeCount();
}

std::size_t Scene::vertexCount() const {
    return _graph.vertexCount();
}

void Scene::setPainter(Painter *p) {
    _painter = p;
    _painter->initPoint(_points);
    _painter->initSection(_sections);
    _painter->initCircle(_circles);
}

void Scene::moveObject(ID id, double dx, double dy) {
    if (_points.contains(id)) {
        Point *p = _points[id];
        p->x += dx;
        p->y += dy;
    } else if (_sections.contains(id)) {
        Section *s = _sections[id];
        s->beg->x += dx;
        s->beg->y += dy;
        s->end->x += dx;
        s->end->x += dy;
    } else if (_circles.contains(id)) {
        Circle *c = _circles[id];
        c->center->x += dx;
        c->center->y += dy;
    } else {
        throw std::runtime_error("Failed to move object");
    }
    _isRectangleDirty = true;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(id);
}

void Scene::movePoint(ID pointID, double dx, double dy) {
    if (_points.contains(pointID)) {
        Point *p = _points[pointID];
        p->x += dx;
        p->y += dy;
    }
    _isRectangleDirty = true;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(pointID);
}
void Scene::moveSection(ID sectionID, double dx, double dy) {
    if (_sections.contains(sectionID)) {
        Section *s = _sections[sectionID];
        s->beg->x += dx;
        s->beg->y += dy;
        s->end->x += dx;
        s->end->x += dy;
    }
    _isRectangleDirty = true;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(sectionID);
}
void Scene::moveCircle(ID circleID, double dx, double dy) {
    if (_circles.contains(circleID)) {
        Circle *c = _circles[circleID];
        c->center->x += dx;
        c->center->y += dy;
    }
    _isRectangleDirty = true;
    // TODO по выполнению функции updateRequirements может изменить систему так что функцию после завершения не выполнила свою работу, исправить.
    updateRequirements(circleID);
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

ID Scene::addRequirement(const RequirementData &reqData) {
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
            }
            else if (_points.contains(id2) && _sections.contains(id1)) {
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
            }
            else if (_points.contains(id2) && _sections.contains(id1)) {
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
            Point* p1 = nullptr;
            Point* p2 = nullptr;
            if (_points.contains(id1) && _points.contains(id2)) {
                p1 = _points[id1];
                p2 = _points[id2];
            } else {
                throw std::invalid_argument("Objects must be two Point");
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
            }
            else if (_sections.contains(id2) && _circles.contains(id1)) {
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
            }
            else if (_sections.contains(id2) && _circles.contains(id1)) {
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
            }
            else if (_sections.contains(id2) && _circles.contains(id1)) {
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
        case ET_POINTONCIRCLE: {
            break;
        }

        // 12
        case ET_POINTINOBJECT: {
            break;
        }

        default: {
            std::invalid_argument("Unknown requirement type");
        }
    }

    if (!requirement) {
        throw std::invalid_argument("Invalid RequirementData");
    }

    ID newID = _idRequirementsGenerator.generate();
    _requirements[newID] = requirement;
    _graph.addEdge(id1, id2, newID);
    _errorRequirementFunctions.push_back(requirement->getFunction());

    updateRequirements(id1);

    return newID;
}

void Scene::updateRequirements(ID objectID) {
    std::size_t count_of_req_in_component = 0;

    std::vector<ID> connectedComponent = _graph.findConnectedComponent(objectID);
    //std::unordered_set<ID> connectedObjects(connectedComponent.begin(), connectedComponent.end());

    if(connectedComponent.size()<=1){
        return;
    }

    // TODO Решать только компоненту
    // TODO Избавиться от копирования
    std::vector<Function*> copyFunctions;
    for (auto& func : _errorRequirementFunctions) {
        copyFunctions.push_back(func->clone());
    }

    // --------------------------- Main Optimizing and Computing Algorithm ---------------------------
    LSMFORLMTask* task = new LSMFORLMTask(copyFunctions, VarsStorage::getVars());
    LevenbergMarquardtSolver solver(10000, 0.5, 2, 4, 1e-07, 1e-07);
    solver.setTask(task);
    solver.optimize();
    // -----------------------------------------------------------------------------------------------

    std::cout << "Number of requirements in a component: " << count_of_req_in_component << std::endl;

    // Check converging
    if (!solver.isConverged() || solver.getCurrentError() > 1e-6){
        delete task;
        VarsStorage::clearVars();
        throw std::runtime_error("Not converged");
    }

    //updateBoundingBox(); // TODO Недешевая операция

    delete task;
    VarsStorage::clearVars();
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
        for (auto& it : vec) {
            rd.params.push_back(*it);
        }
        std::vector<Edge<ID, ID>> edges = _graph.getAllEdges();
        for (auto& edge : edges) {
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
    for (const auto& [id, obj] : _points) {
        std::pair<unsigned int, IGeometricObject*> m{id.get(), obj};
        saver.addObject(m);
    }
    for (const auto& [id, obj] : _sections) {
        std::pair<unsigned int, IGeometricObject*> m{id.get(), obj};
        saver.addObject(m);
    }
    for (const auto& [id, obj] : _circles) {
        std::pair<unsigned int, IGeometricObject*> m{id.get(), obj};
        saver.addObject(m);
    }
    for (const auto& [id, req] : _requirements) {
        RequirementData rd;
        rd.req = req->getType();
        std::vector<double*> vec = req->getParams();
        for (auto& it : vec) {
            rd.params.push_back(*it);
        }
        std::vector<Edge<ID, ID>> edges = _graph.getAllEdges();
        for (auto& edge : edges) {
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
    for (auto& objData : vecObjData) {
        std::pair<unsigned int, IGeometricObject*> pair = objData.to_pair();
        IGeometricObject* obj = pair.second;
        ObjectData od;
        od.et = obj->getType();
        if (od.et == ET_POINT) {
            Point* p = static_cast<Point*>(obj);
            od.params.push_back(p->x);
            od.params.push_back(p->y);
        }
        else if (od.et == ET_SECTION) {
            Section* s = static_cast<Section*>(obj);
            od.params.push_back(s->beg->x);
            od.params.push_back(s->beg->y);
            od.params.push_back(s->end->x);
            od.params.push_back(s->end->y);
        }
        else if (od.et == ET_CIRCLE) {
            Circle* c = static_cast<Circle*>(obj);
            od.params.push_back(c->center->x);
            od.params.push_back(c->center->y);
            od.params.push_back(c->r);
        }
        addObject(od);
    }
    const std::vector<requirementInFile>& vecReqData = loader.getRequirements();
    for (auto& reqData : vecReqData) {
        std::pair<unsigned int, RequirementData> pair = reqData.to_pair();
        addRequirement(pair.second);
    }
}




