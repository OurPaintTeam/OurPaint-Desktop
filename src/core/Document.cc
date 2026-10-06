#include "Document.h"
#include "Sketch.h"

Document::Document(const std::string& documentName) {
    documentName_ = documentName;
    sketch_ = std::move(core::sketch::Sketch::create(core::sketch::BackendKind::SolveSpace).value());
}
Document::~Document() {}

std::string& Document::name() {
    isDirty_ = true;
    return documentName_;
}
const std::string& Document::name() const {
    return documentName_;
}

std::string& Document::path() {
    isDirty_ = true;
    return filePath_;
}
const std::string& Document::path() const {
    return filePath_;
}

core::sketch::Sketch& Document::sketch() {
    isDirty_ = true;
    return *sketch_;
}

const core::sketch::Sketch& Document::sketch() const {
    return *sketch_;
}

bool Document::isDirty() const {
    return isDirty_;
}
