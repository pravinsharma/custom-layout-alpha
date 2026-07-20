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
    try
    {
        vkapp::Core::EventDispatcher dispatcher;
        vkapp::Core::Window window({.width = 400,
                                    .height = 300,
                                    .title = "Flex Example: 08-gap",
                                    .resizable = true});

        if (!window.initialize(dispatcher))
        {
            return EXIT_FAILURE;
        }

        LayoutNode root{"root"};
        root.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            gap: 16px;
            align-items: stretch;
        )");
        root.explicitWidth = 400.0f;
        root.explicitHeight = 300.0f;
        root.hasExplicitWidth = true;
        root.hasExplicitHeight = true;
        root.isFlexContainer = true;

        LayoutNode col{"col"};
        col.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap-row: 10px;
            gap-column: 4px;
        )");
        col.explicitWidth = 120.0f;
        col.explicitHeight = 200.0f;
        col.hasExplicitWidth = true;
        col.hasExplicitHeight = true;
        col.isFlexContainer = true;

        LayoutNode colItem1{"col-item-1"};
        colItem1.flex.parseStyle("flex-grow: 0;");
        colItem1.explicitHeight = 60.0f;
        colItem1.hasExplicitHeight = true;

        LayoutNode colItem2{"col-item-2"};
        colItem2.flex.parseStyle("flex-grow: 0;");
        colItem2.explicitHeight = 60.0f;
        colItem2.hasExplicitHeight = true;

        LayoutNode colItem3{"col-item-3"};
        colItem3.flex.parseStyle("flex-grow: 0;");
        colItem3.explicitHeight = 60.0f;
        colItem3.hasExplicitHeight = true;

        col.addChild(&colItem1);
        col.addChild(&colItem2);
        col.addChild(&colItem3);

        LayoutNode box{"box"};
        box.flex.parseStyle("flex-grow: 0;");
        box.explicitWidth = 120.0f;
        box.explicitHeight = 200.0f;
        box.hasExplicitWidth = true;
        box.hasExplicitHeight = true;

        root.addChild(&col);
        root.addChild(&box);

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(root, 400.0f, 300.0f);

        auto commands = vkapp::Graphics::buildRenderTree(root);

        std::cout << "Computed layout:\n";
        for (const auto &cmd : commands)
        {
            std::cout << "  " << cmd.rect.x << ", " << cmd.rect.y << " "
                      << cmd.rect.width << "x" << cmd.rect.height << "\n";
        }

        std::cout << "\nLayout dump:\n";
        std::cout << vkapp::Layout::LayoutDumper::dumpTree(root);

        vkapp::Graphics::VulkanRenderer renderer(window.getHandle());
        if (!renderer.initialize())
        {
            std::cerr << "Failed to initialize Vulkan renderer\n";
            return EXIT_FAILURE;
        }

        std::cout << "\nRendering... Close the window to exit.\n";

        // while (!window.shouldClose()) {
        window.pollEvents();
        renderer.render(commands);
        //}

        renderer.waitIdle();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
