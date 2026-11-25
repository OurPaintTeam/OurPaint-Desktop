#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "DocumentManager.h"
#include "FileResult.h"
#include "IUIObserver.h"
#include "render/ViewportStyle.h"

namespace app {
using DocumentId = core::DocumentId;
inline constexpr DocumentId InvalidDocumentId = core::InvalidDocumentId;
class DocumentView;
class ViewportController;

// The only frontend entry point for document lifetime, file I/O and tool commands.
// Documents live independently of their views; each view owns its camera/editor/viewport.
// Frontends own windows and graphics contexts, and borrow viewports only through this API.
// A new document exists only in memory. Saving is explicit; closing never writes a file.
// There are no projects. The application-wide .ourpaint file contains metadata, not CAD data.
class UIController {
public:
    UIController();
    ~UIController();
    UIController(const UIController&) = delete;
    UIController& operator=(const UIController&) = delete;

    // Optional, synchronous and non-owning. Unsubscribe before destroying the observer.
    void setObserver(IUIObserver* observer) { observer_ = observer; }

    // Shared by all current and future views. Applying a style requests a redraw of every open view.
    const ViewportStyle& viewportStyle() const noexcept { return viewportStyle_; }
    void setViewportStyle(const ViewportStyle& style);

    DocumentId createDocument(std::string name);
    FileResult<DocumentId> openDocument(const std::filesystem::path& path);
    // The caller asks about unsaved changes first. All views close before the document.
    bool closeDocument(DocumentId document);
    const std::vector<DocumentId>& documents() const { return documents_; }
    std::string documentName(DocumentId document) const;
    bool renameDocument(DocumentId document, const std::string& name);
    std::filesystem::path documentPath(DocumentId document) const;
    // Compares against the last saved/loaded state, or the initial empty draft.
    // Intended for close/save decisions, not for every render frame.
    bool documentModified(DocumentId document) const;
    FileResult<std::filesystem::path> saveDocument(DocumentId document);
    FileResult<std::filesystem::path> saveDocumentAs(DocumentId document, const std::filesystem::path& path);

    // Every call opens an independent view. Stable IDs are never recycled.
    ViewId createDocumentView(DocumentId document);
    // Borrowed until this view/document closes or this controller is destroyed.
    ViewportController* findViewport(ViewId view);
    void closeDocumentView(ViewId view);
    void clearDocumentViews();
    void setActiveDocumentView(ViewId view);
    ViewId activeDocumentView() const { return active_; }
    DocumentId documentOf(ViewId view) const;
    std::vector<ViewId> documentViews(DocumentId document) const;
    void selectTool(ViewId view, ToolId tool, std::optional<double> value = {});
    bool canToggleConstruction(ViewId view) const;
    std::optional<ToolId> activeToolId(ViewId view) const;
    void endInteractions();

    // Frontends supply the platform's user-config path. Missing metadata is normal.
    // Only explicitly opened/saved paths enter this list; drafts are never persisted.
    std::optional<FileError> loadApplicationMetadata(const std::filesystem::path& path);
    const std::vector<std::filesystem::path>& recentDocuments() const { return recentDocuments_; }
    const std::optional<FileError>& metadataError() const { return metadataError_; }

    int getDOFs(DocumentId id) const;

private:
    struct DocumentState {
        std::filesystem::path path;
        core::persistence::DocumentBytes savedState;
    };
    DocumentId addDocument(std::unique_ptr<core::Document> document, std::filesystem::path path = {});
    void rememberDocument(const std::filesystem::path& path);
    void requestRedraw(ViewId view);

    core::DocumentManager manager_;
    std::vector<DocumentId> documents_;
    std::map<DocumentId, DocumentState> documentStates_;
    ViewportStyle viewportStyle_ = ViewportStyle::makeDefault();
    std::map<ViewId, std::unique_ptr<DocumentView>> views_;
    ViewId active_ = InvalidViewId;
    uint64_t nextViewId_ = 1;
    IUIObserver* observer_ = nullptr;
    std::filesystem::path metadataPath_;
    std::vector<std::filesystem::path> recentDocuments_;
    std::optional<FileError> metadataError_;
};
}  // namespace app

#endif
