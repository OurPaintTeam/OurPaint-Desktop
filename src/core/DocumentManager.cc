#include "DocumentManager.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "Document.h"

DocumentManager::DocumentManager() = default;
DocumentManager::~DocumentManager() = default;

Document* DocumentManager::at(DocumentId index) const {
    if (index < documents_.size()) {
        return documents_[index].get();
    }
    return nullptr;
}

Document* DocumentManager::at(const std::string& documentName) const {
    return at(findIdByName(documentName));
}

DocumentId DocumentManager::findIdByName(const std::string& documentName) const {
    for (size_t i = 0; i < documents_.size(); ++i) {
        const auto& document = documents_[i];
        if (document && std::as_const(*document).name() == documentName) {
            return i;
        }
    }
    return InvalidDocumentId;
}

size_t DocumentManager::count() const {
    return std::count_if(documents_.begin(), documents_.end(), [](const auto& document) { return document != nullptr; });
}

DocumentId DocumentManager::createNewDocument(const std::string& documentName) {
    return openDocument(std::make_unique<Document>(documentName));
}

DocumentId DocumentManager::openDocument(std::unique_ptr<Document> document) {
    if (!document) {
        throw std::invalid_argument("Cannot adopt a null document");
    }
    documents_.push_back(std::move(document));
    return documents_.size() - 1;
}

bool DocumentManager::closeDocument(DocumentId index) {
    if (!at(index)) {
        return false;
    }
    documents_[index].reset();
    return true;
}

bool DocumentManager::closeDocument(const std::string& name) {
    return closeDocument(findIdByName(name));
}

bool DocumentManager::isDocumentSaved(DocumentId index) const {
    const Document* document = at(index);
    return document && !document->isDirty();
}

bool DocumentManager::isDocumentSaved(const std::string& name) const {
    return isDocumentSaved(findIdByName(name));
}

bool DocumentManager::isAllDocumentsSaved() const {
    return std::all_of(documents_.begin(), documents_.end(), [](const auto& document) { return !document || !document->isDirty(); });
}

bool DocumentManager::closeAllDocuments() {
    for (auto& document : documents_) {
        document.reset();
    }
    return true;
}
