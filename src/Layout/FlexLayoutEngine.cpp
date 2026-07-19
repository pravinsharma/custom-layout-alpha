#include "FlexLayoutEngine.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

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

float FlexLayoutEngine::estimateIntrinsicSize(const LayoutNode& child) const
{
    if (child.hasExplicitWidth || child.hasExplicitHeight) {
        return 0.0f;
    }

    if (child.flex.flexBasis > 0.0f) {
        return child.flex.flexBasis;
    }

    return 50.0f;
}

void FlexLayoutEngine::measureNode(LayoutNode& node, float availableWidth, float availableHeight)
{
    if (!node.hasExplicitWidth) {
        float maxChildWidth = 0.0f;
        for (const auto* child : node.children) {
            const float intrinsic = estimateIntrinsicSize(*child);
            const float basis = child->flex.flexBasis > 0.0f ? child->flex.flexBasis : intrinsic;
            maxChildWidth = std::max(maxChildWidth, basis + child->box.marginHorizontal());
        }
        node.measuredSize.width = std::max(node.flex.flexBasis, maxChildWidth);
    } else {
        node.measuredSize.width = node.explicitWidth;
    }

    if (!node.hasExplicitHeight) {
        float maxChildHeight = 0.0f;
        for (const auto* child : node.children) {
            const float intrinsic = estimateIntrinsicSize(*child);
            const float basis = child->flex.flexBasis > 0.0f ? child->flex.flexBasis : intrinsic;
            maxChildHeight = std::max(maxChildHeight, basis + child->box.marginVertical());
        }
        node.measuredSize.height = std::max(node.flex.flexBasis, maxChildHeight);
    } else {
        node.measuredSize.height = node.explicitHeight;
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

float FlexLayoutEngine::resolveAlignItemsOffset(AlignItems align, float itemSize, float lineSize) const
{
    switch (align) {
        case AlignItems::FlexStart:
            return 0.0f;
        case AlignItems::FlexEnd:
            return lineSize - itemSize;
        case AlignItems::Center:
            return (lineSize - itemSize) / 2.0f;
        case AlignItems::Stretch:
            return 0.0f;
        case AlignItems::Baseline:
            return 0.0f;
        default:
            return 0.0f;
    }
}

float FlexLayoutEngine::resolveAlignContentOffset(AlignContent align, float totalLinesSize, float containerSize, int lineCount) const
{
    if (lineCount <= 0 || totalLinesSize >= containerSize) {
        return 0.0f;
    }

    const float freeSpace = containerSize - totalLinesSize;

    switch (align) {
        case AlignContent::FlexStart:
            return 0.0f;
        case AlignContent::FlexEnd:
            return freeSpace;
        case AlignContent::Center:
            return freeSpace / 2.0f;
        case AlignContent::SpaceBetween: {
            if (lineCount <= 1) {
                return 0.0f;
            }
            return 0.0f;
        }
        case AlignContent::SpaceAround:
            return freeSpace / (2.0f * lineCount);
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

    float flexGrowTotal = 0.0f;
    float flexShrinkTotal = 0.0f;

    for (auto* child : node.children) {
        flexGrowTotal += child->flex.flexGrow;
        flexShrinkTotal += child->flex.flexShrink;
    }

    std::vector<std::vector<size_t>> lines;
    std::vector<float> lineMainSizes;
    std::vector<float> lineCrossSizes;

    if (node.flex.wrap != FlexWrap::NoWrap) {
        std::vector<size_t> currentLine;
        float currentLineMain = 0.0f;
        float currentLineCross = 0.0f;

        for (size_t i = 0; i < node.children.size(); ++i) {
            auto* child = node.children[i];
            float intrinsic = estimateIntrinsicSize(*child);
            float childMainSize = child->flex.flexBasis > 0.0f ? child->flex.flexBasis : intrinsic;
            childMainSize = std::clamp(childMainSize, child->flex.minWidth, child->flex.maxWidth);

            float childNaturalCrossSize = 0.0f;
            if (row) {
                childNaturalCrossSize = child->hasExplicitHeight ? child->explicitHeight : (crossAxisSize - child->box.marginVertical());
            } else {
                childNaturalCrossSize = child->hasExplicitWidth ? child->explicitWidth : (crossAxisSize - child->box.marginVertical());
            }

            if (!currentLine.empty() && currentLineMain + gapMain + childMainSize > mainAxisSize) {
                lines.push_back(currentLine);
                lineMainSizes.push_back(currentLineMain);
                lineCrossSizes.push_back(currentLineCross);
                currentLine.clear();
                currentLineMain = 0.0f;
                currentLineCross = 0.0f;
            }

            float lineGap = currentLine.empty() ? 0.0f : gapMain;
            currentLine.push_back(i);
            currentLineMain += childMainSize + lineGap;
            currentLineCross = std::max(currentLineCross, childNaturalCrossSize);
        }

        if (!currentLine.empty()) {
            lines.push_back(currentLine);
            lineMainSizes.push_back(currentLineMain);
            lineCrossSizes.push_back(currentLineCross);
        }
    } else {
        lines.push_back(std::vector<size_t>(node.children.size()));
        std::iota(lines.back().begin(), lines.back().end(), 0);
        lineMainSizes.push_back(mainAxisSize);
        lineCrossSizes.push_back(crossAxisSize);
    }

    float totalLinesCrossSize = 0.0f;
    for (float lineCross : lineCrossSizes) {
        totalLinesCrossSize += lineCross;
    }
    totalLinesCrossSize += gapCross * static_cast<float>(std::max(0, static_cast<int>(lineCrossSizes.size()) - 1));

    float alignContentOffset = resolveAlignContentOffset(
        node.flex.alignContent,
        totalLinesCrossSize,
        crossAxisSize,
        static_cast<int>(lineCrossSizes.size())
    );

    float currentCross = alignContentOffset;

    for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        const auto& line = lines[lineIndex];
        const float lineMainSize = lineMainSizes[lineIndex];
        const float lineCrossSize = lineCrossSizes[lineIndex];

        float totalMainBasis = 0.0f;
        for (size_t idx : line) {
            auto* child = node.children[idx];
            totalMainBasis += child->flex.flexBasis;
        }

        float freeSpaceMain = mainAxisSize - gapMain * static_cast<float>(std::max(0, static_cast<int>(line.size()) - 1)) - totalMainBasis;

        float justifyOffset = resolveJustifyOffset(node.flex.justify, freeSpaceMain, static_cast<int>(line.size()), gapMain);
        float itemSpacing = resolveItemSpacing(node.flex.justify, freeSpaceMain, static_cast<int>(line.size()), gapMain);

        float currentMain = justifyOffset;

        if (reverse) {
            currentMain = mainAxisSize - justifyOffset;
        }

        std::vector<float> childMainSizes;
        childMainSizes.reserve(line.size());

        for (size_t idx : line) {
            auto* child = node.children[idx];
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

        for (size_t i = 0; i < line.size(); ++i) {
            auto* child = node.children[line[i]];
            float childMainSize = childMainSizes[i];

            float childNaturalCrossSize = 0.0f;
            if (row) {
                childNaturalCrossSize = child->hasExplicitHeight ? child->explicitHeight : (lineCrossSize - child->box.marginVertical());
            } else {
                childNaturalCrossSize = child->hasExplicitWidth ? child->explicitWidth : (lineCrossSize - child->box.marginVertical());
            }

            float childCrossSize = childNaturalCrossSize;

            if (node.flex.alignItems == AlignItems::Stretch) {
                if (row && !child->hasExplicitHeight) {
                    childCrossSize = lineCrossSize - child->box.marginVertical();
                } else if (!row && !child->hasExplicitWidth) {
                    childCrossSize = lineCrossSize - child->box.marginVertical();
                }
            }

            float alignOffset = resolveAlignItemsOffset(node.flex.alignItems, childCrossSize, lineCrossSize);

            float x = node.computedRect.x + child->box.marginLeft;
            float y = node.computedRect.y + child->box.marginTop + currentCross;

            if (row) {
                if (reverse) {
                    x += currentMain - childMainSize;
                } else {
                    x += currentMain;
                }
                y += alignOffset;
            } else {
                if (reverse) {
                    y += currentMain - childMainSize;
                } else {
                    y += currentMain;
                }
                x += alignOffset;
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

        currentCross += lineCrossSize + gapCross;
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
