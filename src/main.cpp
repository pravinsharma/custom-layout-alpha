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

            const auto& keyboard = window.keyboard();
            const auto& mouse = window.mouse();

            if (keyboard.wasKeyPressed(GLFW_KEY_ESCAPE)) {
                std::cout << "Escape pressed\n";
                glfwSetWindowShouldClose(window.getHandle(), GLFW_TRUE);
            }

            if (mouse.wasButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                auto [x, y] = mouse.getPosition();
                std::cout << "Left mouse button pressed at: " << x << ", " << y << "\n";
            }

            auto [scrollX, scrollY] = mouse.getScrollOffset();
            if (scrollX != 0.0 || scrollY != 0.0) {
                std::cout << "Scroll: " << scrollX << ", " << scrollY << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
