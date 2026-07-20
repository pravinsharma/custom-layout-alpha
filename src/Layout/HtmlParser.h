#pragma once

#include "Layout/FlexStyle.h"
#include "Layout/LayoutNode.h"

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

HtmlTree parseHtml(const std::string& html);

} // namespace vkapp::Layout
