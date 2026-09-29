#ifndef OURPAINT_HEADERS_DOCUMENT_MANAGER_H_
#define OURPAINT_HEADERS_DOCUMENT_MANAGER_H_

#include <string>
#include <vector>
#include <cstdint>
class Document;

using DocumentId = uint64_t;

class DocumentManager {
public:
    Document* at(DocumentId index) const;
    Document* at(const std::string& documentName) const;
    DocumentId findIdByName(const std::string& documentName) const;
    size_t count() const;

    DocumentId createNewDocument(const std::string& documentName = "Untitled");
    DocumentId openDocument(const std::string& path);

    bool closeDocument(DocumentId index, bool autoSave = true);
    bool closeDocument(const std::string& name, bool autoSave = true);

    bool isDocumentSaved(DocumentId index) const;
    bool isDocumentSaved(const std::string& name) const;

    bool isAllDocumentsSaved() const;
    bool closeAllDocuments(bool autoSave = true);

private:
    std::vector<Document*> documents;
};

#endif // ! OURPAINT_HEADERS_DOCUMENT_MANAGER_H_