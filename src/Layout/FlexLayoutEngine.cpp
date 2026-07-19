#include "FlexLayoutEngine.h"

#include <algorithm>
#include <cmath>
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
            if (child->hasExplicitWidth) {
                maxChildWidth += child->box.marginHorizontal() + child->computedRect.width;
            }
        }
        node.measuredSize.width = std::max(node.flex.flexBasis, maxChildWidth);
    } else {
        node.measuredSize.width = availableWidth;
    }

    if (!node.hasExplicitHeight) {
        float maxChildHeight = 0.0f;
        for (const auto* child : node.children) {
            if (child->hasExplicitHeight) {
                maxChildHeight += child->box.marginVertical() + child->computedRect.height;
            }
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

void FlexLayoutEngine::resolveFlexContainer(LayoutNode& node, float availableWidth, float availableHeight)
{
    if (node.children.empty()) {
        return;
    }

    measureNode(node, availableWidth, availableHeight);

    const float mainAxisSize = resolveMainAxisSize(node);
    const float crossAxisSize = resolveCrossAxisSize(node);
    const bool row = isRowDirection(node.flex.direction);

    const float gapMain = row ? node.flex.gapRow : node.flex.gapColumn;
    const float gapCross = row ? node.flex.gapColumn : node.flex.gapRow;

    float totalGapMain = gapMain * static_cast<float>(std::max(0, static_cast<int>(node.children.size()) - 1));
    float totalGapCross = gapCross * static_cast<float>(std::max(0, static_cast<int>(node.children.size()) - 1));

    float flexGrowTotal = 0.0f;
    float flexShrinkTotal = 0.0f;
    float totalMainBasis = 0.0f;

    for (auto* child : node.children) {
        flexGrowTotal += child->flex.flexGrow;
        flexShrinkTotal += child->flex.flexShrink;
        totalMainBasis += child->flex.flexBasis;
    }

    float freeSpaceMain = mainAxisSize - totalGapMain - totalMainBasis;

    float currentMain = 0.0f;
    float currentCross = 0.0f;

    for (auto* child : node.children) {
        float childMainSize = child->flex.flexBasis;

        if (freeSpaceMain > 0 && flexGrowTotal > 0 && child->flex.flexGrow > 0) {
            childMainSize += (freeSpaceMain * (child->flex.flexGrow / flexGrowTotal));
        } else if (freeSpaceMain < 0 && flexShrinkTotal > 0 && child->flex.flexShrink > 0) {
            childMainSize += (freeSpaceMain * (child->flex.flexShrink / flexShrinkTotal));
        }

        childMainSize = std::clamp(childMainSize, child->flex.minWidth, child->flex.maxWidth);

        float childCrossSize = crossAxisSize - child->box.marginVertical();
        if (child->flex.direction == FlexDirection::Column || child->flex.direction == FlexDirection::ColumnReverse) {
            childCrossSize = childMainSize;
        }

        if (row) {
            positionNode(*child, node.computedRect.x + child->box.marginLeft + currentMain,
                         node.computedRect.y + child->box.marginTop + currentCross,
                         childMainSize, childCrossSize);
        } else {
            positionNode(*child, node.computedRect.x + child->box.marginLeft + currentCross,
                         node.computedRect.y + child->box.marginTop + currentMain,
                         childCrossSize, childMainSize);
        }

        currentMain += childMainSize + gapMain;
        currentCross += childCrossSize + gapCross;
    }
}

void FlexLayoutEngine::computeLayout(LayoutNode& node, float availableWidth, float availableHeight)
{
    if (!node.isFlexContainer || node.children.empty()) {
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
