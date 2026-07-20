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
    try
    {
        vkapp::Core::EventDispatcher dispatcher;
        vkapp::Core::Window window({.width = 800,
                                    .height = 600,
                                    .title = "Flex Example: 10-complete-dashboard",
                                    .resizable = true});

        if (!window.initialize(dispatcher))
        {
            return EXIT_FAILURE;
        }

        LayoutNode root{"root"};
        root.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 0px;
        )");
        root.explicitWidth = 800.0f;
        root.explicitHeight = 600.0f;
        root.hasExplicitWidth = true;
        root.hasExplicitHeight = true;
        root.isFlexContainer = true;

        LayoutNode header{"header"};
        header.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            justify-content: space-between;
            align-items: center;
            gap: 16px;
            padding: 0 16px;
        )");
        header.explicitHeight = 56.0f;
        header.hasExplicitHeight = true;
        header.isFlexContainer = true;

        LayoutNode logo{"logo"};
        logo.flex.parseStyle("flex-grow: 0; min-width: 100px; max-width: 160px;");
        logo.explicitHeight = 36.0f;
        logo.hasExplicitHeight = true;

        LayoutNode nav{"nav"};
        nav.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            justify-content: flex-end;
            align-items: center;
            gap: 12px;
            min-width: 200px;
            max-width: 360px;
        )");
        nav.explicitHeight = 36.0f;
        nav.hasExplicitHeight = true;
        nav.isFlexContainer = true;

        LayoutNode navItem1{"nav-item-1"};
        navItem1.flex.parseStyle("flex-grow: 0; min-width: 50px; max-width: 100px;");
        navItem1.explicitHeight = 32.0f;
        navItem1.hasExplicitHeight = true;

        LayoutNode navItem2{"nav-item-2"};
        navItem2.flex.parseStyle("flex-grow: 0; min-width: 50px; max-width: 100px;");
        navItem2.explicitHeight = 32.0f;
        navItem2.hasExplicitHeight = true;

        nav.addChild(&navItem1);
        nav.addChild(&navItem2);
        header.addChild(&logo);
        header.addChild(&nav);

        LayoutNode body{"body"};
        body.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            gap: 16px;
            padding: 16px;
        )");
        body.flex.flexGrow = 1.0f;
        body.isFlexContainer = true;

        LayoutNode sidebar{"sidebar"};
        sidebar.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 8px;
        )");
        sidebar.explicitWidth = 160.0f;
        sidebar.explicitHeight = 168.0f;
        sidebar.hasExplicitWidth = true;
        sidebar.hasExplicitHeight = true;
        sidebar.isFlexContainer = true;

        LayoutNode sideItem1{"side-item-1"};
        sideItem1.flex.parseStyle("flex-grow: 0;");
        sideItem1.explicitHeight = 80.0f;
        sideItem1.hasExplicitHeight = true;

        LayoutNode sideItem2{"side-item-2"};
        sideItem2.flex.parseStyle("flex-grow: 0;");
        sideItem2.explicitHeight = 80.0f;
        sideItem2.hasExplicitHeight = true;

        sidebar.addChild(&sideItem1);
        sidebar.addChild(&sideItem2);

        LayoutNode main{"main"};
        main.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            flex-wrap: wrap;
            gap: 12px;
            align-content: flex-start;
        )");
        main.flex.flexGrow = 1.0f;
        main.isFlexContainer = true;

        std::vector<LayoutNode> cards;
        cards.reserve(4);
        for (int i = 0; i < 4; ++i)
        {
            cards.emplace_back("card-" + std::to_string(i + 1));
            auto &card = cards.back();
            card.flex.parseStyle("flex-grow: 1; flex-basis: 120px;");
            card.explicitHeight = 100.0f;
            card.hasExplicitHeight = true;
            main.addChild(&card);
        }

        body.addChild(&sidebar);
        body.addChild(&main);

        LayoutNode footer{"footer"};
        footer.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            justify-content: center;
            align-items: center;
            gap: 24px;
        )");
        footer.flex.flexGrow = 0.0f;
        footer.explicitHeight = 48.0f;
        footer.hasExplicitHeight = true;
        footer.isFlexContainer = true;

        LayoutNode footerLink1{"footer-link-1"};
        footerLink1.flex.parseStyle("flex-grow: 0;");
        footerLink1.explicitWidth = 60.0f;
        footerLink1.explicitHeight = 24.0f;
        footerLink1.hasExplicitWidth = true;
        footerLink1.hasExplicitHeight = true;

        LayoutNode footerLink2{"footer-link-2"};
        footerLink2.flex.parseStyle("flex-grow: 0;");
        footerLink2.explicitWidth = 60.0f;
        footerLink2.explicitHeight = 24.0f;
        footerLink2.hasExplicitWidth = true;
        footerLink2.hasExplicitHeight = true;

        footer.addChild(&footerLink1);
        footer.addChild(&footerLink2);

        root.addChild(&header);
        root.addChild(&body);
        root.addChild(&footer);

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(root, 800.0f, 600.0f);

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
