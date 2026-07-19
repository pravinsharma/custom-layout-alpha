#pragma once

#include "Layout/LayoutNode.h"
#include "Graphics/RenderCommand.h"

#include <vector>

namespace vkapp::Graphics {

RenderCommandList buildRenderTree(const vkapp::Layout::LayoutNode& node);

} // namespace vkapp::Graphics
