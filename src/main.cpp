#include "Core/Window.h"
#include "Layout/FlexLayoutEngine.h"
#include "Layout/LayoutDumper.h"
#include "Layout/FlexTests.h"
#include "Graphics/RenderCommandBuilder.h"
#include "Graphics/VulkanRenderer.h"
#include "System/Console.h"

#include <iostream>

int main()
{
    try {
        std::cout << "=== Flex Layout Tests ===\n\n";
        int testResult = vkapp::Layout::Test::runAllTests();

        if (testResult != 0) {
            std::cerr << "\nSome tests failed.\n";
            return EXIT_FAILURE;
        }

        std::cout << "\nAll tests passed.\n";

        vkapp::Core::EventDispatcher dispatcher;
        vkapp::Core::Window window({
            .width = 800,
            .height = 600,
            .title = "Flex Layout Demo",
            .resizable = true
        });

        if (!window.initialize(dispatcher)) {
            return EXIT_FAILURE;
        }

        vkapp::Layout::LayoutNode root{"root"};
        root.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            justify-content: space-between;
            align-items: stretch;
            gap: 10px;
        )");
        root.isFlexContainer = true;

        vkapp::Layout::LayoutNode child1{"child1"};
        child1.flex.parseStyle(R"(
            flex-grow: 1;
            min-width: 50px;
            max-width: 200px;
        )");

        vkapp::Layout::LayoutNode child2{"child2"};
        child2.flex.parseStyle(R"(
            flex-grow: 2;
            min-width: 50px;
            max-width: 400px;
        )");

        vkapp::Layout::LayoutNode child3{"child3"};
        child3.flex.parseStyle(R"(
            flex-grow: 1;
            min-width: 50px;
            max-width: 200px;
        )");

        root.addChild(&child1);
        root.addChild(&child2);
        root.addChild(&child3);

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(root, 800.0f, 600.0f);

        auto commands = vkapp::Graphics::buildRenderTree(root);

        std::cout << "\nComputed layout:\n";
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
