#ifndef OURPAINT_HEADERS_DOCUMENT_SAVE_LOAD_H_
#define OURPAINT_HEADERS_DOCUMENT_SAVE_LOAD_H_

#include <string>

#include "../Document.h"

bool loadDocumentFromFile(const std::string& filePath, const Document* document);

bool saveDocumentToFile(const std::string& filePath, const Document* document);

#endif // ! OURPAINT_HEADERS_DOCUMENT_SAVE_LOAD_H_