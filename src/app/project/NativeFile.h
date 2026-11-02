#ifndef OURPAINT_APPLICATION_NATIVE_FILE_H_
#define OURPAINT_APPLICATION_NATIVE_FILE_H_

#include "ProjectSaveLoad.h"

namespace app::detail {

// The same ordering is used for filename uniqueness and publication order.
struct FilenameLess {
    bool operator()(const std::filesystem::path& first, const std::filesystem::path& second) const;
};

// Internal synchronous file operations, not a public persistence backend.
struct FileReplacementResult {
    std::optional<FileError> error;
    // True after publication, including a subsequent directory-sync failure.
    bool replaced = false;
    explicit operator bool() const { return !error; }
};

// Performs no mutations. The parent must exist; the target may be absent or a
// regular file, never a symlink. Callers serialize access to the directory.
[[nodiscard]] std::optional<FileError> validateFileDestination(const std::filesystem::path& target);

// Creates an exclusive sibling, writes/flushes/closes it, then replaces target.
// Linux also synchronizes the parent directory. Unpublished temporary files are
// removed on a best-effort basis. The old target is never explicitly deleted.
[[nodiscard]] FileReplacementResult replaceFile(const std::filesystem::path& target, std::span<const uint8_t> bytes);

}  // namespace app::detail

#endif  // OURPAINT_APPLICATION_NATIVE_FILE_H_
