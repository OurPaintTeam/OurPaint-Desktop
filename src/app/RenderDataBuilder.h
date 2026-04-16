#ifndef OURPAINT_APPLICATION_RENDER_DATA_BUILDER_H_
#define OURPAINT_APPLICATION_RENDER_DATA_BUILDER_H_

class Scene;
class OverlayModel;
namespace renderer { class RenderData; }

class RenderDataBuilder {
public:
    RenderDataBuilder(const Scene& scene,
                      const OverlayModel& overlay,
                      renderer::RenderData& renderData);

    void rebuild();

private:
    const Scene& scene_;
    const OverlayModel& overlay_;
    renderer::RenderData& renderData_;
};

#endif // ! OURPAINT_APPLICATION_RENDER_DATA_BUILDER_H_