#pragma once

#include "FontManager.h"
#include "GlyphAtlas.h"
#include "RenderCommand.h"

#include <cstdint>
#include <string>
#include <vector>

namespace vkapp::Graphics {

enum class TextAlign : uint32_t {
    Left = 0,
    Center,
    Right
};

struct GlyphInstance {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
};

struct TextLayout {
    std::vector<GlyphInstance> glyphs;
    float width = 0.0f;
    float height = 0.0f;
};

class TextLayoutEngine {
public:
    TextLayout layout(const std::string& text, uint32_t fontId, float fontSize,
                      float maxWidth, TextAlign align, FontManager* fontManager,
                      GlyphAtlas* atlas);
};

} // namespace vkapp::Graphics
