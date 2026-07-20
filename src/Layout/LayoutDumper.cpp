#include "LayoutDumper.h"

#include "Graphics/RenderCommandBuilder.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace vkapp::Layout {

std::string LayoutDumper::detectIssues(const LayoutNode& node)
{
    std::ostringstream issues;

    if (node.computedRect.width < 0.0f || node.computedRect.height < 0.0f) {
        issues << "[NEGATIVE_SIZE] ";
    }

    if (node.hasExplicitWidth && node.computedRect.width > node.flex.maxWidth + 0.01f) {
        issues << "[WIDTH_OVER_MAX] ";
    }
    if (node.hasExplicitHeight && node.computedRect.height > node.flex.maxHeight + 0.01f) {
        issues << "[HEIGHT_OVER_MAX] ";
    }

    if (node.flex.flexGrow > 0.0f && node.computedRect.width < 1.0f) {
        issues << "[GROW_ZERO_RESULT] ";
    }

    const float contentWidth = node.computedRect.width - node.box.paddingHorizontal() - node.box.borderHorizontal();
    const float contentHeight = node.computedRect.height - node.box.paddingVertical() - node.box.borderVertical();
    if (contentWidth < 0.0f || contentHeight < 0.0f) {
        issues << "[BOX_BLEED] ";
    }

    return issues.str();
}

std::string LayoutDumper::dumpNode(const LayoutNode& node, int depth, const LayoutDumpOptions& options)
{
    std::ostringstream out;
    const std::string indent = std::string(depth * 2, ' ');

    out << indent << "Node: " << (node.name.empty() ? "(unnamed)" : node.name) << "\n";

    if (options.includeComputed) {
        out << indent << "  computedRect: x=" << node.computedRect.x << " y=" << node.computedRect.y
            << " w=" << node.computedRect.width << " h=" << node.computedRect.height << "\n";
    }

    if (options.includeMeasured) {
        out << indent << "  measuredSize: w=" << node.measuredSize.width << " h=" << node.measuredSize.height << "\n";
    }

    if (options.includeBoxModel) {
        out << indent << "  boxModel:\n";
        out << indent << "    margin: t=" << node.box.marginTop << " r=" << node.box.marginRight
            << " b=" << node.box.marginBottom << " l=" << node.box.marginLeft << "\n";
        out << indent << "    padding: t=" << node.box.paddingTop << " r=" << node.box.paddingRight
            << " b=" << node.box.paddingBottom << " l=" << node.box.paddingLeft << "\n";
        out << indent << "    border: t=" << node.box.borderTop << " r=" << node.box.borderRight
            << " b=" << node.box.borderBottom << " l=" << node.box.borderLeft << "\n";
    }

    if (options.includeFlexStyle) {
        out << indent << "  flexStyle:\n";
        out << indent << "    display=" << static_cast<uint32_t>(node.flex.display)
            << " direction=" << static_cast<uint32_t>(node.flex.direction)
            << " wrap=" << static_cast<uint32_t>(node.flex.wrap) << "\n";
        out << indent << "    justify=" << static_cast<uint32_t>(node.flex.justify)
            << " alignItems=" << static_cast<uint32_t>(node.flex.alignItems)
            << " alignContent=" << static_cast<uint32_t>(node.flex.alignContent) << "\n";
        out << indent << "    grow=" << node.flex.flexGrow << " shrink=" << node.flex.flexShrink
            << " basis=" << node.flex.flexBasis << "\n";
        out << indent << "    gapRow=" << node.flex.gapRow << " gapColumn=" << node.flex.gapColumn << "\n";
        out << indent << "    minWidth=" << node.flex.minWidth << " maxWidth=" << node.flex.maxWidth
            << " minHeight=" << node.flex.minHeight << " maxHeight=" << node.flex.maxHeight << "\n";
    }

    if (options.includeRenderXray) {
        const auto info = vkapp::Graphics::getNodeRenderInfo(node, depth, options.placeholderMode);

        out << indent << "  renderXray:\n";
        out << indent << "    isCard=" << (info.isCard ? "true" : "false")
            << " isLeaf=" << (info.isLeaf ? "true" : "false") << "\n";
        out << indent << "    rect: x=" << info.renderX << " y=" << info.renderY
            << " w=" << info.renderWidth << " h=" << info.renderHeight << "\n";
        out << indent << "    color: r=" << info.colorR << " g=" << info.colorG
            << " b=" << info.colorB << " a=" << info.colorA << "\n";
        out << indent << "    layer=" << info.layer << " opacity=" << info.opacity
            << " zIndex=" << info.zIndex << "\n";

        if (info.hasTextPlaceholder) {
            out << indent << "    textRect: x=" << info.textX << " y=" << info.textY
                << " w=" << info.textWidth << " h=" << info.textHeight << "\n";
        }
    }

    if (options.includeIssues) {
        const std::string issues = detectIssues(node);
        if (!issues.empty()) {
            out << indent << "  issues: " << issues << "\n";
        }
    }

    if (options.includeChildren && !node.children.empty()) {
        out << indent << "  children (" << node.children.size() << "):\n";
        for (const auto* child : node.children) {
            out << dumpNode(*child, depth + 2, options);
        }
    }

    return out.str();
}

std::string LayoutDumper::dump(const LayoutNode& node, const LayoutDumpOptions& options)
{
    return dumpNode(node, 0, options);
}

std::string LayoutDumper::dumpTree(const LayoutNode& root, const LayoutDumpOptions& options)
{
    return dumpNode(root, 0, options);
}

} // namespace vkapp::Layout
