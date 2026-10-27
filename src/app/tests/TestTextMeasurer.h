#ifndef OURPAINT_TEST_TEXT_MEASURER_H_
#define OURPAINT_TEST_TEXT_MEASURER_H_

#include "text/TextMeasurer.h"

// Deterministic metrics keep layout tests independent of fonts and a graphics context.
class TestTextMeasurer : public render::text::TextMeasurer {
public:
    bool available = true;

    std::optional<render::text::TextMetrics> measureText(std::string_view text, double heightPx) const override {
        if (!available) {
            return std::nullopt;
        }
        return render::text::TextMetrics{text.size() * heightPx * 0.6, heightPx * 0.8, heightPx * 0.2};
    }
};

#endif  // OURPAINT_TEST_TEXT_MEASURER_H_
