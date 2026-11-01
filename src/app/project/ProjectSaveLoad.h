#ifndef OURPAINT_APPLICATION_PROJECT_SAVE_LOAD_H_
#define OURPAINT_APPLICATION_PROJECT_SAVE_LOAD_H_

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "Project.h"
#include "saveload/DocumentSaveLoad.h"

namespace app {

inline constexpr uint32_t ProjectFormatVersion = 1;

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

}  // namespace app

#endif  // OURPAINT_APPLICATION_PROJECT_SAVE_LOAD_H_
