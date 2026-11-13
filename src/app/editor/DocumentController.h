#ifndef OURPAINT_DOCUMENTCONTROLLER_H
#define OURPAINT_DOCUMENTCONTROLLER_H

#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "DocumentManager.h"
#include "DocumentView.h"


class DocumentController {
public:
    struct ActivatedView {
        DocumentId id;
        ToolId activeToolId;
        int controllerIndex;
    };

    static constexpr DocumentId InvalidId = std::numeric_limits<DocumentId>::max();

    explicit DocumentController(DocumentManager& manager, std::vector<std::unique_ptr<DocumentView>>& views);
    ~DocumentController() = default;

    DocumentId loadDocument(const std::string& path);
    DocumentId createDocument(const std::string& path);
    bool saveDocument(DocumentId id);
    bool saveDocumentAs(DocumentId id, const std::string& newPath);
    bool closeDocument(DocumentId id);
    bool renameDocument(DocumentId id, const std::string& newName);

    DocumentId idFromPath(const std::string& path) const;
    std::string pathOf(DocumentId id) const;
    std::string nameOf(DocumentId id) const;
    bool isOpened(DocumentId id) const;
    static std::size_t pathHash(const std::string& path);

    DocumentManager& manager() { return manager_; }

    int registerView(std::unique_ptr<DocumentView> view, int controllerIndex);
    std::unique_ptr<DocumentView> extractView(DocumentId id, int* outRemovedIndex);
    void clearViews();

    const std::vector<std::unique_ptr<DocumentView>>& views() const { return views_; }

    DocumentId selectTool(std::optional<DocumentId> id, ToolId tool, std::optional<double> value);
    std::optional<ActivatedView> activateByPath(const std::string& path);

    std::size_t activeTabHash() const { return activeTabHash_; }
    void setActiveTabHash(std::size_t hash);
    void clearActiveTabHashIf(std::size_t hash);

private:
    DocumentView* findView(const std::string& path) const;
    DocumentView* findViewByHash(std::size_t hash) const;
    DocumentView* findViewById(DocumentId id) const;

    int indexOfHash(std::size_t hash) const;
    void setIndexOfHash(std::size_t hash, int index);
    void eraseHashIndex(std::size_t hash);
    void shiftIndicesAfterRemoval(int removedIndex);

private:
    DocumentManager& manager_;
    std::unordered_map<std::size_t, DocumentId> documentIds_;

    std::vector<std::unique_ptr<DocumentView>>& views_;
    std::size_t activeTabHash_{0};

    std::unordered_map<std::size_t, int> indicesMap_;
    std::unordered_map<ViewportController*, int> controllerMap_;
};

#endif  // OURPAINT_DOCUMENTCONTROLLER_H