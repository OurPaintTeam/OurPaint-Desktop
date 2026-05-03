#ifndef OURPAINT_RENDERING_TEXT_H_
#define OURPAINT_RENDERING_TEXT_H_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rendering::text {
// ------------------------------------------------------------
// Basic types
// ------------------------------------------------------------

struct Vec2d {
    double x = 0.0;
    double y = 0.0;
};

struct Vec3d {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct ColorRGBA8 {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;
};

struct RectD {
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
};

struct RectF {
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;
};

using FontId        = std::uint64_t;
using TextObjectId  = std::uint64_t;
using ViewportId    = std::uint64_t;
using GlyphId       = std::uint32_t;

enum class TextSpace {
    Screen,
    World
};

enum class TextHorizontalAlign {
    Left,
    Center,
    Right
};

enum class TextVerticalAlign {
    Top,
    Middle,
    Baseline,
    Bottom
};

enum class FontSlant {
    Normal,
    Italic,
    Oblique
};

enum class FontWeight : std::uint16_t {
    Thin       = 100,
    ExtraLight = 200,
    Light      = 300,
    Regular    = 400,
    Medium     = 500,
    SemiBold   = 600,
    Bold       = 700,
    ExtraBold  = 800,
    Black      = 900
};

enum class TextWrapMode {
    NoWrap,
    WordWrap,
    CharacterWrap
};

enum class TextBackendKind {
    Bitmap,
    MTSDF,
    Vector
};

// ------------------------------------------------------------
//  Model types
// ------------------------------------------------------------

struct FontDescriptor {
    std::string family;
    FontWeight  weight        = FontWeight::Regular;
    FontSlant   slant         = FontSlant::Normal;

    // If non-empty, renderer should resolve exact face from this file.
    std::string fontFilePath;

    bool        allowFallback = true;
};

struct TextStyle {
    FontDescriptor font;

    // For now:
    // - screen space: pixels
    // - world space : world units (renderer later converts as needed)
    double height        = 12.0;
    double pt            = 5.0;

    double letterSpacing = 0.0;
    double lineSpacing   = 1.0;

    ColorRGBA8 fillColor{255, 255, 255, 255};

    // TODO expe
    float r;
    float g;
    float b;
    float a;

    TextHorizontalAlign hAlign   = TextHorizontalAlign::Left;
    TextVerticalAlign   vAlign   = TextVerticalAlign::Baseline;
    TextWrapMode        wrapMode = TextWrapMode::NoWrap;

    std::optional<double> wrapWidth;

    std::optional<ColorRGBA8> backgroundColor;
    std::optional<ColorRGBA8> highlightColor;
    float outlineWidth = 0.f;
};

struct ScreenPlacement {
    Vec2d anchorPx;
    double rotationRad = 0.0;
    int    zOrder      = 0;
};

struct WorldPlacement {
    Vec3d  anchorWorld;

    // MVP simplification:
    // world text lies in XY plane with angle rotationRad.
    // Later replace with full transform/basis.
    double rotationRad = 0.0;

    bool   faceCamera         = false;
    bool   clampToMinPixelSize = false;
    double minPixelHeight     = 0.0;
};

struct TextPlacement {
    TextSpace      space  = TextSpace::Screen;
    ScreenPlacement screen{};
    WorldPlacement  world{};
};

struct TextObject {
    TextObjectId   id = 0;
    std::string    utf8Text;
    TextStyle      style{};
    TextPlacement  placement{};
    bool           visible      = true;
};

// Font resolution
struct ResolvedFontFace {
    FontId      fontId           = 0;
    std::string resolvedFilePath;
    std::string resolvedFaceName;
    long        faceIndex        = 0;
};

// Shaping output
struct GlyphPlacement {
    GlyphId      glyphId = 0;
    Vec2d        offset{};
    Vec2d        advance{};
    std::uint32_t cluster = 0;  // Maps back to source UTF-8.
};

struct GlyphRun {
    ResolvedFontFace             fontFace{};
    std::vector<GlyphPlacement>  glyphs;
    double ascender  = 0.0;
    double descender = 0.0;
    double width     = 0.0;
    double height    = 0.0;
};


// Glyph data from font source
struct GlyphMetrics {
    double advanceX = 0.0;
    double advanceY = 0.0;
    double bearingX = 0.0;
    double bearingY = 0.0;
    RectD  bounds{};
};

struct BitmapGlyphData {
    std::uint32_t             width  = 0;
    std::uint32_t             height = 0;
    std::vector<std::uint8_t> pixels;  // Single-channel, row-major.
    int bitmapLeft = 0;
    int bitmapTop  = 0;
};

struct AtlasLocation {
    std::uint32_t pageIndex = 0;
    RectF         uvRect{};
};

struct BitmapGlyphHandle {
    AtlasLocation atlas{};
    GlyphMetrics  metrics{};
};

// Frame context
struct TextFrameContext {
    ViewportId viewportId = 0;
    double dpi = 96.0;
    double zoom = 1.0;
    bool isInteracting = false;
    int framebufferWidth = 1;
    int framebufferHeight = 1;
};

// ------------------------------------------------------------
// Interfaces
// ------------------------------------------------------------

class IFontManager {
public:
    virtual ~IFontManager() = default;

    // Resolve requested font/style to a concrete face.
    virtual ResolvedFontFace resolveFace(const FontDescriptor& descriptor) = 0;

    // Reload font registry if application font sources changed.
    virtual void reloadFontRegistry() = 0;
};

class ITextShaper {
public:
    virtual ~ITextShaper() = default;

    // Convert source text into glyph ids and placements.
    // MVP implementation may do naive codepoint -> glyph mapping.
    virtual GlyphRun shape(const TextObject& object,
                           const ResolvedFontFace& face,
                           double dpi) = 0;
};

class IGlyphSource {
public:
    virtual ~IGlyphSource() = default;

    // Load glyph metrics from font source.
    virtual GlyphMetrics   loadMetrics(const ResolvedFontFace& face,
                                       GlyphId glyphId,
                                       double nominalHeight) = 0;

    // Rasterize glyph into single-channel bitmap.
    virtual BitmapGlyphData loadBitmap(const ResolvedFontFace& face,
                                       GlyphId glyphId,
                                       double nominalHeight,
                                       double dpi) = 0;
};
}

#endif // OURPAINT_RENDERING_TEXT_H_