#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../core/DocumentManager.h"
#include "../project/ProjectSaveLoad.h"
#include "tool/ToolId.h"

class DocumentView;
class ViewportController;

// Application entry point for project, document and editor commands and queries.
// Each instance owns a workspace: documents outlive their views, and each view owns its
// editor and viewport controller. Frontends use this API instead of reaching into a view.
// Keep windowing, dialogs, toolkit types and presentation in the frontend. Optional
// synchronous callbacks publish application changes; Core never calls into a frontend.
class UIController {
public:
    struct Callbacks {
        // Hosts borrow each viewport from successful attachment until detachment.
        // A failed attachment must leave no borrowed reference behind.
        std::function<bool(ViewportController&)> attachViewport;
        std::function<void(ViewportController&)> detachViewport;
        std::function<void(const std::string&)> tabAdded;
        std::function<void(const std::string&)> tabRemoved;
        std::function<void(const std::string&, ViewportController&)> activeChanged;
        std::function<void(const std::string&, ToolId, bool)> toolChanged;
        std::function<void()> redraw;
    };

    UIController();
    ~UIController();
    UIController(const UIController&) = delete;
    UIController& operator=(const UIController&) = delete;

    // Install before opening views. Detach live views while the receiving host still exists.
    void setCallbacks(Callbacks callbacks);
    void clearViews();

    void selectTool(ToolId tool, std::optional<double> value = {}, const std::string& tabName = {});
    std::optional<ToolId> activeToolId() const;
    void executeConsoleCommand(const std::string& command);
    void previewConsoleSelection(const std::vector<int>& ids);

    void createFile(const std::string& fileName);
    void setActiveFile(const std::string& fileName);
    // Close the view. Its document stays in the project and can be reopened.
    void removeFile(const std::string& fileName);
    bool renameTab(const std::string& oldName, const std::string& newName);

    app::SaveReport saveProject();
    app::SaveReport saveProjectAs(const std::filesystem::path& destination);
    app::SaveReport createProject(const std::filesystem::path& destination);
    std::optional<app::FileError> openProject(const std::filesystem::path& path);
    std::optional<app::FileError> openFile(const std::filesystem::path& path);
    bool needsSaveConfirmation() const;
    void endInteractions();
    const std::filesystem::path& projectRoot() const;
    const app::ProjectMetadata& projectMetadata() const;

private:
    DocumentView* findView(const std::string& name) const;
    void attachDocument(DocumentId id, const std::string& tabName);
    void installProject(app::LoadedProject loaded);
    void presentActiveTool(DocumentId id, ToolId tool);
    std::string uniqueTabName(const std::string& preferred) const;
    std::filesystem::path nextFilename(DocumentId id) const;

    DocumentManager manager_;
    std::vector<std::unique_ptr<DocumentView>> views_;
    app::Project project_{{}, {{}, "Untitled"}, {}};
    bool projectModified_ = true;
    std::unordered_map<std::string, DocumentId> documentIds_;
    std::string activeTabName_;
    Callbacks callbacks_;
};

#endif  // OURPAINT_APPLICATION_UI_CONTROLLER_H_
