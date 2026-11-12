#ifndef OURPAINT_RENDERING_TEXT_MEASURER_H_
#define OURPAINT_RENDERING_TEXT_MEASURER_H_

#include <optional>
#include <string_view>

namespace render::text {

struct TextMetrics {
    double width = 0.0;
    double ascent = 0.0;
    double descent = 0.0;
};

// CPU-only query using the same font metrics as drawing. Sizes are in pixels;
// callers choose logical or framebuffer pixels through heightPx.
class TextMeasurer {
public:
    virtual ~TextMeasurer() = default;
    virtual std::optional<TextMetrics> measureText(std::string_view text, double heightPx) const = 0;
};

}  // namespace render::text

#endif  // OURPAINT_RENDERING_TEXT_MEASURER_H_
