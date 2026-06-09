#ifndef OURPAINT_APPLICATION_PROJECT_MANAGER_H_
#define OURPAINT_APPLICATION_PROJECT_MANAGER_H_

#include "Project.h"
class DocumentManager;

class ProjectManager {
public:
    explicit ProjectManager();

    ProjectId createProject(/*ProjectCreateInfo info*/);
    ProjectId openProject(const std::filesystem::path& path);
    void closeProject(ProjectId id);
    void deleteProject(ProjectId id);

    DocumentId createDocument(ProjectId projectId/*, DocumentCreateInfo info*/);
    void attachDocument(ProjectId projectId, DocumentId documentId);
    void detachDocument(ProjectId projectId, DocumentId documentId);

private:
    DocumentManager& documentManager_;
    std::vector<Project> projects_;
};

#endif // ! OURPAINT_APPLICATION_PROJECT_MANAGER_H_