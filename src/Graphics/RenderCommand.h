#pragma once

#include "Layout/Rect.h"

#include <cstdint>
#include <string>
#include <vector>

namespace vkapp::Graphics {

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct RenderCommand {
    enum class Type : uint32_t {
        Rect,
        Text,
        Image
    };

    Type type = Type::Rect;

    vkapp::Layout::Rect rect{};
    Color color{1.0f, 1.0f, 1.0f, 1.0f};

    int32_t layer = 0;
    float opacity = 1.0f;
    int32_t zIndex = 0;

    vkapp::Layout::Rect scissor{};
    bool hasScissor = false;

    std::string textContent;
    std::string fontFamily;
    uint32_t fontId = UINT32_MAX;
    float fontSize = 16.0f;
    int fontWeight = 400;
    uint32_t textAlign = 0;
    uint32_t firstGlyph = 0;
    uint32_t glyphCount = 0;
};

using RenderCommandList = std::vector<RenderCommand>;

} // namespace vkapp::Graphics
