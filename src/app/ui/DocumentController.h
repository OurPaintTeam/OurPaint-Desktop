#ifndef OURPAINT_DOCUMENTCONTROLLER_H
#define OURPAINT_DOCUMENTCONTROLLER_H

#include <filesystem>
#include <limits>
#include <string>
#include <unordered_map>

#include "../../core/DocumentManager.h"

class DocumentController {
public:
    static constexpr DocumentId InvalidId = std::numeric_limits<DocumentId>::max();

    explicit DocumentController(DocumentManager& manager);
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

private:
    DocumentManager& manager_;
    std::unordered_map<std::size_t, DocumentId> documentIds_;
};

#endif  // OURPAINT_DOCUMENTCONTROLLER_H