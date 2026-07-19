#include "RenderCommandBuilder.h"
#include "Layout/LayoutNode.h"

namespace vkapp::Graphics {

RenderCommandList buildRenderTree(const vkapp::Layout::LayoutNode& node)
{
    RenderCommandList commands;
    commands.reserve(16);

    if (node.flex.display == Layout::Display::None) {
        return commands;
    }

    RenderCommand cmd;
    cmd.type = RenderCommand::Type::Rect;
    cmd.rect = node.computedRect;
    if (node.name == "root") {
        cmd.color = {0.15f, 0.17f, 0.20f, 1.0f}; // Dark gray/blue for root
    } else if (node.name == "child1") {
        cmd.color = {0.9f, 0.3f, 0.3f, 1.0f}; // Warm coral/red
    } else if (node.name == "child2") {
        cmd.color = {0.2f, 0.7f, 0.5f, 1.0f}; // Emerald green
    } else if (node.name == "child3") {
        cmd.color = {0.2f, 0.5f, 0.8f, 1.0f}; // Sky blue
    } else {
        cmd.color = {0.8f, 0.8f, 0.2f, 1.0f}; // Default yellow
    }
    cmd.layer = static_cast<int32_t>(node.order);
    commands.push_back(cmd);

    for (const auto* child : node.children) {
        auto childCommands = buildRenderTree(*child);
        commands.insert(commands.end(), childCommands.begin(), childCommands.end());
    }

    return commands;
}

} // namespace vkapp::Graphics
