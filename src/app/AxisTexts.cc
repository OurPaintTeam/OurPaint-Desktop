#include "AxisTexts.h"

#include <sstream>
#include <iomanip>

AxisTexts::AxisTexts(const Camera2D& camera) : gridInfo_(), camera_(camera) {}

void AxisTexts::update() {
    labels_.clear();
    tickX_.clear();
    tickY_.clear();

    const glm::vec2 worldMin = camera_.visibleMinWorld();
    const glm::vec2 worldMax = camera_.visibleMaxWorld();

    const float roughStepX = textConfig.minPixelSpacing / camera_.zoom();

    const float step = niceStep(roughStepX);

    // Generate marks
    std::vector<float> marksX = computeMarks(worldMin.x, worldMax.x, step);
    std::vector<float> marksY = computeMarks(worldMin.y, worldMax.y, step);

    // Horizontal axis

    textConfig.textColor = {0.2, 0.2, 0.2, 1.0};


    double pixelsPadding = 2.0;
    double worldPadding = camera_.screenLogicalToWorld(pixelsPadding);

    using namespace rendering::text;

    TextHorizontalAlign hAlign = TextHorizontalAlign::Center;
    TextVerticalAlign vAlign = TextVerticalAlign::Top;

    float screenY = 0.0f - worldPadding;
    if (worldMin.y >= 0.0 - camera_.screenLogicalToWorld(15.5)) {
        vAlign = TextVerticalAlign::Bottom;
        screenY = worldMin.y + worldPadding;
        textConfig.textColor = {0.5, 0.5, 0.5, 1.0};
    }
    else if (worldMax.y <= 0.0) {
        screenY = worldMax.y - worldPadding;
        textConfig.textColor = {0.5, 0.5, 0.5, 1.0};
    }

    const float axisY = camera_.worldToScreenFramebuffer(glm::vec2(0.0f, screenY)).y;
    for (float wx : marksX) {
        if (std::abs(wx) < 1e-6f) continue;
        const glm::vec2 screen = camera_.worldToScreenFramebuffer(glm::vec2(wx, 0.0f));
        tickX_.push_back(screen.x);

        TextObject label;
        label.utf8Text = formatValue(wx);
        label.placement.screen.anchorPx.x = screen.x;
        label.placement.screen.anchorPx.y = camera_.hFramebuffer() -axisY;
        label.style.r = textConfig.textColor.r;
        label.style.g = textConfig.textColor.g;
        label.style.b = textConfig.textColor.b;
        label.style.a = textConfig.textColor.a;
        label.style.hAlign = hAlign;
        label.style.vAlign = vAlign;
        label.style.pt = 5.0;
        //label.scale = 1.0f;
        labels_.push_back(label);
    }



    // Vertical axis

    hAlign = TextHorizontalAlign::Right;
    vAlign = TextVerticalAlign::Middle;

    textConfig.textColor = {0.2, 0.2, 0.2, 1.0};
    float screenX = 0.0f - worldPadding;
    if (worldMin.x >= 0.0 - camera_.screenLogicalToWorld(15.5)) {
        hAlign = TextHorizontalAlign::Left;
        screenX = worldMin.x + worldPadding;
        textConfig.textColor = {0.5, 0.5, 0.5, 1.0};
    }
    else if (worldMax.x <= 0.0) {
        screenX = worldMax.x - worldPadding;
        textConfig.textColor = {0.5, 0.5, 0.5, 1.0};
    }

    // Vertical axis
    const float axisX = camera_.worldToScreenFramebuffer(glm::vec2(screenX, 0.0f)).x;
    for (float wy : marksY) {
        if (std::abs(wy) < 1e-6f) continue;

        const glm::vec2 screen = camera_.worldToScreenFramebuffer(glm::vec2(0.0f, wy));
        tickY_.push_back(screen.y);

        TextObject label;
        label.utf8Text = formatValue(wy);
        label.placement.screen.anchorPx.x = axisX;  // левее оси
        label.placement.screen.anchorPx.y = camera_.hFramebuffer() - screen.y;
        label.style.r = textConfig.textColor.r;
        label.style.g = textConfig.textColor.g;
        label.style.b = textConfig.textColor.b;
        label.style.a = textConfig.textColor.a;
        label.style.hAlign = hAlign;
        label.style.vAlign = vAlign;
        //label.scale = 1.0f;
        labels_.push_back(label);
    }

    hAlign = TextHorizontalAlign::Right;
    vAlign = TextVerticalAlign::Top;

    // Zero
    const glm::vec2 screen = camera_.worldToScreenFramebuffer(glm::vec2(0.0f, 0));
    TextObject label;
    label.utf8Text = "0";
    label.placement.screen.anchorPx.x = screen.x - pixelsPadding;
    label.placement.screen.anchorPx.y = camera_.hFramebuffer() - screen.y - pixelsPadding;
    label.style.r = textConfig.textColor.r;
    label.style.g = textConfig.textColor.g;
    label.style.b = textConfig.textColor.b;
    label.style.a = textConfig.textColor.a;
    label.style.hAlign = hAlign;
    label.style.vAlign = vAlign;
    //label.scale = 1.0f;
    labels_.push_back(label);

    gridInfo_.cellSize = step;
    gridInfo_.subCellSize = gridInfo_.cellSize / 5.0f;
}

float AxisTexts::niceStep(float roughStep) const {
    if (roughStep <= 0.0f) {
        return 1.0f;
    }

    const float exponent = std::floor(std::log10(roughStep));

    const float fraction = roughStep / std::pow(10.0f, exponent);

    float niceFraction;
    if (fraction <= 1.0f)
        niceFraction = 1.0f;
    else if (fraction <= 2.0f)
        niceFraction = 2.0f;
    else if (fraction <= 5.0f)
        niceFraction = 5.0f;
    else
        niceFraction = 10.0f;

    // std::cout << "roughStep: " << roughStep << '\n'
    //           << "exponent: " << exponent << '\n'
    //           << "fraction: " << fraction << '\n'
    //           << "niceFraction: " << niceFraction << '\n'
    //           << "niceFraction * 10^exponent: " << niceFraction * std::pow(10.0f, exponent) << '\n';
    // std::cout << '\n';

    return niceFraction * std::pow(10.0f, exponent);
}


std::vector<float> AxisTexts::computeMarks(float min, float max, float step) const {
    std::vector<float> marks;
    const float first = std::ceil(min / step) * step;
    float val = first;
    marks.push_back(val - step);
    while (val <= max + 0.0001f) {
        marks.push_back(val);
        val += step;
    }
    marks.push_back(val);
    return marks;
}

std::string AxisTexts::formatValue(float value) const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(textConfig.precision) << value;
    std::string s = ss.str();
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (s.back() == '.') {
        s.pop_back();
    }
    return s;
}



