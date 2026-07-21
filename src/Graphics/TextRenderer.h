#pragma once

#include "FontManager.h"
#include "GlyphAtlas.h"
#include "TextLayout.h"
#include "RenderCommand.h"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>
#include <string>

namespace vkapp::Graphics {

class TextRenderer {
public:
    TextRenderer() = default;
    ~TextRenderer();

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    TextRenderer(TextRenderer&&) = delete;
    TextRenderer& operator=(TextRenderer&&) = delete;

    bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamily,
                    VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass);
    void shutdown();

    void setExtent(VkExtent2D extent) { m_extent = extent; }
    void processTextCommands(RenderCommandList& commands);
    void render(VkCommandBuffer cmd, const RenderCommandList& commands);

    uint32_t getFontId(const std::string& family, int weight);
    TextLayout layoutText(const std::string& text, uint32_t fontId, float fontSize,
                          float maxWidth, TextAlign align);

private:
    bool createPipeline();
    bool createDescriptorPool();
    bool createVertexBuffer();
    void uploadVertexData();

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    uint32_t m_queueFamily = 0;

    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize m_vertexBufferSize = 0;

    FontManager m_fontManager;
    GlyphAtlas m_atlas;
    TextLayoutEngine m_layoutEngine;

    struct GlyphVertex {
        float x, y;
        float u, v;
    };

    std::vector<GlyphVertex> m_cpuVertices;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkExtent2D m_extent = {0, 0};
};

} // namespace vkapp::Graphics
