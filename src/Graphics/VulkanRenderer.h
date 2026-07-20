#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include "Graphics/RenderCommand.h"

#include <vector>
#include <array>

namespace vkapp::Graphics {

class VulkanRenderer {
public:
    VulkanRenderer(GLFWwindow* window);
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    VulkanRenderer(VulkanRenderer&&) = delete;
    VulkanRenderer& operator=(VulkanRenderer&&) = delete;

    bool initialize();
    void shutdown();
    void render(const RenderCommandList& commands);
    void waitIdle() const;

    VkExtent2D getExtent() const { return m_extent; }
    bool needsResize() const { return m_needsResize; }
    void resetResize() { m_needsResize = false; }

private:
    bool createInstance();
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createSurface();
    bool createSwapchain();
    bool createRenderPass();
    bool createPipeline();
    bool createFramebuffers();
    bool createCommandPool();
    bool allocateCommandBuffers();
    bool createSyncObjects();
    bool createVertexBuffer();

    void cleanupSwapchain();
    bool recreateSwapchain();

    GLFWwindow* m_window;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    uint32_t m_graphicsQueueFamily;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_swapchainImages;
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_extent;
    std::vector<VkImageView> m_swapchainImageViews;
    VkRenderPass m_renderPass;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipelineLayout;
    std::vector<VkFramebuffer> m_framebuffers;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    size_t m_currentFrame;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    bool m_needsResize;
    uint32_t m_resizeThrottle;
    VkExtent2D m_pendingExtent;
};

} // namespace vkapp::Graphics
