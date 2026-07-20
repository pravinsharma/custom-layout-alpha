#pragma once

#include "Layout/LayoutNode.h"
#include "Graphics/RenderCommand.h"

#include <string>
#include <vector>

namespace vkapp::Graphics {

RenderCommandList buildRenderTree(const vkapp::Layout::LayoutNode& node, int depth = 0, bool placeholderMode = false, const vkapp::Layout::Rect* parentScissor = nullptr);

struct NodeRenderInfo {
    bool isCard = false;
    bool isLeaf = false;
    float renderX = 0.0f;
    float renderY = 0.0f;
    float renderWidth = 0.0f;
    float renderHeight = 0.0f;
    float colorR = 0.0f;
    float colorG = 0.0f;
    float colorB = 0.0f;
    float colorA = 1.0f;
    int32_t layer = 0;
    float opacity = 1.0f;
    int32_t zIndex = 0;
    float textX = 0.0f;
    float textY = 0.0f;
    float textWidth = 0.0f;
    float textHeight = 0.0f;
    bool hasTextPlaceholder = false;
};

NodeRenderInfo getNodeRenderInfo(const vkapp::Layout::LayoutNode& node, int depth, bool placeholderMode);

} // namespace vkapp::Graphics
