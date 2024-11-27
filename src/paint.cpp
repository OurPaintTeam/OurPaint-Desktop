#include "paint.h"

ElementData::ElementData() {
    params = std::vector<double>();
}

ID Paint::addRequirement(const RequirementData &rd) {
    std::vector<IReq *> allRequirements;
    ActionsInfo info;
    auto it = m_reqD.addElement(rd);
    int countOfReq = 0;

    // Update graph: add vertex and edges for new req
    for (const auto& obj : rd.objects) {
        m_graph.addVertex(obj);
    }
    for (size_t i = 0; i < rd.objects.size(); ++i) {
        for (size_t j = i + 1; j < rd.objects.size(); ++j) {
            m_graph.addEdge(rd, rd.objects[i], rd.objects[j]);
        }
    }

    // Find component
    std::vector<ID> connectedComponent = m_graph.findConnectedComponent(rd.objects[0]);
    std::unordered_set<ID> connectedObjects(connectedComponent.begin(), connectedComponent.end());


    // Getting all req and parameters
    for (const auto &r: m_reqD) {
        bool isConnected = false;
        for (const auto& obj : r.objects) {
            if (connectedObjects.find(obj) != connectedObjects.end()) {
                isConnected = true;
                break;
            }
        }
        if (!isConnected) {
            continue;
        }

        countOfReq++;
        IReq *requirement = nullptr;

        // 1
        if (r.req == ET_POINTSECTIONDIST) {
            point *p_it = nullptr;
            section *s_it = nullptr;
            try {
                p_it = &(*(m_pointIDs.at(r.objects[0])));
                s_it = &(*(m_sectionIDs.at(r.objects[1])));
            } catch (...) {
                try {
                    p_it = &(*(m_pointIDs.at(r.objects[1])));
                    s_it = &(*(m_sectionIDs.at(r.objects[0])));
                } catch (...) {
                    m_reqD.remove(it);
                    throw std::runtime_error("No such point or section");
                }
            }
            requirement = new ReqPointSecDist(p_it, s_it, r.params[0]);
        }

        // 2
        else if (r.req == ET_POINTONSECTION) {
            point *p_it = nullptr;
            section *s_it = nullptr;
            try {
                p_it = &(*(m_pointIDs.at(r.objects[0])));
                s_it = &(*(m_sectionIDs.at(r.objects[1])));
            } catch (...) {
                try {
                    p_it = &(*(m_pointIDs.at(r.objects[1])));
                    s_it = &(*(m_sectionIDs.at(r.objects[0])));
                } catch (...) {
                    m_reqD.remove(it);
                    throw std::runtime_error("No such point or section");
                }
            }
            requirement = new ReqPointOnSec(p_it, s_it);
        }

        // 3
        else if (r.req == ET_POINTPOINTDIST) {
            point *p1_it = nullptr;
            point *p2_it = nullptr;
            try {
                p1_it = &(*(m_pointIDs.at(r.objects[0])));
                p2_it = &(*(m_pointIDs.at(r.objects[1])));
            } catch (...) {
                m_reqD.remove(it);
                throw std::runtime_error("No such point");
            }
            requirement = new ReqPointPointDist(p1_it, p2_it, r.params[0]);
        }

        // 4
        else if (r.req == ET_POINTONPOINT) {
            point *p1_it = nullptr;
            point *p2_it = nullptr;
            try {
                p1_it = &(*(m_pointIDs.at(r.objects[0])));
                p2_it = &(*(m_pointIDs.at(r.objects[1])));
            } catch (...) {
                m_reqD.remove(it);
                throw std::runtime_error("No such point");
            }
            requirement = new ReqPointOnPoint(p1_it, p2_it);
        }

        // 5
        else if (r.req == ET_SECTIONCIRCLEDIST) {
            circle* c_it = nullptr;
            section* s_it = nullptr;
            try {
                c_it = &(*(m_circleIDs.at(r.objects[0])));
                s_it = &(*(m_sectionIDs.at(r.objects[1])));
            }catch (...){
                try {
                    c_it = &(*(m_circleIDs.at(r.objects[1])));
                    s_it = &(*(m_sectionIDs.at(r.objects[0])));
                }catch(...) {
                    m_reqD.remove(it);
                    throw std::runtime_error("No such circle or section");
                }
            }
            requirement = new ReqSecCircleDist(s_it, c_it, r.params[0]);
        }

        // 6
        else if (r.req == ET_SECTIONONCIRCLE) {
            circle* c_it = nullptr;
            section* s_it = nullptr;
            try {
                c_it = &(*(m_circleIDs.at(r.objects[0])));
                s_it = &(*(m_sectionIDs.at(r.objects[1])));
            }catch (...){
                try {
                    c_it = &(*(m_circleIDs.at(r.objects[1])));
                    s_it = &(*(m_sectionIDs.at(r.objects[0])));
                }catch(...) {
                    m_reqD.remove(it);
                    throw std::runtime_error("No such circle or section");
                }
            }
            requirement = new ReqSecOnCircle(s_it, c_it);
        }

        // 7
        else if (r.req == ET_SECTIONINCIRCLE) {
            circle* c_it = nullptr;
            section* s_it = nullptr;
            try {
                c_it = &(*(m_circleIDs.at(r.objects[0])));
                s_it = &(*(m_sectionIDs.at(r.objects[1])));
            }catch (...){
                try {
                    c_it = &(*(m_circleIDs.at(r.objects[1])));
                    s_it = &(*(m_sectionIDs.at(r.objects[0])));
                }catch(...) {
                    m_reqD.remove(it);
                    throw std::runtime_error("No such circle or section");
                }
            }
            requirement = new ReqSecInCircle(s_it, c_it);
        }

        // 8
        else if (r.req == ET_SECTIONSECTIONPARALLEL) {
            section* s1_it = nullptr;
            section* s2_it = nullptr;
            try {
                s1_it = &(*(m_sectionIDs.at(r.objects[0])));
                s2_it = &(*(m_sectionIDs.at(r.objects[1])));
            }catch(...){
                m_reqD.remove(it);
                throw std::runtime_error("No such section");
            }
            requirement = new ReqSecSecParallel(s1_it, s2_it);
        }

        // 9
        else if (r.req == ET_SECTIONSECTIONPERPENDICULAR) {
            section* s1_it = nullptr;
            section* s2_it = nullptr;
            try {
                s1_it = &(*(m_sectionIDs.at(r.objects[0])));
                s2_it = &(*(m_sectionIDs.at(r.objects[1])));
            }catch(...){
                m_reqD.remove(it);
                throw std::runtime_error("No such section");
            }
            requirement = new ReqSecSecPerpendicular(s1_it, s2_it);
        }

        // 10
        else if (r.req == ET_SECTIONSECTIONANGEL) {
            section* s1_it = nullptr;
            section* s2_it = nullptr;
            try {
                s1_it = &(*(m_sectionIDs.at(r.objects[0])));
                s2_it = &(*(m_sectionIDs.at(r.objects[1])));
            }catch(...){
                m_reqD.remove(it);
                throw std::runtime_error("No such section");
            }
            requirement = new ReqSecSecAngel(s1_it, s2_it, r.params[0]);
        }


        if (requirement) {
            allRequirements.push_back(requirement);
        }
    }

    std::vector<Function*> allFunctions;
    for (auto requirement: allRequirements) {
        allFunctions.push_back(requirement->getFunction());
    }

    for (const auto& req: m_reqD){
        for (auto i: req.objects){
            info.m_objects.push_back(i);
            info.m_paramsBefore.push_back(getElementInfo(i).params);
        }
    }

    LSMTask* task = new LSMTask(allFunctions, VarsStorage::getVars());
    LMSolver solver;
    solver.setTask(task);
    solver.optimize();

    std::cout << "Component count of req: " << countOfReq << std::endl;

    if (!solver.isConverged() || solver.getCurrentError() > 1e-6){
        for (const auto& req: m_reqD){
            for (auto i: req.objects){
                info.m_paramsAfter.push_back(getElementInfo(i).params);
            }
        }
        for (const auto &req: allRequirements) {
            s_allFigures = s_allFigures || req->getRectangle();
        }
        // Clear
        for (auto requirement: allRequirements) {
            delete requirement;
        }
        info.m_objects.emplace_back(++s_maxID.id);
        c_undoRedo.add(info);
        m_reqIDs[s_maxID.id] = it;
        undo();
        throw std::runtime_error("Not converged");
    }
    for (const auto& req: m_reqD){
        for (auto i: req.objects){
            info.m_paramsAfter.push_back(getElementInfo(i).params);
        }
    }
    for (const auto &req: allRequirements) {
        s_allFigures = s_allFigures || req->getRectangle();
    }
    // Clear
    for (auto requirement: allRequirements) {
        delete requirement;
    }
    info.m_objects.emplace_back(++s_maxID.id);
    c_undoRedo.add(info);
    m_reqIDs[s_maxID.id] = it;
    return s_maxID;
}

ID Paint::addElement(const ElementData &ed) {
    ActionsInfo info;
    if (ed.et == ET_POINT) {
        point tmp;
        tmp.x = ed.params[0];
        tmp.y = ed.params[1];
        s_allFigures = s_allFigures || tmp.rect();
        m_pointIDs[++s_maxID.id] = m_pointStorage.addElement(tmp);
        m_graph.addVertex(s_maxID);
        info.m_objects.push_back(s_maxID);
        info.m_paramsAfter.push_back(ed.params);
        c_undoRedo.add(info);
        return s_maxID;
    }
    if (ed.et == ET_SECTION) {
        point tmp1;
        tmp1.x = ed.params[0];
        tmp1.y = ed.params[1];
        auto beg = m_pointStorage.addElement(tmp1);
        m_pointIDs[++s_maxID.id] = beg;
        m_graph.addVertex(s_maxID);
        std::vector<double> params1;
        params1.push_back(ed.params[0]);
        params1.push_back(ed.params[1]);
        info.m_objects.push_back(s_maxID);
        info.m_paramsAfter.push_back(params1);
        point tmp2;
        tmp2.x = ed.params[2];
        tmp2.y = ed.params[3];
        auto end = m_pointStorage.addElement(tmp2);
        m_pointIDs[++s_maxID.id] = end;
        m_graph.addVertex(s_maxID);
        std::vector<double> params2;
        params2.push_back(ed.params[2]);
        params2.push_back(ed.params[3]);
        info.m_objects.push_back(s_maxID);
        info.m_paramsAfter.push_back(params2);
        section tmp;
        tmp.beg = &(*beg);
        tmp.end = &(*end);
        s_allFigures = s_allFigures || tmp.rect();
        m_sectionIDs[++s_maxID.id] = m_sectionStorage.addElement(tmp);

        m_graph.addVertex(s_maxID);
        RequirementData sectionReq;
        sectionReq.objects.emplace_back(s_maxID.id);
        sectionReq.objects.emplace_back(s_maxID.id-2);
        sectionReq.req = ET_POINTINOBJECT;
        m_graph.addEdge(sectionReq, s_maxID.id, s_maxID.id-2);
        sectionReq.objects.pop_back();
        sectionReq.objects.emplace_back(s_maxID.id-1);
        m_graph.addEdge(sectionReq, s_maxID.id, s_maxID.id-1);

        info.m_objects.push_back(s_maxID);
        info.m_paramsAfter.push_back(ed.params);
        c_undoRedo.add(info);
        return s_maxID;
    }
    if (ed.et == ET_CIRCLE) {
        point center;
        center.x = ed.params[0];
        center.y = ed.params[1];
        std::vector<double> params1;
        params1.push_back(ed.params[0]);
        params1.push_back(ed.params[1]);
        info.m_paramsAfter.push_back(params1);
        auto cent = m_pointStorage.addElement(center);
        m_pointIDs[++s_maxID.id] = cent;

        m_graph.addVertex(s_maxID);

        info.m_objects.push_back(s_maxID);
        circle tmp;
        tmp.center = &(*cent);
        tmp.R = ed.params[2];
        s_allFigures = s_allFigures || tmp.rect();
        m_circleIDs[++s_maxID.id] = m_circleStorage.addElement(tmp);

        // add vertexes and edge to graph
        m_graph.addVertex(s_maxID);
        RequirementData circleReq;
        circleReq.objects.emplace_back(s_maxID.id);
        circleReq.objects.emplace_back(s_maxID.id-1);
        circleReq.req = ET_POINTINOBJECT;
        m_graph.addEdge(circleReq, s_maxID.id, s_maxID.id-1);

        info.m_objects.push_back(s_maxID);
        params1.push_back(ed.params[2]);
        info.m_paramsAfter.push_back(params1);
        c_undoRedo.add(info);
        return s_maxID;
    }
    return ID{-1};
}

ElementData Paint::getElementInfo(ID id) {
    ElementData result;

    try {
        auto p = m_pointIDs.at(id);
        result.et = ET_POINT;
        result.params.push_back((*p).x);
        result.params.push_back((*p).y);
    }
    catch (...) {
        try {
            auto sec = m_sectionIDs.at(id);
            result.et = ET_SECTION;
            result.params.push_back((*sec).beg->x);
            result.params.push_back((*sec).beg->y);
            result.params.push_back((*sec).end->x);
            result.params.push_back((*sec).end->y);
        }
        catch (...) {
            auto circ = m_circleIDs.at(id);
            result.et = ET_CIRCLE;
            result.params.push_back((*circ).center->x);
            result.params.push_back((*circ).center->y);
            result.params.push_back((*circ).R);
        }
    }

    return result;
}

RequirementData Paint::getRequirementInfo(ID id) {
    if (m_reqIDs.find(id) == m_reqIDs.end()) {
        throw std::invalid_argument("No such requirement!");
    }
    return *m_reqIDs[id];
}

void Paint::paint() {

    c_bmpPainter->changeSize(s_allFigures);
    for (auto &point: m_pointStorage) {
        c_bmpPainter->drawPoint(point, false);
    }
    for (auto &circle: m_circleStorage) {
        c_bmpPainter->drawCircle(circle, false);
    }
    for (auto &section: m_sectionStorage) {
        c_bmpPainter->drawSection(section, false);
    }
}

void Paint::exportToBMP(const char *file) {
    paint();
    try {
        BMPpainter b;
        b.changeSize(s_allFigures);
        for (auto &point: m_pointStorage) {
            b.drawPoint(point, false);
        }
        for (auto &circle: m_circleStorage) {
            b.drawCircle(circle, false);
        }
        for (auto &section: m_sectionStorage) {
            b.drawSection(section, false);
        }
        b.saveBMP(file);
    }
    catch (...) {
        throw std::invalid_argument("Can not opened file!");
    }
}

/*
void Paint::makeMySectionOrt(const ElementData& ed, ElementData& changing){
    if (ed.et != ET_SECTION or changing.et != ET_SECTION) {
        throw "Some of the elements is not section!";
    }
    //изменяем changing так, чтобы он был ортогонален с ed(cкорее всего, поворотом одной из точек)
}
void Paint::makeMySectionEqual(const ElementData& ed, ElementData& changing) {
    if (ed.et != ET_SECTION or changing.et != ET_SECTION) {
        throw "Some of the elements is not section!";
    }
    //изменяем changing так, чтобы его длина была равна длине ed (cкорее всего, поворотом одной из точек)
    //например поменяем Y второй точки
    double len2 = pow(ed.point1.x - ed.point2.x, 2) + pow(ed.point1.x - ed.point2.x, 2)
    changing.point1.y = sqrt(len2-pow(changing.point1.x -changing.point2.x, 2))+ changing.point1.y
}
void Paint::makeMySectionParallel(const ElementData& ed, ElementData& changing) {
    if (ed.et != ET_SECTION or changing.et != ET_SECTION) {
        throw "Some of the elements is not section!";
    }
    //изменяем changing так, чтобы он стал параллелен ed
}
void Paint::makeMySectionVertical(ElementData& changing) {
    if (changing.et != ET_SECTION) {
        throw "The element is not section!";
    }
    //отрезок становится строго вертикальным
    changing.point2.x=changing.point1.x; - чтобы были равны X координаты точек
}
void Paint::makeMySectionHorizontal(ElementData& changing) {
    if (changing.et != ET_SECTION) {
        throw "The element is not section!";
    }
    //отрезок становится строго горизонтальным2
    changing.point2.y=changing.point1.y; - чтобы были равны Y координаты точек
}
void Paint::makeMyCircleEqual(const ElementData& ed, ElementData& changing) {
    if (ed.et != ET_CIRCLE or changing.et != ET_CIRCLE) {
        throw "Some of the elements is not Circle!";
    }
    //окружности становятся одинакового размера
    changing.radius = ed.radius;
}
*/





void Paint::deleteRequirement(ID req) {
    if (!m_reqIDs.contains(req)) {
        throw std::invalid_argument("No such requirement!");
    }
    m_reqD.remove(m_reqIDs[req]);
    m_reqIDs.erase(req);
}

void Paint::undo() {
    ActionsInfo info = c_undoRedo.undo();
    point *p = nullptr;
    section *s = nullptr;
    circle *c = nullptr;
    if (info.m_paramsBefore.empty()) {
        int i = 0;
        while (!info.m_objects.empty()) {
            if (m_pointIDs.contains(info.m_objects[i])) {
                m_pointStorage.remove(m_pointIDs[info.m_objects[i]]);
                m_pointIDs.erase(info.m_objects[i]);
            } else if (m_sectionIDs.contains(info.m_objects[i])) {
                m_sectionStorage.remove(m_sectionIDs[info.m_objects[i]]);
                m_sectionIDs.erase(info.m_objects[i]);
            } else if (m_circleIDs.contains(info.m_objects[i])) {
                m_circleStorage.remove(m_circleIDs[info.m_objects[i]]);
                m_circleIDs.erase(info.m_objects[i]);
            } else {
                std::cout << "No ID to undo" << std::endl;
                break;
            }
            ++i;
        }
        return;
    }
    for (int i = 0; i < info.m_objects.size(); ++i) {
        try {
            p = &(*m_pointIDs.at(info.m_objects[i]));
            p->x = info.m_paramsBefore[i][0];
            p->y = info.m_paramsBefore[i][1];
        } catch (...) {
            try {
                s = &(*m_sectionIDs.at(info.m_objects[i]));
                s->beg->x = info.m_paramsBefore[i][0];
                s->beg->y = info.m_paramsBefore[i][1];
                s->end->x = info.m_paramsBefore[i][2];
                s->end->y = info.m_paramsBefore[i][3];
            } catch (...) {
                try {
                    c = &(*m_circleIDs.at(info.m_objects[i]));
                    c->center->x = info.m_paramsBefore[i][0];
                    c->center->y = info.m_paramsBefore[i][1];
                } catch (...) {
                    try{
                        m_reqD.remove(m_reqIDs.at(info.m_objects[i]));
                        m_reqIDs.erase(info.m_objects[i]);
                    } catch (...) {
                        std::cout << "No ID to redo" << std::endl;
                        break;
                    }
                }
            }
        }
    }
}

void Paint::redo() {
    ActionsInfo info = c_undoRedo.redo();
    point *p = nullptr;
    section *s = nullptr;
    circle *c = nullptr;
    if (info.m_paramsBefore.size() == 0) {
        if (info.m_objects.size() == 3) {
            point beg;
            beg.x = info.m_paramsAfter[0][0];
            beg.y = info.m_paramsAfter[0][1];
            auto p1 = m_pointStorage.addElement(beg);
            m_pointIDs[info.m_objects[0]] = p1;
            point end;
            end.x = info.m_paramsAfter[1][0];
            end.y = info.m_paramsAfter[1][1];
            auto p2 = m_pointStorage.addElement(end);
            m_pointIDs[info.m_objects[1]] = p2;
            section sec;
            sec.beg = &(*(p1));
            sec.end = &(*(p2));
            m_sectionIDs[info.m_objects[2]] = m_sectionStorage.addElement(sec);
            s_allFigures = s_allFigures || sec.rect();
        } else if (info.m_objects.size() == 2) {
            point center;
            center.x = info.m_paramsAfter[0][0];
            center.y = info.m_paramsAfter[0][1];
            circle circ;
            auto p1 = m_pointStorage.addElement(center);
            m_pointIDs[info.m_objects[0]] = p1;
            circ.center = &(*(p1));
            circ.R = info.m_paramsAfter[1][2];
            m_circleIDs[info.m_objects[1]] = m_circleStorage.addElement(circ);
            s_allFigures = s_allFigures || circ.rect();
        } else if (info.m_objects.size() == 1) {
            point pt;
            pt.x = info.m_paramsAfter[0][0];
            pt.y = info.m_paramsAfter[0][1];
            m_pointIDs[info.m_objects[0]] = m_pointStorage.addElement(pt);
            s_allFigures = s_allFigures || pt.rect();
        }
        return;
    }
    for (int i = 0; i < info.m_objects.size(); ++i) {
        try {
            p = &(*m_pointIDs.at(info.m_objects[i]));
            p->x = info.m_paramsAfter[i][0];
            p->y = info.m_paramsAfter[i][1];
        } catch (...) {
            try {
                s = &(*m_sectionIDs.at(info.m_objects[i]));
                s->beg->x = info.m_paramsAfter[i][0];
                s->beg->y = info.m_paramsAfter[i][1];
                s->end->x = info.m_paramsAfter[i][2];
                s->end->y = info.m_paramsAfter[i][3];
            } catch (...) {
                try {
                    c = &(*m_circleIDs.at(info.m_objects[i]));
                    c->center->x = info.m_paramsAfter[i][0];
                    c->center->y = info.m_paramsAfter[i][1];
                } catch (...) {
                    std::cout << "No ID to redo" << std::endl;
                    break;
                }
            }
        }
    }
}

void Paint::saveToFile(const char *filename) {
    FileOurP saver;
    for (auto i = m_pointIDs.begin(); i != m_pointIDs.end(); ++i) {
        point *p = &(*i->second);
        std::pair<ID, primitive *> m{i->first, p};
        saver.addObject(m);
    }
    for (auto i = m_sectionIDs.begin(); i != m_sectionIDs.end(); ++i) {
        section *s = &(*i->second);
        std::pair<ID, primitive *> m{i->first, s};
        saver.addObject(m);
    }
    for (auto i = m_circleIDs.begin(); i != m_circleIDs.end(); ++i) {
        circle *c = &(*i->second);
        std::pair<ID, primitive *> m{i->first, c};
        saver.addObject(m);
    }
    for (auto i = m_reqIDs.begin(); i != m_reqIDs.end(); ++i) {
        std::pair<ID, RequirementData> m = {i->first, *i->second};
        saver.addRequirement(m);
    }
    saver.saveToOurP(filename);
}

void Paint::loadFromFile(const char *file) {
    FileOurP loader;
    loader.loadFromOurP(file);
    clear();
    for (const auto &i: loader.getObjects()) {
        if (i.to_pair().second->type() == ET_POINT) {
            point *p = static_cast<point *>(i.to_pair().second);
            m_pointIDs[i.to_pair().first] = m_pointStorage.addElement(*p);
            s_allFigures = s_allFigures || p->rect();
        } else if (i.to_pair().second->type() == ET_CIRCLE) {
            circle *c = static_cast<circle *>(i.to_pair().second);
            m_circleIDs[i.to_pair().first] = m_circleStorage.addElement(*c);
            s_allFigures = s_allFigures || c->rect();
        } else if (i.to_pair().second->type() == ET_SECTION) {
            section *s = static_cast<section *>(i.to_pair().second);
            m_sectionIDs[i.to_pair().first] = m_sectionStorage.addElement(*s);
            s_allFigures = s_allFigures || s->rect();
        }
    }
    for (const auto &i: loader.getRequirements()) {
        auto it = m_reqD.addElement(i.to_pair().second);
        m_reqIDs[i.to_pair().first] = it;
    }
}

void Paint::clear() {
    m_pointIDs.clear();
    m_sectionIDs.clear();
    m_circleIDs.clear();
    m_reqIDs.clear();
    for (auto i = m_reqD.begin(); i != m_reqD.end(); ++i) {
        m_reqD.remove(i);
    }
    for (auto i = m_pointStorage.begin(); i != m_pointStorage.end(); ++i) {
        m_pointStorage.remove(i);
    }
    for (auto i = m_sectionStorage.begin(); i != m_sectionStorage.end(); ++i) {
        m_sectionStorage.remove(i);
    }
    for (auto i = m_circleStorage.begin(); i != m_circleStorage.end(); ++i) {
        m_circleStorage.remove(i);
    }
    s_allFigures = rectangle(10, 10, 10, 10);
    s_maxID = ID(0);

}

std::vector<std::pair<ID, ElementData>> Paint::getAllElementsInfo() {
    std::vector<std::pair<ID, ElementData>> data;
    for (auto &m_pointID: m_pointIDs) {
        point *p = &(*m_pointID.second);
        ElementData info;
        info.et = ET_POINT;
        info.params.push_back(p->x);
        info.params.push_back(p->y);
        data.emplace_back(m_pointID.first, info);
    }
    for (auto &m_sectionID: m_sectionIDs) {
        section *s = &(*m_sectionID.second);
        ElementData info;
        info.et = ET_SECTION;
        info.params.push_back(s->beg->x);
        info.params.push_back(s->beg->y);
        info.params.push_back(s->end->x);
        info.params.push_back(s->end->y);
        data.emplace_back(m_sectionID.first, info);
    }
    for (auto &m_circleID: m_circleIDs) {
        circle *c = &(*m_circleID.second);
        ElementData info;
        info.et = ET_CIRCLE;
        info.params.push_back(c->center->x);
        info.params.push_back(c->center->y);
        info.params.push_back(c->R);
        data.emplace_back(m_circleID.first, info);
    }
    return data;
}

void Paint::setPainter(Painter *p) {
    c_bmpPainter = p;
}

void Paint::loadFromString(const std::string &str) {
    FileOurP loader;
    loader.loadString(str);
    clear();
    for (const auto &i: loader.getObjects()) {
        if (i.to_pair().second->type() == ET_POINT) {
            point *p = static_cast<point *>(i.to_pair().second);
            m_pointIDs[i.to_pair().first] = m_pointStorage.addElement(*p);
            s_allFigures = s_allFigures || p->rect();
        } else if (i.to_pair().second->type() == ET_CIRCLE) {
            circle *c = static_cast<circle *>(i.to_pair().second);
            m_circleIDs[i.to_pair().first] = m_circleStorage.addElement(*c);
            s_allFigures = s_allFigures || c->rect();
        } else if (i.to_pair().second->type() == ET_SECTION) {
            section *s = static_cast<section *>(i.to_pair().second);
            m_sectionIDs[i.to_pair().first] = m_sectionStorage.addElement(*s);
            s_allFigures = s_allFigures || s->rect();
        }
    }
    for (const auto &m_reqID: loader.getRequirements()) {
        m_reqIDs[m_reqID.to_pair().first] = m_reqD.addElement(m_reqID.to_pair().second);
    }
}

std::string Paint::to_string() const {
    FileOurP saver;
    for (const auto &m_pointID: m_pointIDs) {
        point *p = &(*m_pointID.second);
        std::pair<ID, primitive *> m{m_pointID.first, p};
        saver.addObject(m);
    }
    for (const auto &m_sectionID: m_sectionIDs) {
        section *s = &(*m_sectionID.second);
        std::pair<ID, primitive *> m{m_sectionID.first, s};
        saver.addObject(m);
    }
    for (const auto &m_circleID: m_circleIDs) {
        circle *c = &(*m_circleID.second);
        std::pair<ID, primitive *> m{m_circleID.first, c};
        saver.addObject(m);
    }
    for (auto &m_reqID: m_reqIDs) {
        std::pair<ID, RequirementData> m{m_reqID.first, *m_reqID.second};
        saver.addRequirement(m);
    }
    return saver.to_string();
}

std::vector<std::pair<ID, RequirementData>> Paint::getAllRequirementsInfo() {
    std::vector<std::pair<ID, RequirementData>> data;
    for (auto req: m_reqIDs) {
        data.emplace_back(req.first, *req.second);
    }
    return data;
}

