#pragma once

#include "Rect.h"
#include "BoxModel.h"
#include "FlexStyle.h"

#include <cstdint>
#include <string>
#include <vector>

namespace vkapp::Layout {

struct LayoutNode {
    std::string name;
    BoxModel box;
    FlexStyle flex;

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

    LayoutNode* parent = nullptr;

    void addChild(LayoutNode* child)
    {
        child->parent = this;
        children.push_back(child);
    }
};

} // namespace vkapp::Layout
