#include "Window.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

namespace vkapp {

bool Window::s_glfwInitialized = false;

Window::Window(Config config)
    : m_config(std::move(config))
{
}

Window::~Window()
{
    shutdown();
}

bool Window::initialize()
{
    if (m_initialized) {
        return true;
    }

    if (!s_glfwInitialized) {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW\n";
            return false;
        }

        glfwSetErrorCallback([](int error, const char* description) {
            std::cerr << "GLFW Error " << error << ": " << description << "\n";
        });

        s_glfwInitialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (!m_config.resizable) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    GLFWmonitor* monitor = m_config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    GLFWwindow* window = glfwCreateWindow(
        static_cast<int>(m_config.width),
        static_cast<int>(m_config.height),
        m_config.title.c_str(),
        monitor,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        return false;
    }

    m_window = window;
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);

    m_instanceExtensionCount = getInstanceExtensionCount();
    std::cout << m_instanceExtensionCount << " Vulkan instance extensions supported\n";

    m_initialized = true;
    return true;
}

void Window::shutdown()
{
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    m_initialized = false;
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents()
{
    m_keyboard.beginFrame();
    m_mouse.beginFrame();
    glfwPollEvents();
}

void Window::getFramebufferSize(uint32_t& width, uint32_t& height) const
{
    int w = 0;
    int h = 0;
    glfwGetFramebufferSize(m_window, &w, &h);
    width = static_cast<uint32_t>(w);
    height = static_cast<uint32_t>(h);
}

bool Window::createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) const
{
    if (!m_window || !instance || !surface) {
        return false;
    }

    if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan surface\n";
        return false;
    }

    return true;
}

void Window::setResizeCallback(ResizeCallback callback)
{
    m_resizeCallback = std::move(callback);
}

uint32_t Window::getInstanceExtensionCount() const
{
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    return count;
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    if (width == 0 || height == 0) {
        return;
    }

    Window* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (owner && owner->m_resizeCallback) {
        owner->m_resizeCallback(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Window* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (owner) {
        owner->m_keyboard.onKeyEvent(key, scancode, action, mods);
    }
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Window* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (owner) {
        owner->m_mouse.onButtonEvent(button, action, mods);
    }
}

void Window::cursorPosCallback(GLFWwindow* window, double x, double y)
{
    Window* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (owner) {
        owner->m_mouse.onCursorPosition(x, y);
    }
}

void Window::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    Window* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (owner) {
        owner->m_mouse.onScroll(xOffset, yOffset);
    }
}

} // namespace vkapp
