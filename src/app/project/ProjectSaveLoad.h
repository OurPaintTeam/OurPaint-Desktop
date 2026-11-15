#ifndef OURPAINT_APPLICATION_PROJECT_SAVE_LOAD_H_
#define OURPAINT_APPLICATION_PROJECT_SAVE_LOAD_H_

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include "Project.h"
#include "saveload/DocumentSaveLoad.h"

namespace app {

inline constexpr uint32_t ProjectFormatVersion = 1;

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
    // Native errors retain their category; codec failures use codecError.
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

struct LoadedDocument {
    std::filesystem::path filename;
    std::unique_ptr<Document> document;
};

struct LoadedProject {
    std::filesystem::path root;
    ProjectMetadata metadata;
    std::vector<LoadedDocument> documents;

    // Create this description before moving documents into a DocumentManager.
    // Its references survive that move while the manager owns the documents.
    // Throws std::logic_error if a document slot has already been moved out.
    [[nodiscard]] Project view() const;
};

struct SaveReport {
    std::filesystem::path root;
    // Prospective metadata; a failed report is not a complete saved project.
    ProjectMetadata metadata;
    // Paths are root / filename. Files enter saved only after synchronization.
    std::vector<std::filesystem::path> saved;
    std::optional<FileError> error;
    // All intended files on a preparation failure; otherwise only files after
    // the failed file. The failed file itself is identified by error.path.
    std::vector<std::filesystem::path> unprocessed;
    bool failedFileReplaced = false;
    explicit operator bool() const { return !error; }
};

// No value means valid. Empty IDs are allowed only for unsaved descriptions
// when allowEmptyId is true. These checks perform no filesystem operations.
[[nodiscard]] std::optional<core::persistence::CodecError> validateProjectMetadata(const ProjectMetadata& metadata, bool allowEmptyId = false);
// Checks metadata and single .ourp filenames, with native platform comparison.
// An empty root is allowed for save-as; root existence is a file-I/O concern.
[[nodiscard]] std::optional<core::persistence::CodecError> validateProject(const Project& project);

// In-memory .ourpaint marker codec. Requires a published, nonempty ID; does not
// generate IDs, change metadata, access documents or perform file I/O.
[[nodiscard]] core::persistence::CodecResult<std::vector<uint8_t>> encodeProjectMetadata(const ProjectMetadata& metadata);
[[nodiscard]] core::persistence::CodecResult<ProjectMetadata> decodeProjectMetadata(std::span<const uint8_t> bytes);

// Synchronous save into an existing directory. Prepares all bytes and validates
// every destination before mutation, writes in native filename order, marker
// last. Stops on the first failure; this is not a transaction across files.
// An empty project ID is generated and adopted once the marker is replaced,
// even if its subsequent directory synchronization fails. Documents stay const.
// Callers serialize model/directory access. Allocation failures may propagate.
[[nodiscard]] SaveReport saveProject(Project& project);

// Saves a copy with a distinct ID into a new or empty directory, separate from
// the source root. For a new directory its parent must already exist. Prepares
// all bytes before creating the directory. Source metadata/documents stay const.
// A failure may leave a partial destination; no published files are removed.
// Adopt the report's root/metadata as a complete copy only on success.
[[nodiscard]] SaveReport saveProjectAs(const Project& project, const std::filesystem::path& destination);

// Reads .ourpaint and regular .ourp files directly in root, in native filename
// order. Directories/other extensions are ignored; .ourp symlinks are rejected.
// Every file is bounded by MaxDocumentBytes. Returns only fully staged models,
// without modifying existing documents, managers, files, paths or dirty flags.
[[nodiscard]] FileResult<LoadedProject> loadProject(const std::filesystem::path& root);

}  // namespace app

#endif  // OURPAINT_APPLICATION_PROJECT_SAVE_LOAD_H_
