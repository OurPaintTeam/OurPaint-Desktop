#ifndef OURPAINT_APPLICATION_PROJECT_H_
#define OURPAINT_APPLICATION_PROJECT_H_

#include <cstdint>
#include <filesystem>
#include <vector>

namespace app {

using ProjectId = uint64_t;
using DocumentId = uint64_t;

class Project {
public:
    Project(ProjectId id, std::string name);

    ProjectId id() const;
    std::string name() const;

    const std::vector<DocumentId>& documents() const;

    void addDocument(DocumentId id);
    void removeDocument(DocumentId id);

private:
    ProjectId id_;
    std::string name_;
    std::filesystem::path projectPath_;

    std::vector<DocumentId> documents_;
};

}

#endif // ! OURPAINT_APPLICATION_PROJECT_H_