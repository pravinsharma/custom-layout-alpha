#include "Core/Window.h"
#include "Layout/FlexLayoutEngine.h"
#include "Layout/LayoutDumper.h"
#include "Graphics/RenderCommandBuilder.h"
#include "Graphics/VulkanRenderer.h"

#include <iostream>
#include <string>
#include <vector>

using namespace vkapp::Layout;

int main()
{
    try {
        vkapp::Core::EventDispatcher dispatcher;
        vkapp::Core::Window window({
            .width = 400,
            .height = 300,
            .title = "Flex Example: 06-align-content",
            .resizable = true
        });

        if (!window.initialize(dispatcher)) {
            return EXIT_FAILURE;
        }

        LayoutNode root{"root"};
        root.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            flex-wrap: wrap;
            align-content: center;
            gap: 8px;
        )");
        root.explicitWidth = 400.0f;
        root.explicitHeight = 300.0f;
        root.hasExplicitWidth = true;
        root.hasExplicitHeight = true;
        root.isFlexContainer = true;

        std::vector<LayoutNode> items;
        items.reserve(6);
        for (int i = 0; i < 6; ++i) {
            items.emplace_back("item-" + std::to_string(i + 1));
            auto& item = items.back();
            item.flex.parseStyle("flex-grow: 0; flex-basis: 120px;");
            item.explicitHeight = 50.0f;
            item.hasExplicitHeight = true;
            root.addChild(&item);
        }

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(root, 400.0f, 300.0f);

        auto commands = vkapp::Graphics::buildRenderTree(root);

        std::cout << "Computed layout:\n";
        for (const auto& cmd : commands) {
            std::cout << "  " << cmd.rect.x << ", " << cmd.rect.y << " "
                      << cmd.rect.width << "x" << cmd.rect.height << "\n";
        }

        std::cout << "\nLayout dump:\n";
        std::cout << vkapp::Layout::LayoutDumper::dumpTree(root);

        vkapp::Graphics::VulkanRenderer renderer(window.getHandle());
        if (!renderer.initialize()) {
            std::cerr << "Failed to initialize Vulkan renderer\n";
            return EXIT_FAILURE;
        }

        std::cout << "\nRendering... Close the window to exit.\n";

        while (!window.shouldClose()) {
            window.pollEvents();
            renderer.render(commands);

            if (renderer.needsResize()) {
                renderer.resetResize();
            }
        }

        renderer.waitIdle();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
