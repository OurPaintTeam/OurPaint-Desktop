#ifndef OURPAINT_HEADERS_DOCUMENT_H_
#define OURPAINT_HEADERS_DOCUMENT_H_

#include <string>
#include "sketch/Sketch.h"

class Document {
public:
    explicit Document(const std::string& documentName = "Untitled");
    ~Document();

    std::string& name();
    const std::string& name() const;

    std::string& path();
    const std::string& path() const;

    core::sketch::Sketch& sketch();
    const core::sketch::Sketch& sketch() const;

    bool isDirty() const;

private:
    std::string filePath_{};
    std::string documentName_{};
    bool isDirty_ = false;

    std::unique_ptr<core::sketch::Sketch> sketch_;
};


#endif // ! OURPAINT_HEADERS_DOCUMENT_H_