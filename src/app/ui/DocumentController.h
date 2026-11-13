#ifndef OURPAINT_DOCUMENTCONTROLLER_H
#define OURPAINT_DOCUMENTCONTROLLER_H

#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../core/DocumentManager.h"
#include "../project/DocumentView.h"

class QtViewportHost;

class DocumentController {
public:
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

    std::size_t pathHash(const std::string& path) const;

    DocumentManager& manager() { return manager_; }

    std::vector<std::unique_ptr<DocumentView>>& views() { return views_; }
    const std::vector<std::unique_ptr<DocumentView>>& views() const { return views_; }

    DocumentView* findView(const std::string& path) const;
    DocumentView* findViewByHash(std::size_t hash) const;

    int addView(std::unique_ptr<DocumentView> view, QtViewportHost* host);
    bool removeView(DocumentId id, QtViewportHost* host);

    std::size_t activeTabHash() const { return activeTabHash_; }
    void setActiveTabHash(std::size_t hash) { activeTabHash_ = hash; }
    void clearActiveTabHashIf(std::size_t hash) {
        if (activeTabHash_ == hash) {
            activeTabHash_ = 0;
        }
    }

    void shiftIndicesAfterRemoval(int removedIndex);

    int indexOfHash(std::size_t hash) const;
    void setIndexOfHash(std::size_t hash, int index);
    void eraseHashIndex(std::size_t hash);

    int  registerView(std::unique_ptr<DocumentView> view);
    bool unregisterView(DocumentId id, int* outRemovedIndex);
private:
    DocumentManager& manager_;
    std::unordered_map<std::size_t, DocumentId> documentIds_;

    std::vector<std::unique_ptr<DocumentView>>& views_;
    std::size_t activeTabHash_{0};

    std::unordered_map<std::size_t, int> indicesMap_;
    std::unordered_map<ViewportController*, int> controllerMap_;
};

#endif  // OURPAINT_DOCUMENTCONTROLLER_H