#pragma once

#include <filesystem>
#include <optional>
#include <system_error>
#include <utility>
#include <variant>

#include "saveload/DocumentSaveLoad.h"

namespace app {

enum class FileStage {
    ValidateProject,
    ValidatePath,
    GenerateId,
    Encode,
    CreateDirectory,
    CreateTemporary,
    Open,
    Read,
    Enumerate,
    Decode,
    Write,
    Flush,
    Close,
    Replace,
    SyncDirectory
};

struct FileError {
    std::filesystem::path path;
    FileStage stage;
    std::error_code systemError;
    std::optional<core::persistence::CodecError> codecError;
};

// Check the active alternative before access. Allocation failures propagate.
template <class T>
class FileResult {
public:
    static FileResult success(T value) { return FileResult(std::move(value)); }
    static FileResult failure(FileError error) { return FileResult(std::move(error)); }
    explicit operator bool() const { return std::holds_alternative<T>(data_); }
    T& value() { return std::get<T>(data_); }
    const T& value() const { return std::get<T>(data_); }
    const FileError& error() const { return std::get<FileError>(data_); }

private:
    explicit FileResult(T value) : data_(std::move(value)) {}
    explicit FileResult(FileError error) : data_(std::move(error)) {}
    std::variant<T, FileError> data_;
};

}  // namespace app
