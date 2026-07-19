#include "Window.h"

#include <iostream>

int main()
{
    try {
        vkapp::Window window({
            .width = 1280,
            .height = 720,
            .title = "Vulkan App",
            .resizable = true,
            .fullscreen = false
        });

        if (!window.initialize()) {
            return EXIT_FAILURE;
        }

        window.setResizeCallback([](uint32_t width, uint32_t height) {
            std::cout << "Framebuffer resized: " << width << "x" << height << "\n";
        });

        while (!window.shouldClose()) {
            window.pollEvents();
        }
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
