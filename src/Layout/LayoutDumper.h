#pragma once

#include "Layout/LayoutNode.h"

#include <string>

namespace vkapp::Layout {

struct LayoutDumpOptions {
    bool includeBoxModel = true;
    bool includeFlexStyle = true;
    bool includeComputed = true;
    bool includeMeasured = true;
    bool includeChildren = true;
    bool includeIssues = true;
    std::string indentString = "  ";
};

class LayoutDumper {
public:
    static std::string dump(const LayoutNode& node, const LayoutDumpOptions& options = {});
    static std::string dumpTree(const LayoutNode& root, const LayoutDumpOptions& options = {});
    static std::string detectIssues(const LayoutNode& node);

private:
    static std::string dumpNode(const LayoutNode& node, int depth, const LayoutDumpOptions& options);
};

} // namespace vkapp::Layout
