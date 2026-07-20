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
    const bool row = isRowDirection(node.flex.direction);

    auto childMainExtent = [&](const LayoutNode* child) -> float {
        if (child->isFlexContainer) {
            return row ? child->measuredSize.width : child->measuredSize.height;
        }
        if (row && child->hasExplicitWidth) {
            return child->explicitWidth;
        }
        if (!row && child->hasExplicitHeight) {
            return child->explicitHeight;
        }
        float basis = child->flex.flexBasis;
        if (!basis) {
            basis = estimateIntrinsicSize(*child);
        }
        return basis;
    };

    auto childCrossExtent = [&](const LayoutNode* child) -> float {
        if (child->isFlexContainer) {
            return row ? child->measuredSize.height : child->measuredSize.width;
        }
        if (row && child->hasExplicitHeight) {
            return child->explicitHeight;
        }
        if (!row && child->hasExplicitWidth) {
            return child->explicitWidth;
        }
        float basis = child->flex.flexBasis;
        if (!basis) {
            basis = estimateIntrinsicSize(*child);
        }
        return basis;
    };

    if (!node.hasExplicitWidth) {
        if (row) {
            float totalWidth = 0.0f;
            for (const auto* child : node.children) {
                totalWidth += childMainExtent(child) + child->box.marginHorizontal()
                             + child->box.paddingHorizontal() + child->box.borderHorizontal();
            }
            totalWidth += node.flex.gapColumn * static_cast<float>(std::max(0, static_cast<int>(node.children.size()) - 1));
            node.measuredSize.width = totalWidth;
        } else {
            float maxChildWidth = 0.0f;
            for (const auto* child : node.children) {
                maxChildWidth = std::max(maxChildWidth, childCrossExtent(child) + child->box.marginHorizontal()
                                         + child->box.paddingHorizontal() + child->box.borderHorizontal());
            }
            node.measuredSize.width = maxChildWidth;
        }
    } else {
        node.measuredSize.width = node.explicitWidth;
    }

    if (!node.hasExplicitHeight) {
        if (!row) {
            float totalHeight = 0.0f;
            for (const auto* child : node.children) {
                totalHeight += childMainExtent(child) + child->box.marginVertical()
                              + child->box.paddingVertical() + child->box.borderVertical();
            }
            totalHeight += node.flex.gapRow * static_cast<float>(std::max(0, static_cast<int>(node.children.size()) - 1));
            node.measuredSize.height = totalHeight;
        } else {
            float maxChildHeight = 0.0f;
            for (const auto* child : node.children) {
                maxChildHeight = std::max(maxChildHeight, childCrossExtent(child) + child->box.marginVertical()
                                          + child->box.paddingVertical() + child->box.borderVertical());
            }
            node.measuredSize.height = maxChildHeight;
        }
    } else {
        node.measuredSize.height = node.explicitHeight;
    }

    node.measuredSize.width += node.box.paddingHorizontal() + node.box.borderHorizontal();
    node.measuredSize.height += node.box.paddingVertical() + node.box.borderVertical();

    node.measuredSize.width = std::clamp(node.measuredSize.width, node.flex.minWidth, node.flex.maxWidth);
    node.measuredSize.height = std::clamp(node.measuredSize.height, node.flex.minHeight, node.flex.maxHeight);
}

void FlexLayoutEngine::positionNode(LayoutNode& node, float x, float y, float width, float height)
{
    node.computedRect.x = x;
    node.computedRect.y = y;
    node.computedRect.width = width + node.box.paddingHorizontal() + node.box.borderHorizontal();
    node.computedRect.height = height + node.box.paddingVertical() + node.box.borderVertical();
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

    for (auto* child : node.children) {
        measureNode(*child, availableWidth, availableHeight);
    }

    const bool row = isRowDirection(node.flex.direction);
    const bool reverse = (node.flex.direction == FlexDirection::RowReverse || node.flex.direction == FlexDirection::ColumnReverse);

    const float mainAxisSize = row ? availableWidth : availableHeight;
    const float crossAxisSize = row ? availableHeight : availableWidth;

    const float gapMain = row ? node.flex.gapColumn : node.flex.gapRow;
    const float gapCross = row ? node.flex.gapRow : node.flex.gapColumn;

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
            float childMainSize = 0.0f;
            if (child->isFlexContainer) {
                childMainSize = row ? child->measuredSize.width : child->measuredSize.height;
            } else {
                float intrinsic = estimateIntrinsicSize(*child);
                childMainSize = child->flex.flexBasis > 0.0f ? child->flex.flexBasis : intrinsic;
                childMainSize = std::clamp(childMainSize, child->flex.minWidth, child->flex.maxWidth);
                float childMainBorder = row ?
                    (child->box.paddingHorizontal() + child->box.borderHorizontal()) :
                    (child->box.paddingVertical() + child->box.borderVertical());
                childMainSize += childMainBorder;
            }

            float childNaturalCrossSize = 0.0f;
            if (row) {
                if (child->hasExplicitHeight) {
                    childNaturalCrossSize = child->explicitHeight;
                } else if (child->isFlexContainer) {
                    childNaturalCrossSize = child->measuredSize.height;
                } else {
                    childNaturalCrossSize = estimateIntrinsicSize(*child);
                }
            } else {
                if (child->hasExplicitWidth) {
                    childNaturalCrossSize = child->explicitWidth;
                } else if (child->isFlexContainer) {
                    childNaturalCrossSize = child->measuredSize.width;
                } else {
                    childNaturalCrossSize = estimateIntrinsicSize(*child);
                }
            }
            float childCrossBorder = row ?
                (child->box.paddingVertical() + child->box.borderVertical()) :
                (child->box.paddingHorizontal() + child->box.borderHorizontal());
            if (!child->isFlexContainer) {
                childNaturalCrossSize += childCrossBorder;
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

        float naturalCrossSize = 0.0f;
        for (auto* child : node.children) {
            float childCross = 0.0f;
            if (row) {
                if (child->hasExplicitHeight) {
                    childCross = child->explicitHeight;
                } else if (child->isFlexContainer) {
                    childCross = child->measuredSize.height;
                } else {
                    childCross = estimateIntrinsicSize(*child);
                }
            } else {
                if (child->hasExplicitWidth) {
                    childCross = child->explicitWidth;
                } else if (child->isFlexContainer) {
                    childCross = child->measuredSize.width;
                } else {
                    childCross = estimateIntrinsicSize(*child);
                }
            }
            float childCrossBorder = row ?
                (child->box.paddingVertical() + child->box.borderVertical()) :
                (child->box.paddingHorizontal() + child->box.borderHorizontal());
            float childMarginCross = row ? child->box.marginVertical() : child->box.marginHorizontal();
            float childCrossTotal = child->isFlexContainer ? childCross : childCross + childCrossBorder;
            naturalCrossSize = std::max(naturalCrossSize, childCrossTotal + childMarginCross);
        }
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
            float basis = 0.0f;
            if (child->isFlexContainer) {
                basis = row ? child->measuredSize.width : child->measuredSize.height;
            } else {
                basis = child->flex.flexBasis;
                if (row) {
                    if (!basis && child->hasExplicitWidth) {
                        basis = child->explicitWidth;
                    }
                } else {
                    if (!basis && child->hasExplicitHeight) {
                        basis = child->explicitHeight;
                    }
                }
            }
            totalMainBasis += basis;
        }

        float freeSpaceMain = mainAxisSize - gapMain * static_cast<float>(std::max(0, static_cast<int>(line.size()) - 1)) - totalMainBasis;

        std::vector<float> childMainSizes;
        childMainSizes.reserve(line.size());

        for (size_t idx : line) {
            auto* child = node.children[idx];
            float childMainSize = 0.0f;
            if (child->isFlexContainer) {
                childMainSize = row ? child->measuredSize.width : child->measuredSize.height;
            } else {
                childMainSize = child->flex.flexBasis;
                if (row) {
                    if (!child->flex.flexBasis && child->hasExplicitWidth) {
                        childMainSize = child->explicitWidth;
                    }
                } else {
                    if (!child->flex.flexBasis && child->hasExplicitHeight) {
                        childMainSize = child->explicitHeight;
                    }
                }
            }

            if (freeSpaceMain > 0 && flexGrowTotal > 0 && child->flex.flexGrow > 0) {
                childMainSize += (freeSpaceMain * (child->flex.flexGrow / flexGrowTotal));
            } else if (freeSpaceMain < 0 && flexShrinkTotal > 0 && child->flex.flexShrink > 0
                       && !child->hasExplicitWidth && !child->hasExplicitHeight) {
                childMainSize += (freeSpaceMain * (child->flex.flexShrink / flexShrinkTotal));
            }

            if (row) {
                childMainSize = std::clamp(childMainSize, child->flex.minWidth, child->flex.maxWidth);
            } else {
                childMainSize = std::clamp(childMainSize, child->flex.minHeight, child->flex.maxHeight);
            }

            childMainSizes.push_back(childMainSize);
        }

        float totalChildMainSize = 0.0f;
        for (float size : childMainSizes) {
            totalChildMainSize += size;
        }

        float remainingSpace = mainAxisSize - gapMain * static_cast<float>(std::max(0, static_cast<int>(line.size()) - 1)) - totalChildMainSize;

        float justifyOffset = resolveJustifyOffset(node.flex.justify, remainingSpace, static_cast<int>(line.size()), gapMain);
        float itemSpacing = resolveItemSpacing(node.flex.justify, remainingSpace, static_cast<int>(line.size()), gapMain);

        float currentMain = justifyOffset;

        if (reverse) {
            currentMain = mainAxisSize - justifyOffset;
        }

        for (size_t i = 0; i < line.size(); ++i) {
            auto* child = node.children[line[i]];
            float childMainSize = childMainSizes[i];

            float childNaturalCrossSize = 0.0f;
            if (row) {
                if (child->hasExplicitHeight) {
                    childNaturalCrossSize = child->explicitHeight;
                } else if (child->isFlexContainer) {
                    childNaturalCrossSize = child->measuredSize.height;
                } else {
                    childNaturalCrossSize = estimateIntrinsicSize(*child);
                }
            } else {
                if (child->hasExplicitWidth) {
                    childNaturalCrossSize = child->explicitWidth;
                } else if (child->isFlexContainer) {
                    childNaturalCrossSize = child->measuredSize.width;
                } else {
                    childNaturalCrossSize = estimateIntrinsicSize(*child);
                }
            }
            float childCrossBorder = row ?
                (child->box.paddingVertical() + child->box.borderVertical()) :
                (child->box.paddingHorizontal() + child->box.borderHorizontal());
            if (!child->isFlexContainer) {
                childNaturalCrossSize += childCrossBorder;
            }

            float childCrossSize = childNaturalCrossSize;

            if (node.flex.alignItems == AlignItems::Stretch) {
                if (row && !child->hasExplicitHeight) {
                    childCrossSize = lineCrossSize - child->box.marginVertical();
                } else if (!row && !child->hasExplicitWidth) {
                    childCrossSize = lineCrossSize - child->box.marginHorizontal();
                }
            }

            float alignOffset = resolveAlignItemsOffset(node.flex.alignItems, childCrossSize, lineCrossSize);

            float x = node.computedRect.x + node.box.paddingLeft + child->box.marginLeft;
            float y = node.computedRect.y + node.box.paddingTop + child->box.marginTop + currentCross;

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

            if (node.name == "features") {
                std::cout << "  [POS " << child->name << "] x=" << x << " y=" << y
                          << " w=" << (row ? childMainSize : childCrossSize)
                          << " h=" << (row ? childCrossSize : childMainSize) << "\n";
            }

            if (i + 1 < line.size()) {
                if (reverse) {
                    currentMain -= childMainSize + gapMain + itemSpacing;
                } else {
                    currentMain += childMainSize + gapMain + itemSpacing;
                }
            }
        }

        currentCross += lineCrossSize + gapCross;
    }
}

void FlexLayoutEngine::computeLayout(LayoutNode& node, float availableWidth, float availableHeight)
{
    node.box.marginTop = node.flex.marginTop;
    node.box.marginRight = node.flex.marginRight;
    node.box.marginBottom = node.flex.marginBottom;
    node.box.marginLeft = node.flex.marginLeft;
    node.box.paddingTop = node.flex.paddingTop;
    node.box.paddingRight = node.flex.paddingRight;
    node.box.paddingBottom = node.flex.paddingBottom;
    node.box.paddingLeft = node.flex.paddingLeft;
    node.box.borderTop = node.flex.borderTop;
    node.box.borderRight = node.flex.borderRight;
    node.box.borderBottom = node.flex.borderBottom;
    node.box.borderLeft = node.flex.borderLeft;
    node.backgroundColor = node.flex.backgroundColor;
    node.borderColor = node.flex.borderColor;

    if (!node.isFlexContainer || node.children.empty()) {
        if (node.computedRect.width <= 0.0f) {
            if (node.hasExplicitWidth) {
                node.computedRect.width = node.explicitWidth;
            } else if (node.measuredSize.width > 0.0f) {
                node.computedRect.width = node.measuredSize.width;
            } else {
                node.computedRect.width = availableWidth;
            }
        }
        if (node.computedRect.height <= 0.0f) {
            if (node.hasExplicitHeight) {
                node.computedRect.height = node.explicitHeight;
            } else if (node.measuredSize.height > 0.0f) {
                node.computedRect.height = node.measuredSize.height;
            } else {
                node.computedRect.height = availableHeight;
            }
        }
        return;
    }

    node.computedRect.width = availableWidth;
    node.computedRect.height = availableHeight;

    float contentWidth = availableWidth - node.box.paddingHorizontal() - node.box.borderHorizontal();
    float contentHeight = availableHeight - node.box.paddingVertical() - node.box.borderVertical();

    resolveFlexContainer(node, contentWidth, contentHeight);

    for (auto* child : node.children) {
        computeLayout(*child, child->computedRect.width, child->computedRect.height);
    }
}

} // namespace vkapp::Layout
