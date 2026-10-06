#include "AxisTexts.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

AxisTexts::AxisTexts(const Camera2D& camera) : gridInfo_(), camera_(camera) {}

void AxisTexts::update() {
    labels_.clear();
    tickX_.clear();
    tickY_.clear();

    const glm::dvec2 worldMin = camera_.visibleMinWorld();
    const glm::dvec2 worldMax = camera_.visibleMaxWorld();

    const double roughStepX = textConfig.minPixelSpacing / camera_.zoom();

    const double step = niceStep(roughStepX);

    // Generate marks
    const std::vector<double> marksX = computeMarks(worldMin.x, worldMax.x, step);
    const std::vector<double> marksY = computeMarks(worldMin.y, worldMax.y, step);

    // Horizontal axis

    textConfig.textColor = {0.2, 0.2, 0.2, 1.0};


    double pixelsPadding = 2.0;
    double worldPadding = camera_.screenLogicalToWorld(pixelsPadding);

    using namespace render::text;

    TextHorizontalAlign hAlign = TextHorizontalAlign::Center;
    TextVerticalAlign vAlign = TextVerticalAlign::Top;

    double screenY = -worldPadding;
    if (worldMin.y >= 0.0 - camera_.screenLogicalToWorld(15.5)) {
        vAlign = TextVerticalAlign::Bottom;
        screenY = worldMin.y + worldPadding;
        textConfig.textColor = {0.5, 0.5, 0.5, 1.0};
    }
    else if (worldMax.y <= 0.0) {
        screenY = worldMax.y - worldPadding;
        textConfig.textColor = {0.5, 0.5, 0.5, 1.0};
    }

    const double axisY = camera_.worldToScreenFramebuffer(glm::dvec2(0.0, screenY)).y;
    for (double wx : marksX) {
        if (std::abs(wx) < 1e-6f) {
            continue;
        }
        const glm::dvec2 screen = camera_.worldToScreenFramebuffer(glm::dvec2(wx, 0.0));
        tickX_.push_back(screen.x);

        TextObject label;
        label.utf8Text = formatValue(wx, step);
        label.placement.screen.anchorPx.x = screen.x;
        label.placement.screen.anchorPx.y = camera_.hFramebuffer() - axisY;
        label.style.r = textConfig.textColor.r;
        label.style.g = textConfig.textColor.g;
        label.style.b = textConfig.textColor.b;
        label.style.a = textConfig.textColor.a;
        label.style.hAlign = hAlign;
        label.style.vAlign = vAlign;
        label.style.pt = 1.0;
        //label.scale = 1.0f;
        labels_.push_back(label);
    }

    // Vertical axis

    hAlign = TextHorizontalAlign::Right;
    vAlign = TextVerticalAlign::Middle;

    textConfig.textColor = {0.2, 0.2, 0.2, 1.0};
    double screenX = -worldPadding;
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
    const double axisX = camera_.worldToScreenFramebuffer(glm::dvec2(screenX, 0.0)).x;
    for (double wy : marksY) {
        if (std::abs(wy) < 1e-6f) {
            continue;
        }

        const glm::dvec2 screen = camera_.worldToScreenFramebuffer(glm::dvec2(0.0, wy));
        tickY_.push_back(screen.y);

        TextObject label;
        label.utf8Text = formatValue(wy, step);
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
    const glm::dvec2 screen = camera_.worldToScreenFramebuffer(glm::dvec2(0.0, 0.0));
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

double AxisTexts::niceStep(double roughStep) const {
    if (roughStep <= 0.0) {
        return 1.0;
    }

    const double exponent = std::floor(std::log10(roughStep));

    const double fraction = roughStep / std::pow(10.0, exponent);

    double niceFraction;
    if (fraction <= 1.0) {
        niceFraction = 1.0;
    } else if (fraction <= 2.0) {
        niceFraction = 2.0;
    } else if (fraction <= 5.0) {
        niceFraction = 5.0;
    } else {
        niceFraction = 10.0;
    }

    // std::cout << "roughStep: " << roughStep << '\n'
    //           << "exponent: " << exponent << '\n'
    //           << "fraction: " << fraction << '\n'
    //           << "niceFraction: " << niceFraction << '\n'
    //           << "niceFraction * 10^exponent: " << niceFraction * std::pow(10.0f, exponent) << '\n';
    // std::cout << '\n';

    return niceFraction * std::pow(10.0, exponent);
}

std::vector<double> AxisTexts::computeMarks(const double min, const double max, const double step) {
    std::vector<double> marks;
    const double firstIndex = std::ceil(min / step);
    const double lastIndex = std::floor(max / step);
    const int count = static_cast<int>(lastIndex - firstIndex) + 1;

    marks.reserve(count);
    for (int i = 0 - 1; i < count + 1; ++i) {
        marks.push_back((firstIndex + i) * step);
    }
    return marks;
}

std::string AxisTexts::formatValue(double value, double step) const {
    const int stepPrecision = static_cast<int>(std::ceil(-std::log10(step)));
    const int precision = std::clamp(stepPrecision, 0, std::max(0, textConfig.precision));
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::fixed << std::setprecision(precision) << value;
    std::string s = ss.str();
    if (s.find('.') != std::string::npos) {
        s.erase(s.find_last_not_of('0') + 1, std::string::npos);
        if (s.back() == '.') {
            s.pop_back();
        }
    }
    return s == "-0" ? "0" : s;
}
