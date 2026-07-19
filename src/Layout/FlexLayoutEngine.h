#pragma once

#include "Layout/LayoutNode.h"

namespace vkapp::Layout {

class FlexLayoutEngine {
public:
    void computeLayout(LayoutNode& node, float availableWidth, float availableHeight);

private:
    void resolveFlexContainer(LayoutNode& node, float availableWidth, float availableHeight);
    void measureNode(LayoutNode& node, float availableWidth, float availableHeight);
    void positionNode(LayoutNode& node, float x, float y, float width, float height);
    float resolveMainAxisSize(const LayoutNode& node) const;
    float resolveCrossAxisSize(const LayoutNode& node) const;
    bool isRowDirection(FlexDirection direction) const;
};

} // namespace vkapp::Layout
