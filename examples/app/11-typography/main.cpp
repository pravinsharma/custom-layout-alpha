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
        vkapp::Core::Window window({.width = 800,
                                    .height = 900,
                                    .title = "Flex Example: 11-typography",
                                    .resizable = true});

        if (!window.initialize(dispatcher))
        {
            return EXIT_FAILURE;
        }

        LayoutNode body{"body"};
        body.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 20px;
            padding: 24px;
            background-color: #FAFAFA;
            align-items: stretch;
        )");
        body.explicitWidth = 800.0f;
        body.explicitHeight = 900.0f;
        body.hasExplicitWidth = true;
        body.hasExplicitHeight = true;
        body.isFlexContainer = true;
        body.backgroundColor = {0.98f, 0.98f, 0.98f, 1.0f};

        LayoutNode hero{"hero"};
        hero.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 8px;
            padding: 32px;
            align-items: stretch;
        )");
        hero.isFlexContainer = true;
        hero.backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
        hero.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode heroH1{"hero-h1"};
        heroH1.flex.parseStyle("flex-grow: 0;");
        heroH1.explicitHeight = 40.0f;
        heroH1.hasExplicitHeight = true;
        heroH1.color = {0.18f, 0.24f, 0.35f, 1.0f};

        LayoutNode heroBody{"hero-body"};
        heroBody.flex.parseStyle("flex-grow: 0;");
        heroBody.explicitHeight = 24.0f;
        heroBody.hasExplicitHeight = true;
        heroBody.color = {0.35f, 0.41f, 0.85f, 1.0f};

        hero.addChild(&heroH1);
        hero.addChild(&heroBody);

        LayoutNode section1{"section-1"};
        section1.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 12px;
            padding: 20px;
            align-items: stretch;
        )");
        section1.isFlexContainer = true;
        section1.backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
        section1.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode label1{"label-1"};
        label1.flex.parseStyle("flex-grow: 0;");
        label1.explicitHeight = 26.0f;
        label1.hasExplicitHeight = true;
        label1.color = {0.35f, 0.41f, 0.85f, 1.0f};

        LayoutNode row1{"row-1"};
        row1.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            gap: 24px;
            align-items: stretch;
        )");
        row1.isFlexContainer = true;

        LayoutNode card1a{"card-1a"};
        card1a.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 4px;
            padding: 16px;
            flex-grow: 1;
            align-items: stretch;
        )");
        card1a.explicitHeight = 92.0f;
        card1a.hasExplicitHeight = true;
        card1a.isFlexContainer = true;
        card1a.backgroundColor = {0.97f, 0.98f, 0.99f, 1.0f};
        card1a.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode card1aLabel{"card-1a-label"};
        card1aLabel.flex.parseStyle("flex-grow: 0;");
        card1aLabel.explicitHeight = 16.0f;
        card1aLabel.hasExplicitHeight = true;
        card1aLabel.color = {0.63f, 0.68f, 0.75f, 1.0f};

        LayoutNode card1aText{"card-1a-text"};
        card1aText.flex.parseStyle("flex-grow: 0;");
        card1aText.explicitHeight = 40.0f;
        card1aText.hasExplicitHeight = true;
        card1aText.color = {0.18f, 0.24f, 0.35f, 1.0f};

        card1a.addChild(&card1aLabel);
        card1a.addChild(&card1aText);

        LayoutNode card1b{"card-1b"};
        card1b.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 4px;
            padding: 16px;
            flex-grow: 1;
            align-items: stretch;
        )");
        card1b.explicitHeight = 76.0f;
        card1b.hasExplicitHeight = true;
        card1b.isFlexContainer = true;
        card1b.backgroundColor = {0.97f, 0.98f, 0.99f, 1.0f};
        card1b.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode card1bLabel{"card-1b-label"};
        card1bLabel.flex.parseStyle("flex-grow: 0;");
        card1bLabel.explicitHeight = 16.0f;
        card1bLabel.hasExplicitHeight = true;
        card1bLabel.color = {0.63f, 0.68f, 0.75f, 1.0f};

        LayoutNode card1bText{"card-1b-text"};
        card1bText.flex.parseStyle("flex-grow: 0;");
        card1bText.explicitHeight = 24.0f;
        card1bText.hasExplicitHeight = true;
        card1bText.color = {0.18f, 0.24f, 0.35f, 1.0f};

        card1b.addChild(&card1bLabel);
        card1b.addChild(&card1bText);

        LayoutNode card1c{"card-1c"};
        card1c.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 4px;
            padding: 16px;
            flex-grow: 1;
            align-items: stretch;
        )");
        card1c.explicitHeight = 76.0f;
        card1c.hasExplicitHeight = true;
        card1c.isFlexContainer = true;
        card1c.backgroundColor = {0.97f, 0.98f, 0.99f, 1.0f};
        card1c.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode card1cLabel{"card-1c-label"};
        card1cLabel.flex.parseStyle("flex-grow: 0;");
        card1cLabel.explicitHeight = 16.0f;
        card1cLabel.hasExplicitHeight = true;
        card1cLabel.color = {0.63f, 0.68f, 0.75f, 1.0f};

        LayoutNode card1cText{"card-1c-text"};
        card1cText.flex.parseStyle("flex-grow: 0;");
        card1cText.explicitHeight = 24.0f;
        card1cText.hasExplicitHeight = true;
        card1cText.color = {0.18f, 0.24f, 0.35f, 1.0f};

        card1c.addChild(&card1cLabel);
        card1c.addChild(&card1cText);

        LayoutNode card1d{"card-1d"};
        card1d.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 4px;
            padding: 16px;
            flex-grow: 1;
            align-items: stretch;
        )");
        card1d.explicitHeight = 76.0f;
        card1d.hasExplicitHeight = true;
        card1d.isFlexContainer = true;
        card1d.backgroundColor = {0.97f, 0.98f, 0.99f, 1.0f};
        card1d.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode card1dLabel{"card-1d-label"};
        card1dLabel.flex.parseStyle("flex-grow: 0;");
        card1dLabel.explicitHeight = 16.0f;
        card1dLabel.hasExplicitHeight = true;
        card1dLabel.color = {0.63f, 0.68f, 0.75f, 1.0f};

        LayoutNode card1dText{"card-1d-text"};
        card1dText.flex.parseStyle("flex-grow: 0;");
        card1dText.explicitHeight = 24.0f;
        card1dText.hasExplicitHeight = true;
        card1dText.color = {0.18f, 0.24f, 0.35f, 1.0f};

        card1d.addChild(&card1dLabel);
        card1d.addChild(&card1dText);

        row1.addChild(&card1a);
        row1.addChild(&card1b);
        row1.addChild(&card1c);
        row1.addChild(&card1d);

        section1.addChild(&label1);
        section1.addChild(&row1);

        LayoutNode section2{"section-2"};
        section2.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 12px;
            padding: 20px;
            align-items: stretch;
        )");
        section2.isFlexContainer = true;
        section2.backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
        section2.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode label2{"label-2"};
        label2.flex.parseStyle("flex-grow: 0;");
        label2.explicitHeight = 26.0f;
        label2.hasExplicitHeight = true;
        label2.color = {0.35f, 0.41f, 0.85f, 1.0f};

        LayoutNode col2{"col-2"};
        col2.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 8px;
            align-items: stretch;
        )");
        col2.isFlexContainer = true;

        LayoutNode h1{"h1"};
        h1.flex.parseStyle("flex-grow: 0;");
        h1.explicitHeight = 40.0f;
        h1.hasExplicitHeight = true;
        h1.color = {0.18f, 0.24f, 0.35f, 1.0f};

        LayoutNode h2{"h2"};
        h2.flex.parseStyle("flex-grow: 0;");
        h2.explicitHeight = 32.0f;
        h2.hasExplicitHeight = true;
        h2.color = {0.18f, 0.24f, 0.35f, 1.0f};

        LayoutNode h3{"h3"};
        h3.flex.parseStyle("flex-grow: 0;");
        h3.explicitHeight = 26.0f;
        h3.hasExplicitHeight = true;
        h3.color = {0.35f, 0.41f, 0.85f, 1.0f};

        LayoutNode bodyText{"body-text"};
        bodyText.flex.parseStyle("flex-grow: 0;");
        bodyText.explicitHeight = 24.0f;
        bodyText.hasExplicitHeight = true;
        bodyText.color = {0.18f, 0.24f, 0.35f, 1.0f};

        LayoutNode caption{"caption"};
        caption.flex.parseStyle("flex-grow: 0;");
        caption.explicitHeight = 20.0f;
        caption.hasExplicitHeight = true;
        caption.color = {0.63f, 0.68f, 0.75f, 1.0f};

        LayoutNode mono{"mono"};
        mono.flex.parseStyle("flex-grow: 0;");
        mono.explicitHeight = 24.0f;
        mono.hasExplicitHeight = true;
        mono.color = {0.18f, 0.24f, 0.35f, 1.0f};

        col2.addChild(&h1);
        col2.addChild(&h2);
        col2.addChild(&h3);
        col2.addChild(&bodyText);
        col2.addChild(&caption);
        col2.addChild(&mono);

        section2.addChild(&label2);
        section2.addChild(&col2);

        LayoutNode section3{"section-3"};
        section3.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 12px;
            padding: 20px;
            align-items: stretch;
        )");
        section3.isFlexContainer = true;
        section3.backgroundColor = {1.0f, 1.0f, 1.0f, 1.0f};
        section3.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode label3{"label-3"};
        label3.flex.parseStyle("flex-grow: 0;");
        label3.explicitHeight = 26.0f;
        label3.hasExplicitHeight = true;
        label3.color = {0.35f, 0.41f, 0.85f, 1.0f};

        LayoutNode row3{"row-3"};
        row3.flex.parseStyle(R"(
            display: flex;
            flex-direction: row;
            gap: 24px;
            align-items: stretch;
        )");
        row3.isFlexContainer = true;

        LayoutNode card3a{"card-3a"};
        card3a.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 4px;
            padding: 16px;
            flex-grow: 1;
            align-items: stretch;
        )");
        card3a.explicitHeight = 72.0f;
        card3a.hasExplicitHeight = true;
        card3a.isFlexContainer = true;
        card3a.backgroundColor = {0.97f, 0.98f, 0.99f, 1.0f};
        card3a.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode card3aLabel{"card-3a-label"};
        card3aLabel.flex.parseStyle("flex-grow: 0;");
        card3aLabel.explicitHeight = 16.0f;
        card3aLabel.hasExplicitHeight = true;
        card3aLabel.color = {0.63f, 0.68f, 0.75f, 1.0f};

        LayoutNode card3aText{"card-3a-text"};
        card3aText.flex.parseStyle("flex-grow: 0;");
        card3aText.explicitHeight = 20.0f;
        card3aText.hasExplicitHeight = true;
        card3aText.color = {0.18f, 0.24f, 0.35f, 1.0f};

        card3a.addChild(&card3aLabel);
        card3a.addChild(&card3aText);

        LayoutNode card3b{"card-3b"};
        card3b.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 4px;
            padding: 16px;
            flex-grow: 1;
            align-items: stretch;
        )");
        card3b.explicitHeight = 76.0f;
        card3b.hasExplicitHeight = true;
        card3b.isFlexContainer = true;
        card3b.backgroundColor = {0.97f, 0.98f, 0.99f, 1.0f};
        card3b.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode card3bLabel{"card-3b-label"};
        card3bLabel.flex.parseStyle("flex-grow: 0;");
        card3bLabel.explicitHeight = 16.0f;
        card3bLabel.hasExplicitHeight = true;
        card3bLabel.color = {0.63f, 0.68f, 0.75f, 1.0f};

        LayoutNode card3bText{"card-3b-text"};
        card3bText.flex.parseStyle("flex-grow: 0;");
        card3bText.explicitHeight = 24.0f;
        card3bText.hasExplicitHeight = true;
        card3bText.color = {0.18f, 0.24f, 0.35f, 1.0f};

        card3b.addChild(&card3bLabel);
        card3b.addChild(&card3bText);

        LayoutNode card3c{"card-3c"};
        card3c.flex.parseStyle(R"(
            display: flex;
            flex-direction: column;
            gap: 4px;
            padding: 16px;
            flex-grow: 1;
            align-items: stretch;
        )");
        card3c.explicitHeight = 76.0f;
        card3c.hasExplicitHeight = true;
        card3c.isFlexContainer = true;
        card3c.backgroundColor = {0.97f, 0.98f, 0.99f, 1.0f};
        card3c.borderColor = {0.89f, 0.91f, 0.94f, 1.0f};

        LayoutNode card3cLabel{"card-3c-label"};
        card3cLabel.flex.parseStyle("flex-grow: 0;");
        card3cLabel.explicitHeight = 16.0f;
        card3cLabel.hasExplicitHeight = true;
        card3cLabel.color = {0.63f, 0.68f, 0.75f, 1.0f};

        LayoutNode card3cText{"card-3c-text"};
        card3cText.flex.parseStyle("flex-grow: 0;");
        card3cText.explicitHeight = 24.0f;
        card3cText.hasExplicitHeight = true;
        card3cText.color = {0.18f, 0.24f, 0.35f, 1.0f};

        card3c.addChild(&card3cLabel);
        card3c.addChild(&card3cText);

        row3.addChild(&card3a);
        row3.addChild(&card3b);
        row3.addChild(&card3c);

        section3.addChild(&label3);
        section3.addChild(&row3);

        body.addChild(&hero);
        body.addChild(&section1);
        body.addChild(&section2);
        body.addChild(&section3);

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(body, 800.0f, 900.0f);

        auto commands = vkapp::Graphics::buildRenderTree(body, 0, true);

        std::cout << "Computed layout:\n";
        for (const auto &cmd : commands)
        {
            std::cout << "  " << cmd.rect.x << ", " << cmd.rect.y << " "
                      << cmd.rect.width << "x" << cmd.rect.height << "\n";
        }

        std::cout << "\nLayout dump:\n";
        std::cout << vkapp::Layout::LayoutDumper::dumpTree(body, {.includeRenderXray = true,
                                                                  .placeholderMode = true});

        vkapp::Graphics::VulkanRenderer renderer(window.getHandle());
        if (!renderer.initialize())
        {
            std::cerr << "Failed to initialize Vulkan renderer\n";
            return EXIT_FAILURE;
        }

        std::cout << "\nRendering... Close the window to exit.\n";

        while (!window.shouldClose())
        {
            window.pollEvents();
            renderer.render(commands);

            if (renderer.needsResize())
            {
                renderer.resetResize();
            }
        }

        renderer.waitIdle();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
