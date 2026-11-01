#ifndef OURPAINT_HEADERS_DOCUMENT_SAVE_LOAD_H_
#define OURPAINT_HEADERS_DOCUMENT_SAVE_LOAD_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../Document.h"

namespace core::persistence {

inline constexpr uint32_t DocumentFormatVersion = 1;
inline constexpr size_t MaxDocumentBytes = 64U * 1024U * 1024U;

enum class CodecErrorCode { InvalidCbor, InvalidSchema, UnsupportedVersion, UnsupportedData, InvalidModel, LimitExceeded };

struct CodecError {
    CodecErrorCode code;
    // A document field, or "$" for an error concerning the whole input.
    std::string field;
    std::string message;
};

// Check the result before accessing value() or error(). Allocation failures and
// access to the inactive alternative are not recovered as codec errors.
template <class T>
class CodecResult {
public:
    static CodecResult success(T value) { return CodecResult(std::move(value)); }
    static CodecResult failure(CodecError error) { return CodecResult(std::move(error)); }
    explicit operator bool() const { return std::holds_alternative<T>(data_); }
    T& value() { return std::get<T>(data_); }
    const T& value() const { return std::get<T>(data_); }
    const CodecError& error() const { return std::get<CodecError>(data_); }

private:
    explicit CodecResult(T value) : data_(std::move(value)) {}
    explicit CodecResult(CodecError error) : data_(std::move(error)) {}
    std::variant<T, CodecError> data_;
};

using DocumentBytes = std::vector<uint8_t>;

// Synchronous in-memory codec: no file I/O, solve(), or source metadata changes.
// The caller serializes access to the source model. Returned bytes own their data.
// Currently supports the version-1 envelope and empty sketches only; nonempty
// entities/constraints return UnsupportedData rather than being discarded.
[[nodiscard]] CodecResult<DocumentBytes> encodeDocument(const Document& document);

// Borrows input only for this call and returns a newly owned document. No caller
// model is modified or installed in a manager. The path is not part of the format;
// dirty state follows Document's existing mutation semantics.
[[nodiscard]] CodecResult<std::unique_ptr<Document>> decodeDocument(std::span<const uint8_t> bytes);

}  // namespace core::persistence

#endif  // OURPAINT_HEADERS_DOCUMENT_SAVE_LOAD_H_
