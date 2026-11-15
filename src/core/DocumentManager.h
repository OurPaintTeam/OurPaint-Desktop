#ifndef OURPAINT_HEADERS_DOCUMENT_MANAGER_H_
#define OURPAINT_HEADERS_DOCUMENT_MANAGER_H_

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

class Document;

using DocumentId = uint64_t;
inline constexpr DocumentId InvalidDocumentId = std::numeric_limits<DocumentId>::max();

// Owns documents, without file I/O. Views must be destroyed before their documents.
// IDs are stable slots: closing leaves a hole and never recycles an ID.
class DocumentManager {
public:
    DocumentManager();
    ~DocumentManager();
    DocumentManager(const DocumentManager&) = delete;
    DocumentManager& operator=(const DocumentManager&) = delete;

    Document* at(DocumentId index) const;
    Document* at(const std::string& documentName) const;
    // Returns InvalidDocumentId when no live document has the requested name.
    DocumentId findIdByName(const std::string& documentName) const;
    // Counts live documents, not the slot range.
    size_t count() const;

    DocumentId createNewDocument(const std::string& documentName = "Untitled");
    // Adopts a staged document without changing its address or contents.
    // A null document throws std::invalid_argument.
    DocumentId openDocument(std::unique_ptr<Document> document);

    bool closeDocument(DocumentId index);
    bool closeDocument(const std::string& name);

    bool isDocumentSaved(DocumentId index) const;
    bool isDocumentSaved(const std::string& name) const;

    bool isAllDocumentsSaved() const;
    bool closeAllDocuments();

private:
    std::vector<std::unique_ptr<Document>> documents_;
};

#endif // ! OURPAINT_HEADERS_DOCUMENT_MANAGER_H_
