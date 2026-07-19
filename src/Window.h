#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>
#include <string>

namespace vkapp {

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

    bool initialize();
    void shutdown();

    GLFWwindow* getHandle() const noexcept { return m_window; }

    bool shouldClose() const;
    void pollEvents() const;
    void getFramebufferSize(uint32_t& width, uint32_t& height) const;

    bool createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) const;

    void setResizeCallback(ResizeCallback callback);
    uint32_t getInstanceExtensionCount() const;
    bool isInitialized() const noexcept { return m_initialized; }
    const Config& getConfig() const noexcept { return m_config; }

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static bool s_glfwInitialized;

    Config m_config;
    GLFWwindow* m_window = nullptr;
    ResizeCallback m_resizeCallback;
    uint32_t m_instanceExtensionCount = 0;
    bool m_initialized = false;
};

} // namespace vkapp
