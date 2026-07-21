#include "RenderCommandBuilder.h"
#include "Layout/LayoutNode.h"
#include "Graphics/TextLayout.h"

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

Color colorForPlaceholder(int depth, bool isCard) {
    if (isCard) {
        return linearize({1.0f, 1.0f, 0.85f, 1.0f});
    }

    float v = 0.95f - depth * 0.08f;
    v = std::clamp(v, 0.25f, 0.95f);
    return linearize({v, v, v, 1.0f});
}

float approximateTextWidth(const std::string& name) {
    size_t chars = name.size();
    float charWidth = 9.0f;
    float width = static_cast<float>(chars) * charWidth + 10.0f;
    return std::max(width, 40.0f);
}

TextAlign resolveTextAlign(const std::string& align) {
    if (align == "center") return TextAlign::Center;
    if (align == "right") return TextAlign::Right;
    return TextAlign::Left;
}

}

NodeRenderInfo getNodeRenderInfo(const vkapp::Layout::LayoutNode& node, int depth, bool placeholderMode)
{
    NodeRenderInfo info{};
    info.isCard = node.name.find("card") != std::string::npos;
    info.isLeaf = node.children.empty();
    info.layer = static_cast<int32_t>(node.order);
    info.opacity = node.opacity;
    info.zIndex = node.positioning.zIndex;

    const auto& rect = node.computedRect;
    const float bt = node.box.borderTop;
    const float br = node.box.borderRight;
    const float bb = node.box.borderBottom;
    const float bl = node.box.borderLeft;

    if (placeholderMode) {
        if (info.isLeaf) {
            float textWidth = approximateTextWidth(node.name);
            float textHeight = rect.height;
            float textX = rect.x + bl;
            float textY = rect.y + bt;

            float innerWidth = std::max(0.0f, rect.width - bl - br);
            float innerHeight = std::max(0.0f, rect.height - bt - bb);

            if (textWidth > innerWidth) {
                textWidth = innerWidth;
            }
            if (textHeight > innerHeight) {
                textHeight = innerHeight;
            }

            info.renderX = textX;
            info.renderY = textY;
            info.renderWidth = textWidth;
            info.renderHeight = textHeight;
            info.hasTextPlaceholder = true;
            info.textX = textX;
            info.textY = textY;
            info.textWidth = textWidth;
            info.textHeight = textHeight;

            Color textColor = linearize({0.0f, 0.0f, 0.0f, 1.0f});
            info.colorR = textColor.r;
            info.colorG = textColor.g;
            info.colorB = textColor.b;
            info.colorA = textColor.a;
        } else {
            info.renderX = rect.x;
            info.renderY = rect.y;
            info.renderWidth = rect.width;
            info.renderHeight = rect.height;

            Color bg = colorForPlaceholder(depth, info.isCard);
            info.colorR = bg.r;
            info.colorG = bg.g;
            info.colorB = bg.b;
            info.colorA = bg.a;
        }
    } else {
        info.renderX = rect.x;
        info.renderY = rect.y;
        info.renderWidth = rect.width;
        info.renderHeight = rect.height;

        Color c = linearize(node.color.value_or(colorFromDepthAndName(depth, node.name)));
        info.colorR = c.r;
        info.colorG = c.g;
        info.colorB = c.b;
        info.colorA = c.a;
    }

    return info;
}

RenderCommandList buildRenderTree(const vkapp::Layout::LayoutNode& node, int depth, bool placeholderMode, const vkapp::Layout::Rect* parentScissor)
{
    RenderCommandList commands;
    commands.reserve(16);

    if (node.flex.display == Layout::Display::None) {
        return commands;
    }

    const vkapp::Layout::Rect& rect = node.computedRect;
    const float bt = node.box.borderTop;
    const float br = node.box.borderRight;
    const float bb = node.box.borderBottom;
    const float bl = node.box.borderLeft;
    const bool isCard = node.name.find("card") != std::string::npos;
    const bool isLeaf = node.children.empty();

    vkapp::Layout::Rect currentScissor = parentScissor ? *parentScissor : vkapp::Layout::Rect{};
    if (node.overflow == Layout::Overflow::Hidden) {
        currentScissor = rect;
    }

    auto applyScissor = [&](RenderCommand& cmd) {
        if (currentScissor.width > 0.0f && currentScissor.height > 0.0f) {
            cmd.scissor = currentScissor;
            cmd.hasScissor = true;
        }
    };

    if (node.borderColor.has_value() && (bt > 0.0f || br > 0.0f || bb > 0.0f || bl > 0.0f)) {
        RenderCommand border;
        border.type = RenderCommand::Type::Rect;
        border.rect = rect;
        border.color = linearize(*node.borderColor);
        border.layer = static_cast<int32_t>(node.order);
        border.opacity = node.opacity;
        border.zIndex = node.positioning.zIndex;
        applyScissor(border);
        commands.push_back(border);
    }

    if (node.backgroundColor.has_value()) {
        RenderCommand bg;
        bg.type = RenderCommand::Type::Rect;
        bg.rect = {
            rect.x + bl,
            rect.y + bt,
            std::max(0.0f, rect.width - bl - br),
            std::max(0.0f, rect.height - bt - bb)
        };
        bg.color = linearize(*node.backgroundColor);
        bg.layer = static_cast<int32_t>(node.order);
        bg.opacity = node.opacity;
        bg.zIndex = node.positioning.zIndex;
        applyScissor(bg);
        commands.push_back(bg);
    }

    RenderCommand cmd;
    cmd.type = RenderCommand::Type::Rect;

    if (placeholderMode) {
        if (isLeaf && !node.textContent.empty()) {
            float textX = rect.x + bl;
            float textY = rect.y + bt;
            float innerWidth = std::max(0.0f, rect.width - bl - br);
            float innerHeight = std::max(0.0f, rect.height - bt - bb);

            RenderCommand textCmd;
            textCmd.type = RenderCommand::Type::Text;
            textCmd.rect = {textX, textY, innerWidth, innerHeight};
            textCmd.color = linearize({0.0f, 0.0f, 0.0f, 1.0f});
            textCmd.textContent = node.textContent;
            textCmd.fontId = 0;
            textCmd.fontSize = node.flex.fontSize > 0.0f ? node.flex.fontSize : 14.0f;
            textCmd.fontWeight = node.flex.fontWeight;
            textCmd.layer = static_cast<int32_t>(node.order);
            textCmd.opacity = node.opacity;
            textCmd.zIndex = node.positioning.zIndex;
            applyScissor(textCmd);
            commands.push_back(textCmd);
        } else if (isLeaf) {
            float textWidth = approximateTextWidth(node.name);
            float textHeight = rect.height;
            float textX = rect.x + bl;
            float textY = rect.y + bt;

            float innerWidth = std::max(0.0f, rect.width - bl - br);
            float innerHeight = std::max(0.0f, rect.height - bt - bb);

            if (textWidth > innerWidth) {
                textWidth = innerWidth;
            }
            if (textHeight > innerHeight) {
                textHeight = innerHeight;
            }

            cmd.rect = {textX, textY, textWidth, textHeight};
            cmd.color = linearize({0.0f, 0.0f, 0.0f, 1.0f});
        } else {
            cmd.rect = rect;
            cmd.color = colorForPlaceholder(depth, isCard);
        }
    } else {
        if (isLeaf && !node.textContent.empty()) {
            RenderCommand textCmd;
            textCmd.type = RenderCommand::Type::Text;
            textCmd.rect = {
                rect.x + bl,
                rect.y + bt,
                std::max(0.0f, rect.width - bl - br),
                std::max(0.0f, rect.height - bt - bb)
            };
            textCmd.color = linearize(node.flex.color.value_or(node.color.value_or(colorFromDepthAndName(depth, node.name))));
            textCmd.textContent = node.textContent;
            textCmd.fontFamily = node.flex.fontFamily;
            textCmd.fontId = UINT32_MAX;
            textCmd.fontSize = node.flex.fontSize > 0.0f ? node.flex.fontSize : 14.0f;
            textCmd.fontWeight = node.flex.fontWeight;
            textCmd.layer = static_cast<int32_t>(node.order);
            textCmd.opacity = node.opacity;
            textCmd.zIndex = node.positioning.zIndex;
            applyScissor(textCmd);
            commands.push_back(textCmd);
        } else {
            cmd.rect = rect;
            cmd.color = linearize(node.color.value_or(colorFromDepthAndName(depth, node.name)));
        }
    }

    if (cmd.type == RenderCommand::Type::Rect) {
        cmd.layer = static_cast<int32_t>(node.order);
        cmd.opacity = node.opacity;
        cmd.zIndex = node.positioning.zIndex;
        applyScissor(cmd);
        commands.push_back(cmd);
    }

    for (const auto* child : node.children) {
        auto childCommands = buildRenderTree(*child, depth + 1, placeholderMode, &currentScissor);
        commands.insert(commands.end(), childCommands.begin(), childCommands.end());
    }

    return commands;
}

} // namespace vkapp::Graphics
