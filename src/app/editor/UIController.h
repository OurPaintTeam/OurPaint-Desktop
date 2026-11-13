#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../../core/DocumentManager.h"
#include "../project/ProjectSaveLoad.h"
#include "IUIObserver.h"

class DocumentView;
class IViewportController;

// Application entry point for project, document and tool commands. A project owns
// documents independently of any views. Each open view has its own editor, camera
// and viewport controller. Frontends own windowing, host attachment and presentation.
class UIController {
public:
    UIController();
    ~UIController();
    UIController(const UIController&) = delete;
    UIController& operator=(const UIController&) = delete;

    // Non-owning. Clear the subscription before the observer is destroyed.
    void setObserver(IUIObserver* observer) { observer_ = observer; }

    DocumentId createDocument(std::string name);
    // Every call creates another view of the existing document. Invalid IDs fail
    // without side effects. IDs issued by this controller are never recycled.
    ViewId createDocumentView(DocumentId document);
    // Borrowed until closeView, project replacement or controller destruction.
    IViewportController* findViewport(ViewId view);
    void closeDocumentView(ViewId view);
    void clearDocumentViews();
    void setActiveDocumentView(ViewId view);
    ViewId activeDocumentView() const { return active_; }
    DocumentId documentOf(ViewId view) const;
    const std::vector<DocumentId>& documents() const { return documents_; }
    std::string documentName(DocumentId document) const;
    bool renameDocument(DocumentId document, const std::string& name);

    void selectTool(ViewId view, ToolId tool, std::optional<double> value = {});
    std::optional<ToolId> activeToolId(ViewId view) const;

    // void executeConsoleCommand(const std::string& command);
    // void previewConsoleSelection(const std::vector<int>& ids);

    app::SaveReport saveProject();
    app::SaveReport saveProjectAs(const std::filesystem::path& destination);
    app::SaveReport createProject(const std::filesystem::path& destination);
    std::optional<app::FileError> openProject(const std::filesystem::path& path);
    app::FileResult<DocumentId> openFile(const std::filesystem::path& path);
    bool needsSaveConfirmation() const;
    void endInteractions();
    const std::filesystem::path& projectRoot() const;
    const app::ProjectMetadata& projectMetadata() const;

private:
    DocumentId addDocument(std::unique_ptr<Document> document);
    void installProject(app::LoadedProject loaded);
    std::filesystem::path nextFilename(DocumentId id) const;
    void requestRedraw(ViewId view);

    DocumentManager manager_;
    std::vector<DocumentId> documents_;
    std::map<ViewId, std::unique_ptr<DocumentView>> views_;
    ViewId active_ = InvalidViewId;
    uint64_t nextViewId_ = 1;
    IUIObserver* observer_ = nullptr;
    app::Project project_{{}, {{}, "Untitled"}, {}};
    bool projectModified_ = true;
};
