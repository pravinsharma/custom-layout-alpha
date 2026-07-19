#include "RenderCommandBuilder.h"
#include "Layout/LayoutNode.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace vkapp::Graphics {

namespace {

uint32_t nameHash(const std::string& name) {
    uint32_t h = 2166136261u;
    for (char c : name) {
        h ^= static_cast<uint8_t>(c);
        h *= 16777619u;
    }
    return h;
}

float srgbToLinear(float srgb) {
    if (srgb <= 0.04045f) {
        return srgb / 12.92f;
    }
    return std::pow((srgb + 0.055f) / 1.055f, 2.4f);
}

Color linearize(const Color& c) {
    return {
        srgbToLinear(c.r),
        srgbToLinear(c.g),
        srgbToLinear(c.b),
        c.a
    };
}

Color colorFromDepthAndName(int depth, const std::string& name) {
    if (name == "root") {
        return linearize({0.12f, 0.14f, 0.18f, 1.0f});
    }

    uint32_t h = nameHash(name);
    float hue = static_cast<float>(h % 360) / 360.0f;
    float sat = 0.55f + static_cast<float>((h >> 8) % 40) / 100.0f;
    float val = 0.75f + static_cast<float>((h >> 16) % 30) / 100.0f;

    float c = val * sat;
    float x = c * (1.0f - std::fabs(static_cast<float>(static_cast<int>(hue * 6.0f) % 2) - 1.0f));
    float m = val - c;

    float r = 0.0f, g = 0.0f, b = 0.0f;
    int sector = static_cast<int>(hue * 6.0f) % 6;
    if (sector == 0) { r = c; g = x; b = 0.0f; }
    else if (sector == 1) { r = x; g = c; b = 0.0f; }
    else if (sector == 2) { r = 0.0f; g = c; b = x; }
    else if (sector == 3) { r = 0.0f; g = x; b = c; }
    else if (sector == 4) { r = x; g = 0.0f; b = c; }
    else { r = c; g = 0.0f; b = x; }

    return linearize({r + m, g + m, b + m, 1.0f});
}

}

RenderCommandList buildRenderTree(const vkapp::Layout::LayoutNode& node)
{
    RenderCommandList commands;
    commands.reserve(16);

    if (node.flex.display == Layout::Display::None) {
        return commands;
    }

    RenderCommand cmd;
    cmd.type = RenderCommand::Type::Rect;
    cmd.rect = node.computedRect;
    cmd.color = linearize(node.color.value_or(colorFromDepthAndName(0, node.name)));
    cmd.layer = static_cast<int32_t>(node.order);
    commands.push_back(cmd);

    for (const auto* child : node.children) {
        auto childCommands = buildRenderTree(*child);
        commands.insert(commands.end(), childCommands.begin(), childCommands.end());
    }

    return commands;
}

} // namespace vkapp::Graphics
