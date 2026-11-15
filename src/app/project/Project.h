#ifndef OURPAINT_APPLICATION_PROJECT_H_
#define OURPAINT_APPLICATION_PROJECT_H_

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

class Document;

namespace app {

struct ProjectMetadata {
    // Empty for an unsaved description; published IDs are 32 lowercase hex digits.
    std::string id;
    std::string name;
};

struct ProjectDocument {
    std::filesystem::path filename;
    std::reference_wrapper<const Document> document;
};

// Borrowed description, not another editable document model. Documents must
// outlive every operation using the description. Filenames do not track names.
struct Project {
    std::filesystem::path root;
    ProjectMetadata metadata;
    std::vector<ProjectDocument> documents;
};

}  // namespace app

#endif  // OURPAINT_APPLICATION_PROJECT_H_
