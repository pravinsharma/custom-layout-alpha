#include "Core/Window.h"
#include "Core/EventDispatcher.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "System/Console.h"

#include <iostream>

int main()
{
    try {
        vkapp::Core::EventDispatcher dispatcher;
        vkapp::Core::Window window({
            .width = 1280,
            .height = 720,
            .title = "Vulkan App",
            .resizable = true,
            .fullscreen = false
        });

        if (!window.initialize(dispatcher)) {
            return EXIT_FAILURE;
        }

        vkapp::Input::Keyboard keyboard;
        vkapp::Input::Mouse mouse;
        keyboard.registerListeners(dispatcher);
        mouse.registerListeners(dispatcher);

        window.setResizeCallback([](uint32_t width, uint32_t height) {
            std::cout << "Framebuffer resized: " << width << "x" << height << "\n";
        });

        dispatcher.addListener([](const vkapp::Core::Event& event) {
            if (event.getType() == vkapp::Core::EventType::WindowClose) {
                std::cout << "Window close event received\n";
                return true;
            }
            return false;
        });

        while (!window.shouldClose()) {
            window.pollEvents();
            keyboard.beginFrame();
            mouse.beginFrame();

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
