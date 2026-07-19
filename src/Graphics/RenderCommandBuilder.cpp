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
    cmd.color = {0.2f, 0.6f, 0.9f, 1.0f};
    cmd.layer = static_cast<int32_t>(node.order);
    commands.push_back(cmd);

    for (const auto* child : node.children) {
        auto childCommands = buildRenderTree(*child);
        commands.insert(commands.end(), childCommands.begin(), childCommands.end());
    }

    return commands;
}

} // namespace vkapp::Graphics
