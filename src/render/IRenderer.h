#ifndef OURPAINT_RENDERING_IRENDERER_H_
#define OURPAINT_RENDERING_IRENDERER_H_

namespace render { class RenderScene; }
class Camera2D;

class IRenderer {
public:
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void resize(int w, int h) = 0;
    virtual void render(const render::RenderScene& scene, const Camera2D& camera) = 0;
    virtual ~IRenderer() = default;
};

#endif // ! OURPAINT_RENDERING_IRENDERER_H_