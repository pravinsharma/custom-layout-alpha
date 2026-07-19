#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "Event.h"
#include "EventDispatcher.h"
#include "../Input/Keyboard.h"
#include "../Input/Mouse.h"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>
#include <string>

namespace vkapp::Core {

class Window {
public:
    struct Config {
        uint32_t width = 800;
        uint32_t height = 600;
        std::string title = "Vulkan App";
        bool resizable = true;
        bool fullscreen = false;
    };

    using ResizeCallback = std::function<void(uint32_t width, uint32_t height)>;

    explicit Window(Config config);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    bool initialize(EventDispatcher& dispatcher);
    void shutdown();

    GLFWwindow* getHandle() const noexcept { return m_window; }

    bool shouldClose() const;
    void pollEvents();

    bool createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) const;

    void setResizeCallback(ResizeCallback callback);
    uint32_t getInstanceExtensionCount() const;

    const Input::Keyboard& keyboard() const noexcept { return m_keyboard; }
    Input::Keyboard& keyboard() noexcept { return m_keyboard; }
    const Input::Mouse& mouse() const noexcept { return m_mouse; }
    Input::Mouse& mouse() noexcept { return m_mouse; }

    bool isInitialized() const noexcept { return m_initialized; }
    const Config& getConfig() const noexcept { return m_config; }

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double x, double y);
    static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    static void windowCloseCallback(GLFWwindow* window);

    static bool s_glfwInitialized;

    Config m_config;
    GLFWwindow* m_window = nullptr;
    ResizeCallback m_resizeCallback;
    uint32_t m_instanceExtensionCount = 0;
    EventDispatcher* m_dispatcher = nullptr;
    Input::Keyboard m_keyboard;
    Input::Mouse m_mouse;
    bool m_initialized = false;
};

} // namespace vkapp::Core
