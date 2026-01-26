#include "DocumentManager.h"
#include "Document.h"
#include "DocumentSaveLoad.h"

#include <stdexcept>

Document* DocumentManager::getActiveDocument() {
    if (activeDocumentIndex == -1) {
        return nullptr;
    }
    return documents[activeDocumentIndex];;
}

Document* DocumentManager::at(int index) {
    try {
        return documents[index];
    }
    catch (...) {
        throw std::out_of_range("out of range");
    }
}

bool DocumentManager::setActiveDocument(int index) {
    if (index < 0 || index >= count()) {
        return false;
    }
    activeDocumentIndex = index;
    return true;
}

bool DocumentManager::setActiveDocument(const std::string& name) {
    for (int i = 0; i < count(); i++) {
        if (documents[i]->name() == name) {
            activeDocumentIndex = i;
            return true;
        }
    }
    return false;
}

int DocumentManager::getActiveIndex() const {
    return activeDocumentIndex;
}

int DocumentManager::count() const {
    return documents.size();
}

int DocumentManager::createNewDocument(const std::string& documentName) {
    Document* document = new Document(documentName);
    documents.push_back(document);
    activeDocumentIndex = documents.size() - 1;
    return activeDocumentIndex;
}

int DocumentManager::openDocument(const std::string& path) {
    Document* document = nullptr;
    if (loadDocumentFromFile(path, document) == false) {
        return -1;
    }
    documents.push_back(document);
    return documents.size() - 1;
}

bool DocumentManager::closeDocument(const int index, const bool autoSave) {
    if (index < 0 || index >= count()) {
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

bool DocumentManager::isDocumentSaved(const int index) const {
    if (index < 0 || index >= count()) {
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


