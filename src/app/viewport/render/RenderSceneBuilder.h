#ifndef OURPAINT_APPLICATION_RENDER_SCENE_BUILDER_H_
#define OURPAINT_APPLICATION_RENDER_SCENE_BUILDER_H_

namespace core {class Scene;}
class OverlayModel;
namespace render { class RenderScene; }
class AxisTexts;
namespace app { struct ViewportStyle; }

class RenderSceneBuilder {
public:
    RenderSceneBuilder(const core::Scene& scene,
                       const OverlayModel& overlay,
                       AxisTexts& axis,
                       const app::ViewportStyle& style,
                       render::RenderScene& renderScene);

    void rebuild();

private:
    void buildGrid();

    void buildBaseObjects();
    void buildBaseMarkers();

    void buildBezierCurves();
    void buildBezierHandles();

    void buildOverlayObjects();
    void buildOverlayMarkers();

    void buildSelectedObjects();
    void buildSelectedMarkers();

    void buildSelectionRectangle();
    void buildAxisText();

private:
    const core::Scene& scene_;
    const OverlayModel& overlay_;
    AxisTexts& axis_;
    const app::ViewportStyle& style_;
    render::RenderScene& renderScene_;
};

#endif // ! OURPAINT_APPLICATION_RENDER_SCENE_BUILDER_H_