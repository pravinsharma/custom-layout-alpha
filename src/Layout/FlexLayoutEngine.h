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
    float resolveJustifyOffset(JustifyContent justify, float freeSpace, int itemCount, float gap) const;
    float resolveItemSpacing(JustifyContent justify, float freeSpace, int itemCount, float gap) const;
    float resolveAlignItemsOffset(AlignItems align, float itemSize, float lineSize) const;
    float resolveAlignContentOffset(AlignContent align, float totalLinesSize, float containerSize, int lineCount) const;
    float estimateIntrinsicSize(const LayoutNode& child) const;
};

} // namespace vkapp::Layout
