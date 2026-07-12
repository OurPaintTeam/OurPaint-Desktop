#include "DocumentManager.h"

#include <stdexcept>

#include "Document.h"
#include "saveload/DocumentSaveLoad.h"

Document* DocumentManager::at(DocumentId index) const {
    if (index < documents.size()) {
        return documents[index];
    }
    return nullptr;
}

Document* DocumentManager::at(const std::string& documentName) const {
    for (auto& document : documents) {
        if (document->name() == documentName) {
            return document;
        }
    }
    return nullptr;
}

DocumentId DocumentManager::findIdByName(const std::string& documentName) const {
    size_t i = 0;
    for (; i < documents.size(); ++i) {
        if (documents[i]->name() == documentName) {
            return i;
        }
    }
    return -1;
}

size_t DocumentManager::count() const {
    return documents.size();
}

DocumentId DocumentManager::createNewDocument(const std::string& documentName) {
    Document* document = new Document(documentName);
    documents.push_back(document);
    return documents.size() - 1;
}

DocumentId DocumentManager::openDocument(const std::string& path) {
    Document* document = nullptr;
    if (loadDocumentFromFile(path, document) == false) {
        return -1;
    }
    documents.push_back(document);
    return documents.size() - 1;
}

bool DocumentManager::closeDocument(const DocumentId index, const bool autoSave) {
    if (index >= count()) {
        return false;
    }
    Document* document = documents[index];
    if (autoSave) {
        if (saveDocumentToFile(document->path(), document) == false) {
            return false;
        }
    }
    delete document;
    return true;
}
bool DocumentManager::closeDocument(const std::string& name, bool autoSave) {
    Document* document = nullptr;
    for (int i = 0; i < count(); i++) {
        if (documents[i]->name() == name) {
            document = documents[i];
        }
    }
    if (document && autoSave) {
        if (saveDocumentToFile(document->path(), document) == false) {
            return false;
        }
    }
    delete document;
    return true;
}

bool DocumentManager::isDocumentSaved(const DocumentId index) const {
    if (index >= count()) {

        return false;
    }
    const Document* document = documents[index];
    return !document->isDirty();
}

bool DocumentManager::isDocumentSaved(const std::string& name) const {
    const Document* document = nullptr;
    for (int i = 0; i < count(); i++) {
        if (documents[i]->name() == name) {
            document = documents[i];
        }
    }
    return document && !document->isDirty();
}

bool DocumentManager::isAllDocumentsSaved() const {
    for (const Document* document : documents) {
        if (document->isDirty() == true) {
            return false;
        }
    }
    return true;
}

bool DocumentManager::closeAllDocuments(const bool autoSave) {
    for (const Document* document : documents) {
        if (autoSave) {
            if (saveDocumentToFile(document->path(), document) == false) {
                return false;
            }
        }
    }
    return true;
}


