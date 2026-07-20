#pragma once

#include "Layout/FlexStyle.h"
#include "Layout/LayoutNode.h"

#include <map>
#include <memory>
#include <string>

namespace vkapp::Layout {

struct HtmlTreeDeleter {
    void operator()(LayoutNode* node) const {
        if (!node) return;
        for (auto* child : node->children) {
            operator()(child);
        }
        delete node;
    }
};

using HtmlTree = std::unique_ptr<LayoutNode, HtmlTreeDeleter>;
using Stylesheet = std::map<std::string, std::string>;

HtmlTree parseHtml(const std::string& html);
Stylesheet parseCss(const std::string& css);
void applyCss(LayoutNode& root, const Stylesheet& stylesheet);

} // namespace vkapp::Layout
