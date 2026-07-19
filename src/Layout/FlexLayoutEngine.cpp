#include "FlexLayoutEngine.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

namespace vkapp::Layout {

bool FlexLayoutEngine::isRowDirection(FlexDirection direction) const
{
    return direction == FlexDirection::Row || direction == FlexDirection::RowReverse;
}

float FlexLayoutEngine::resolveMainAxisSize(const LayoutNode& node) const
{
    return isRowDirection(node.flex.direction) ? node.computedRect.width : node.computedRect.height;
}

float FlexLayoutEngine::resolveCrossAxisSize(const LayoutNode& node) const
{
    return isRowDirection(node.flex.direction) ? node.computedRect.height : node.computedRect.width;
}

void FlexLayoutEngine::measureNode(LayoutNode& node, float availableWidth, float availableHeight)
{
    if (!node.hasExplicitWidth) {
        float maxChildWidth = 0.0f;
        for (const auto* child : node.children) {
            maxChildWidth = std::max(maxChildWidth, child->flex.flexBasis + child->box.marginHorizontal());
        }
        node.measuredSize.width = std::max(node.flex.flexBasis, maxChildWidth);
    } else {
        node.measuredSize.width = availableWidth;
    }

    if (!node.hasExplicitHeight) {
        float maxChildHeight = 0.0f;
        for (const auto* child : node.children) {
            maxChildHeight = std::max(maxChildHeight, child->flex.flexBasis + child->box.marginVertical());
        }
        node.measuredSize.height = std::max(node.flex.flexBasis, maxChildHeight);
    } else {
        node.measuredSize.height = availableHeight;
    }

    node.measuredSize.width = std::clamp(node.measuredSize.width, node.flex.minWidth, node.flex.maxWidth);
    node.measuredSize.height = std::clamp(node.measuredSize.height, node.flex.minHeight, node.flex.maxHeight);
}

void FlexLayoutEngine::positionNode(LayoutNode& node, float x, float y, float width, float height)
{
    node.computedRect.x = x;
    node.computedRect.y = y;
    node.computedRect.width = width;
    node.computedRect.height = height;
}

float FlexLayoutEngine::resolveJustifyOffset(JustifyContent justify, float freeSpace, int itemCount, float gap) const
{
    if (itemCount <= 0 || freeSpace <= 0.0f) {
        return 0.0f;
    }

    switch (justify) {
        case JustifyContent::Center:
            return freeSpace / 2.0f;
        case JustifyContent::FlexEnd:
            return freeSpace;
        case JustifyContent::SpaceBetween: {
            if (itemCount <= 1) {
                return 0.0f;
            }
            return 0.0f; // handled in item spacing
        }
        case JustifyContent::SpaceAround:
            return freeSpace / (2.0f * itemCount);
        case JustifyContent::SpaceEvenly:
            return freeSpace / (itemCount + 1);
        default:
            return 0.0f;
    }
}

float FlexLayoutEngine::resolveItemSpacing(JustifyContent justify, float freeSpace, int itemCount, float gap) const
{
    if (itemCount <= 1 || freeSpace <= 0.0f) {
        return 0.0f;
    }

    switch (justify) {
        case JustifyContent::SpaceBetween:
            return freeSpace / static_cast<float>(itemCount - 1);
        case JustifyContent::SpaceAround:
            return freeSpace / static_cast<float>(itemCount);
        case JustifyContent::SpaceEvenly:
            return freeSpace / static_cast<float>(itemCount + 1);
        default:
            return 0.0f;
    }
}

void FlexLayoutEngine::resolveFlexContainer(LayoutNode& node, float availableWidth, float availableHeight)
{
    if (node.children.empty()) {
        return;
    }

    measureNode(node, availableWidth, availableHeight);

    const bool row = isRowDirection(node.flex.direction);
    const bool reverse = (node.flex.direction == FlexDirection::RowReverse || node.flex.direction == FlexDirection::ColumnReverse);

    const float mainAxisSize = resolveMainAxisSize(node);
    const float crossAxisSize = resolveCrossAxisSize(node);

    const float gapMain = row ? node.flex.gapRow : node.flex.gapColumn;
    const float gapCross = row ? node.flex.gapColumn : node.flex.gapRow;

    float totalGapMain = gapMain * static_cast<float>(std::max(0, static_cast<int>(node.children.size()) - 1));

    float flexGrowTotal = 0.0f;
    float flexShrinkTotal = 0.0f;
    float totalMainBasis = 0.0f;

    for (auto* child : node.children) {
        flexGrowTotal += child->flex.flexGrow;
        flexShrinkTotal += child->flex.flexShrink;
        totalMainBasis += child->flex.flexBasis;
    }

    float freeSpaceMain = mainAxisSize - totalGapMain - totalMainBasis;

    std::vector<float> childMainSizes;
    childMainSizes.reserve(node.children.size());

    for (auto* child : node.children) {
        float childMainSize = child->flex.flexBasis;

        if (freeSpaceMain > 0 && flexGrowTotal > 0 && child->flex.flexGrow > 0) {
            childMainSize += (freeSpaceMain * (child->flex.flexGrow / flexGrowTotal));
        } else if (freeSpaceMain < 0 && flexShrinkTotal > 0 && child->flex.flexShrink > 0) {
            childMainSize += (freeSpaceMain * (child->flex.flexShrink / flexShrinkTotal));
        }

        if (row) {
            childMainSize = std::clamp(childMainSize, child->flex.minWidth, child->flex.maxWidth);
        } else {
            childMainSize = std::clamp(childMainSize, child->flex.minHeight, child->flex.maxHeight);
        }

        childMainSizes.push_back(childMainSize);
    }

    float justifyOffset = resolveJustifyOffset(node.flex.justify, freeSpaceMain, static_cast<int>(node.children.size()), gapMain);
    float itemSpacing = resolveItemSpacing(node.flex.justify, freeSpaceMain, static_cast<int>(node.children.size()), gapMain);

    float currentMain = justifyOffset;
    float currentCross = 0.0f;

    if (reverse) {
        currentMain = mainAxisSize - justifyOffset;
    }

    for (size_t i = 0; i < node.children.size(); ++i) {
        auto* child = node.children[i];
        float childMainSize = childMainSizes[i];
        float childCrossSize = crossAxisSize - child->box.marginVertical();

        if (node.flex.wrap != FlexWrap::NoWrap) {
            if (reverse) {
                if (currentMain - childMainSize < 0.0f) {
                    currentMain = mainAxisSize;
                    currentCross += (row ? childCrossSize : childMainSize) + gapCross;
                }
            } else {
                if (currentMain + childMainSize > mainAxisSize) {
                    currentMain = justifyOffset;
                    currentCross += (row ? childCrossSize : childMainSize) + gapCross;
                }
            }
        }

        float x = node.computedRect.x + child->box.marginLeft;
        float y = node.computedRect.y + child->box.marginTop;

        if (row) {
            if (reverse) {
                x += currentMain - childMainSize;
                y += currentCross;
            } else {
                x += currentMain;
                y += currentCross;
            }
        } else {
            if (reverse) {
                x += currentCross;
                y += currentMain - childMainSize;
            } else {
                x += currentCross;
                y += currentMain;
            }
        }

        positionNode(*child, x, y,
                     row ? childMainSize : childCrossSize,
                     row ? childCrossSize : childMainSize);

        if (reverse) {
            currentMain -= childMainSize + gapMain + itemSpacing;
        } else {
            currentMain += childMainSize + gapMain + itemSpacing;
        }
    }
}

void FlexLayoutEngine::computeLayout(LayoutNode& node, float availableWidth, float availableHeight)
{
    if (!node.isFlexContainer || node.children.empty()) {
        node.computedRect.width = availableWidth;
        node.computedRect.height = availableHeight;
        return;
    }

    node.computedRect.width = availableWidth;
    node.computedRect.height = availableHeight;

    resolveFlexContainer(node, availableWidth, availableHeight);

    for (auto* child : node.children) {
        computeLayout(*child, child->computedRect.width, child->computedRect.height);
    }
}

} // namespace vkapp::Layout
