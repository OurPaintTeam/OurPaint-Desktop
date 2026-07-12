#ifndef OURPAINT_APPLICATION_AXIS_TEXTS_H_
#define OURPAINT_APPLICATION_AXIS_TEXTS_H_

#include "glm/glm.hpp"
#include "Camera2D.h"
#include "text/Types.h"

class AxisTexts {
public:
    AxisTexts(const Camera2D& camera);

    struct Color {
        float r{};
        float g{};
        float b{};
        float a = 1.0f;
    };

    struct {
        float stepWorld = 100.0f;
        int minPixelSpacing = 80;
        int marginPixels = 40;
        int tickSize = 6;
        int precision = 6;
        Color textColor;
    } textConfig;

    struct GridInfo {
        float cellSize;
        float subCellSize;
        glm::vec3 axisColor{0.9f, 0.2f, 0.2f};
        glm::vec3 gridColor{0.5f, 0.5f, 0.5f};
    };

    void update();

    const std::vector<float>& getTickPositionsX() const { return tickX_; }
    const std::vector<float>& getTickPositionsY() const { return tickY_; }

    const std::vector<render::text::TextObject>& getLabels() const { return labels_; }
    const GridInfo& getGridInfo() const { return gridInfo_; }

private:
    float niceStep(float roughStep) const;

    static std::vector<float> computeMarks(float min, float max, float step);
    std::string formatValue(float value) const;

    GridInfo gridInfo_;

    std::vector<render::text::TextObject> labels_;
    std::vector<float> tickX_;
    std::vector<float> tickY_;

    const Camera2D& camera_;
};
#endif // ! OURPAINT_APPLICATION_AXIS_TEXTS_H_