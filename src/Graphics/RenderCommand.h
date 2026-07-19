#pragma once

#include "Layout/Rect.h"

#include <cstdint>
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
};

using RenderCommandList = std::vector<RenderCommand>;

} // namespace vkapp::Graphics
