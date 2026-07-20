#pragma once

#include "Rect.h"
#include "BoxModel.h"
#include "FlexStyle.h"
#include "Positioning.h"
#include "../Graphics/RenderCommand.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace vkapp::Layout {

struct LayoutNode {
    std::string name;
    BoxModel box;
    FlexStyle flex;
    Positioning positioning;

    std::vector<LayoutNode*> children;

    Rect computedRect{};
    Size measuredSize{};

    float explicitWidth = 0.0f;
    float explicitHeight = 0.0f;
    bool hasExplicitWidth = false;
    bool hasExplicitHeight = false;
    bool isFlexContainer = false;
    uint32_t order = 0;
    float aspectRatio = 0.0f;
    float opacity = 1.0f;

    std::optional<Graphics::Color> color;
    std::optional<Graphics::Color> backgroundColor;
    std::optional<Graphics::Color> borderColor;

    LayoutNode* parent = nullptr;

    bool isInFlexFlow() const
    {
        return positioning.type != Position::Absolute && positioning.type != Position::Fixed;
    }

    void addChild(LayoutNode* child)
    {
        child->parent = this;
        children.push_back(child);
    }
};

} // namespace vkapp::Layout
