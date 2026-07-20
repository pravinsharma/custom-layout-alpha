#include "Core/Window.h"
#include "Layout/FlexLayoutEngine.h"
#include "Layout/LayoutDumper.h"
#include "Layout/HtmlParser.h"
#include "Graphics/RenderCommandBuilder.h"
#include "Graphics/VulkanRenderer.h"

#include <iostream>

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

        double lastInteractionTime = glfwGetTime();

        dispatcher.addListener([&](const vkapp::Core::Event& event) -> bool {
            if (event.isInCategory(vkapp::Core::EventCategory::Input))
            {
                lastInteractionTime = glfwGetTime();
            }
            return false;
        });

        auto rootCss = parseCss(R"(
            body {
                display: flex;
                flex-direction: column;
                background-color: #FAFAFA;
                margin: 0;
                padding: 24px;
                gap: 20px;
            }
            .hero {
                display: flex;
                flex-direction: column;
                padding: 32px;
                background-color: #FFFFFF;
                border-width: 1px;
                border-color: #E2E8F0;
                border-radius: 8px;
                gap: 8px;
            }
            .section {
                display: flex;
                flex-direction: column;
                padding: 20px;
                border-width: 1px;
                border-color: #E2E8F0;
                border-radius: 8px;
                gap: 12px;
                overflow: hidden;
            }
            .section-1 {
                background-color: #E6FFFA;
            }
            .section-2 {
                background-color: #FFFBEB;
            }
            .section-3 {
                background-color: #F0FFF4;
            }
            .section-label {
                height: 24px;
            }
            .row {
                display: flex;
                flex-direction: row;
                gap: 24px;
                overflow: hidden;
            }
            .col {
                display: flex;
                flex-direction: column;
                gap: 8px;
                flex: 1;
            }
            .card {
                display: flex;
                flex-direction: column;
                padding: 14px;
                background-color: #F7FAFC;
                border-width: 1px;
                border-color: #E2E8F0;
                border-radius: 6px;
                gap: 6px;
            }
            .card-label {
                height: 14px;
            }
            .heading1 {
                height: 36px;
            }
            .heading2 {
                height: 28px;
            }
            .heading3 {
                height: 24px;
            }
            .body-text {
                height: 22px;
            }
            .caption-text {
                height: 20px;
            }
            .mono-text {
                height: 20px;
            }
        )");

        auto rootHtml = parseHtml(R"(
            <body class="body" width="800" height="900">
              <div class="hero">
                <div class="heading1">Typography Design Essentials</div>
                <div class="body-text">Good type. Better design.</div>
              </div>

              <div class="section section-1">
                <div class="section-label">01  Font Families</div>
                <div class="row">
                  <div class="card">
                    <div class="card-label">Roboto — Sans Serif</div>
                    <div class="heading1">The quick brown fox</div>
                  </div>
                  <div class="card">
                    <div class="card-label">Open Sans — Humanist</div>
                    <div class="body-text">The quick brown fox</div>
                  </div>
                  <div class="card">
                    <div class="card-label">Times New Roman — Serif</div>
                    <div class="body-text" style="font-family:'Times New Roman';">The quick brown fox</div>
                  </div>
                  <div class="card">
                    <div class="card-label">Courier New — Monospace</div>
                    <div class="mono-text">The quick brown fox</div>
                  </div>
                </div>
              </div>

              <div class="section section-2">
                <div class="section-label">02  Type Scale</div>
                <div class="col">
                  <div class="heading1">Heading 1 — 32px Bold</div>
                  <div class="heading2">Heading 2 — 22px Medium</div>
                  <div class="heading3">Heading 3 — 18px Condensed Bold</div>
                  <div class="body-text">Body — 16px Open Sans Regular. Typography is the art and technique of arranging type to make written language legible, readable and visually appealing.</div>
                  <div class="caption-text">Caption — 13px Open Sans Light. Used for secondary information and labels.</div>
                  <div class="mono-text">Mono — 16px Courier New. var x = 42; let name = "VePL";</div>
                </div>
              </div>

              <div class="section section-3">
                <div class="section-label">03  Font Weights (Open Sans)</div>
                <div class="row">
                  <div class="card">
                    <div class="card-label">Light (300)</div>
                    <div class="caption-text" style="font-weight:300;">Aa Bb Cc</div>
                  </div>
                  <div class="card">
                    <div class="card-label">Regular (400)</div>
                    <div class="body-text" style="font-weight:400;">Aa Bb Cc</div>
                  </div>
                  <div class="card">
                    <div class="card-label">Bold (700)</div>
                    <div class="body-text" style="font-weight:700;">Aa Bb Cc</div>
                  </div>
                </div>
              </div>
            </body>
        )");

        applyCss(*rootHtml, rootCss);

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(*rootHtml, 800.0f, 900.0f);

        auto commands = vkapp::Graphics::buildRenderTree(*rootHtml, 0, true);

        std::cout << "Computed layout:\n";
        for (const auto &cmd : commands)
        {
            std::cout << "  " << cmd.rect.x << ", " << cmd.rect.y << " "
                      << cmd.rect.width << "x" << cmd.rect.height << "\n";
        }

        std::cout << "\nLayout dump:\n";
        std::cout << vkapp::Layout::LayoutDumper::dumpTree(*rootHtml, {.includeRenderXray = true,
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

            if (glfwGetTime() - lastInteractionTime > 4.0)
            {
                glfwSetWindowShouldClose(window.getHandle(), GLFW_TRUE);
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
