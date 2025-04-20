#include "FileOurP.h"

void FileOurP::parseFile(std::istream &file) {
    std::vector<objectInFile> objects;
    std::vector<requirementInFile> requirements;
    std::string line;
    std::queue<std::pair<unsigned int, IGeometricObject *>> q;
    std::queue<std::pair<unsigned int, RequirementData>> q2;
    while (std::getline(file, line)) {
        if (file.fail()) {
            throw std::runtime_error("Failed to read line from file");
        }
        if (line.find("ID") != std::string::npos) {
            std::stringstream ss(line);
            std::string id_str;
            long long id;
            ss >> id_str >> id;

            unsigned int object_id(id);
            std::getline(file, line);

            std::stringstream data(line);
            std::string type;
            data >> type;
            if (type == "addreq") {
                data >> type;
                long long int i1, i2;
                data >> i1;
                data >> i2;
                unsigned int id1(i1);
                unsigned int id2(i2);
                double param;
                data >> param;
                RequirementData req;
                req.req = static_cast<enum Requirement>(type.c_str()[0] - '0');
                req.objects.push_back(id1);
                req.objects.push_back(id2);
                req.params.push_back(param);
                std::pair<unsigned int, RequirementData> ll = {object_id, req};
                requirements.emplace_back(ll);
            } else if (type == "point") {
                double x, y;
                data >> x >> y;
                Point *p = new Point;
                p->x = x;
                p->y = y;
                std::pair<unsigned int, IGeometricObject *> a = {object_id, p};
                if (q.size() == 2) {
                    objects.emplace_back(q.front());
                    q.pop();
                }
                q.push(a);

            } else if (type == "section") {
                if (q.size() != 2) {
                    throw std::runtime_error("Invalid file format. Check instruction on rules.md");
                }

                IGeometricObject *p1 = q.front().second;
                objects.emplace_back(q.front());
                q.pop();
                IGeometricObject *p2 = q.front().second;
                objects.emplace_back(q.front());
                q.pop();
                Section *sec = new Section;
                sec->beg = dynamic_cast<Point *>(p1);
                sec->end = dynamic_cast<Point *>(p2);
                std::pair<unsigned int, IGeometricObject *> a = {object_id, sec};
                objects.emplace_back(a);
            } else if (type == "circle") {
                double r;
                data >> r;
                IGeometricObject *center = q.front().second;
                objects.emplace_back(q.front());
                q.pop();
                Circle *c = new Circle;
                c->center = dynamic_cast<Point *>(center);
                c->r = r;
                std::pair<unsigned int, IGeometricObject *> a = {object_id, c};
                objects.emplace_back(a);;
            }
        }
    }

    m_objects = objects;
    m_requirements = requirements;
}

FileOurP::FileOurP() {
    m_objects = std::vector<objectInFile>();
}

FileOurP::FileOurP(FileOurP &&other) noexcept {
    m_objects = std::move(other.m_objects);
}

FileOurP::FileOurP(const FileOurP &other) = default;

FileOurP &FileOurP::operator=(const FileOurP &other) = default;

FileOurP &FileOurP::operator=(FileOurP &&other) noexcept {
    m_objects = std::move(other.m_objects);
    return *this;
}

void FileOurP::addObject(std::pair<unsigned int, IGeometricObject *> &obj) {
    m_objects.emplace_back(obj);
}

std::string FileOurP::to_string() const {
    std::stringstream ss;
    std::vector<objectInFile> sort_objects = m_objects;
    std::vector<requirementInFile> sort_requirements = m_requirements;
    std::sort(sort_objects.begin(), sort_objects.end(), [](const objectInFile &a, const objectInFile &b) {
        return a.to_pair().first < b.to_pair().first;
    });
    std::sort(sort_requirements.begin(), sort_requirements.end(), [](const requirementInFile &a, const requirementInFile &b) {
        return a.to_pair().first < b.to_pair().first;
    });
    ss << "Elements: {\n";
    for (const auto &obj: sort_objects) {
        ss << obj.to_string() << "\n";
    }
    ss << "}\n";
    ss << "Requirements: {\n";
    for (const auto &req: sort_requirements) {
        ss << req.to_string() << "\n";
    }
    ss << "}\n";
    return ss.str();
}

void FileOurP::saveToOurP(const std::string &fileName) const {
    std::ofstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("File not found");
    }
    std::vector<objectInFile> sort_objects = m_objects;
    std::vector<requirementInFile> sort_requirements = m_requirements;
    std::sort(sort_objects.begin(), sort_objects.end(), [](const objectInFile &a, const objectInFile &b) {
        return a.to_pair().first < b.to_pair().first;
    });
    std::sort(sort_requirements.begin(), sort_requirements.end(),
              [](requirementInFile &a, const requirementInFile &b) {
                  return a.to_pair().first < b.to_pair().first;
              });
    file << "Elements: {\n";
    for (const auto &obj: sort_objects) {
        file << obj.to_string() << "\n";
    }
    file << "}\n";
    file << "Requirements: {\n";
    for (auto& req: sort_requirements) {
        file << req.to_string() << "\n";
    }
    file << "}\n";
    file.close();
}

void FileOurP::loadFromOurP(const std::string &fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("File not found");
    }
    parseFile(file);
}

const std::vector<objectInFile> &FileOurP::getObjects() const {
    return m_objects;
}

const std::vector<requirementInFile> &FileOurP::getRequirements() const {
    return m_requirements;
}

void FileOurP::loadString(const std::string &s) {
    std::stringstream ss;
    ss << s;
    parseFile(ss);
}

FileOurP::FileOurP(const std::vector<objectInFile> &obj, std::vector<requirementInFile> &req) {
    m_objects = obj;
    m_requirements = req;
}

void FileOurP::addRequirement(std::pair<unsigned int, RequirementData> &req) {
    m_requirements.emplace_back(req);
}
