#include "Core/Window.h"
#include "Layout/FlexLayoutEngine.h"
#include "Graphics/RenderCommandBuilder.h"
#include "System/Console.h"

#include <iostream>

int main()
{
    try {
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
        root.flex.display = vkapp::Layout::Display::Flex;
        root.flex.direction = vkapp::Layout::FlexDirection::Row;
        root.flex.justify = vkapp::Layout::JustifyContent::SpaceBetween;
        root.flex.alignItems = vkapp::Layout::AlignItems::Stretch;
        root.flex.gapRow = 10.0f;
        root.isFlexContainer = true;

        vkapp::Layout::LayoutNode child1{"child1"};
        child1.flex.flexGrow = 1.0f;
        child1.flex.minWidth = 50.0f;
        child1.flex.maxWidth = 200.0f;

        vkapp::Layout::LayoutNode child2{"child2"};
        child2.flex.flexGrow = 2.0f;
        child2.flex.minWidth = 50.0f;
        child2.flex.maxWidth = 400.0f;

        vkapp::Layout::LayoutNode child3{"child3"};
        child3.flex.flexGrow = 1.0f;
        child3.flex.minWidth = 50.0f;
        child3.flex.maxWidth = 200.0f;

        root.addChild(&child1);
        root.addChild(&child2);
        root.addChild(&child3);

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(root, 800.0f, 600.0f);

        auto commands = vkapp::Graphics::buildRenderTree(root);

        std::cout << "Computed layout:\n";
        for (const auto& cmd : commands) {
            std::cout << "  " << cmd.rect.x << ", " << cmd.rect.y << " "
                      << cmd.rect.width << "x" << cmd.rect.height << "\n";
        }

        while (!window.shouldClose()) {
            window.pollEvents();
        }
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
