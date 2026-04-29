#ifndef OURPAINT_HEADERS_DOCUMENT_MANAGER_H_
#define OURPAINT_HEADERS_DOCUMENT_MANAGER_H_

#include <string>
#include <vector>
class Document;

class DocumentManager {
public:
    ~DocumentManager();

    Document* getActiveDocument();
    Document* at(int index);
    bool setActiveDocument(int index);
    bool setActiveDocument(const std::string& name);
    int getActiveIndex() const;
    int count() const;

    int createNewDocument(const std::string& documentName = "Untitled");
    int openDocument(const std::string& path);

    bool closeDocument(int index, bool autoSave = true);
    bool closeDocument(const std::string& name, bool autoSave = true);

    bool isDocumentSaved(int index) const;
    bool isDocumentSaved(const std::string& name) const;

    bool isAllDocumentsSaved() const;
    bool closeAllDocuments(bool autoSave = true);

private:
    int activeDocumentIndex = -1;
    std::vector<Document*> documents;
};

#endif // ! OURPAINT_HEADERS_DOCUMENT_MANAGER_H_
