#ifndef OURPAINT_CORE_CBOR_CODEC_H_
#define OURPAINT_CORE_CBOR_CODEC_H_

#include <optional>
#include <string_view>

#include "DocumentSaveLoad.h"

namespace core::persistence::detail {

// Internal shared checks for document and project codecs, not a public backend.
bool isValidUtf8(std::string_view text);
std::optional<CodecError> checkCbor(std::span<const uint8_t> bytes);

}  // namespace core::persistence::detail

#endif  // OURPAINT_CORE_CBOR_CODEC_H_
