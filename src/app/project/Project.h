#ifndef OURPAINT_APPLICATION_PROJECT_H_
#define OURPAINT_APPLICATION_PROJECT_H_

#include <cstdint>
#include <filesystem>
#include <vector>

namespace app {

class Project {
public:
    Project(std::string name);

    std::string name() const;

    const std::vector<DocumentId>& documents() const;

    void addDocument(DocumentId id);
    void removeDocument(DocumentId id);

private:
    std::string name_;
    std::filesystem::path projectPath_;

    std::vector<DocumentId> documents_;
};

}

#endif // ! OURPAINT_APPLICATION_PROJECT_H_