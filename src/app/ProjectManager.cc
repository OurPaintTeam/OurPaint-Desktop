#include "ProjectManager.h"

ProjectManager::ProjectManager() {}
ProjectId ProjectManager::createProject() {}
ProjectId ProjectManager::openProject(const std::filesystem::path& path) {}
void ProjectManager::closeProject(ProjectId id) {}
void ProjectManager::deleteProject(ProjectId id) {}
DocumentId ProjectManager::createDocument(ProjectId projectId) {}
void ProjectManager::attachDocument(ProjectId projectId, DocumentId documentId) {}
void ProjectManager::detachDocument(ProjectId projectId, DocumentId documentId) {}