#include "Core/Window.h"
#include "Layout/FlexLayoutEngine.h"
#include "Layout/LayoutDumper.h"
#include "Graphics/RenderCommandBuilder.h"
#include "Graphics/VulkanRenderer.h"

#include <iostream>
#include <string>

using namespace vkapp::Layout;

int main()
{
    try {
        vkapp::Core::EventDispatcher dispatcher;
        vkapp::Core::Window window({
            .width = 800,
            .height = 300,
            .title = "Flex Example: 01-basic-row",
            .resizable = true
        });

        if (!window.initialize(dispatcher)) {
            return EXIT_FAILURE;
        }

        LayoutNode root{"root"};
        root.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            align-items: stretch;
        )");
        root.explicitWidth = 800.0f;
        root.explicitHeight = 300.0f;
        root.hasExplicitWidth = true;
        root.hasExplicitHeight = true;
        root.isFlexContainer = true;

        LayoutNode item1{"item-1"};
        item1.flex.parseStyle("flex-grow: 0;");
        item1.explicitWidth = 200.0f;
        item1.hasExplicitWidth = true;

        LayoutNode item2{"item-2"};
        item2.flex.parseStyle("flex-grow: 0;");
        item2.explicitWidth = 200.0f;
        item2.hasExplicitWidth = true;

        LayoutNode item3{"item-3"};
        item3.flex.parseStyle("flex-grow: 0;");
        item3.explicitWidth = 200.0f;
        item3.hasExplicitWidth = true;

        root.addChild(&item1);
        root.addChild(&item2);
        root.addChild(&item3);

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(root, 800.0f, 300.0f);

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
