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

        auto root = parseHtml(R"(
            <body class="body" width="800" height="900"
                  style="display:flex; flex-direction:column; gap:20px; padding:24px; align-items:stretch; background-color:#FAFAFA;">
              <div class="hero"
                   style="display:flex; flex-direction:column; gap:8px; padding:32px; background-color:#FFFFFF; border-color:#E3E8ED;">
                <div class="hero-h1" style="flex-grow:0; height:40px;"></div>
                <div class="hero-body" style="flex-grow:0; height:24px;"></div>
              </div>
              <section class="section-1"
                       style="display:flex; flex-direction:column; gap:12px; padding:20px; align-items:stretch; background-color:#FFFFFF; border-color:#E3E8ED;">
                <div class="label-1" style="flex-grow:0; height:26px;">Typography</div>
                <div class="row-1"
                     style="display:flex; flex-direction:row; gap:24px; align-items:stretch;">
                  <div class="card-1a"
                       style="display:flex; flex-direction:column; gap:4px; padding:16px; flex-grow:1; height:92px; background-color:#F7F9FB; border-color:#E3E8ED;">
                    <div class="card-1a-label" style="flex-grow:0; height:16px;"></div>
                    <div class="card-1a-text" style="flex-grow:0; height:40px;"></div>
                  </div>
                  <div class="card-1b"
                       style="display:flex; flex-direction:column; gap:4px; padding:16px; flex-grow:1; height:76px; background-color:#F7F9FB; border-color:#E3E8ED;">
                    <div class="card-1b-label" style="flex-grow:0; height:16px;"></div>
                    <div class="card-1b-text" style="flex-grow:0; height:24px;"></div>
                  </div>
                  <div class="card-1c"
                       style="display:flex; flex-direction:column; gap:4px; padding:16px; flex-grow:1; height:76px; background-color:#F7F9FB; border-color:#E3E8ED;">
                    <div class="card-1c-label" style="flex-grow:0; height:16px;"></div>
                    <div class="card-1c-text" style="flex-grow:0; height:24px;"></div>
                  </div>
                  <div class="card-1d"
                       style="display:flex; flex-direction:column; gap:4px; padding:16px; flex-grow:1; height:76px; background-color:#F7F9FB; border-color:#E3E8ED;">
                    <div class="card-1d-label" style="flex-grow:0; height:16px;"></div>
                    <div class="card-1d-text" style="flex-grow:0; height:24px;"></div>
                  </div>
                </div>
              </section>
              <section class="section-2"
                       style="display:flex; flex-direction:column; gap:12px; padding:20px; align-items:stretch; background-color:#FFFFFF; border-color:#E3E8ED;">
                <div class="label-2" style="flex-grow:0; height:26px;">Scale</div>
                <div class="col-2"
                     style="display:flex; flex-direction:column; gap:8px; align-items:stretch;">
                  <div class="h1" style="flex-grow:0; height:40px;">Heading 1</div>
                  <div class="h2" style="flex-grow:0; height:32px;">Heading 2</div>
                  <div class="h3" style="flex-grow:0; height:26px;">Heading 3</div>
                  <div class="body-text" style="flex-grow:0; height:24px;">Body text</div>
                  <div class="caption" style="flex-grow:0; height:20px;">Caption</div>
                  <div class="mono" style="flex-grow:0; height:24px;">Mono</div>
                </div>
              </section>
              <section class="section-3"
                       style="display:flex; flex-direction:column; gap:12px; padding:20px; align-items:stretch; background-color:#FFFFFF; border-color:#E3E8ED;">
                <div class="label-3" style="flex-grow:0; height:26px;">Cards</div>
                <div class="row-3"
                     style="display:flex; flex-direction:row; gap:24px; align-items:stretch;">
                  <div class="card-3a"
                       style="display:flex; flex-direction:column; gap:4px; padding:16px; flex-grow:1; height:72px; background-color:#F7F9FB; border-color:#E3E8ED;">
                    <div class="card-3a-label" style="flex-grow:0; height:16px;"></div>
                    <div class="card-3a-text" style="flex-grow:0; height:20px;"></div>
                  </div>
                  <div class="card-3b"
                       style="display:flex; flex-direction:column; gap:4px; padding:16px; flex-grow:1; height:76px; background-color:#F7F9FB; border-color:#E3E8ED;">
                    <div class="card-3b-label" style="flex-grow:0; height:16px;"></div>
                    <div class="card-3b-text" style="flex-grow:0; height:24px;"></div>
                  </div>
                  <div class="card-3c"
                       style="display:flex; flex-direction:column; gap:4px; padding:16px; flex-grow:1; height:76px; background-color:#F7F9FB; border-color:#E3E8ED;">
                    <div class="card-3c-label" style="flex-grow:0; height:16px;"></div>
                    <div class="card-3c-text" style="flex-grow:0; height:24px;"></div>
                  </div>
                </div>
              </section>
            </body>
        )");

        vkapp::Layout::FlexLayoutEngine engine;
        engine.computeLayout(*root, 800.0f, 900.0f);

        auto commands = vkapp::Graphics::buildRenderTree(*root, 0, true);

        std::cout << "Computed layout:\n";
        for (const auto &cmd : commands)
        {
            std::cout << "  " << cmd.rect.x << ", " << cmd.rect.y << " "
                      << cmd.rect.width << "x" << cmd.rect.height << "\n";
        }

        std::cout << "\nLayout dump:\n";
        std::cout << vkapp::Layout::LayoutDumper::dumpTree(*root, {.includeRenderXray = true,
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
