#pragma once

#include "FontManager.h"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>
#include <map>
#include <utility>

namespace vkapp::Graphics {

struct Glyph {
    uint32_t fontId = 0;
    uint32_t codepoint = 0;
    float width = 0.0f;
    float height = 0.0f;
    float bearingX = 0.0f;
    float bearingY = 0.0f;
    float advance = 0.0f;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
};

class GlyphAtlas {
public:
    GlyphAtlas() = default;
    ~GlyphAtlas();

    GlyphAtlas(const GlyphAtlas&) = delete;
    GlyphAtlas& operator=(const GlyphAtlas&) = delete;
    GlyphAtlas(GlyphAtlas&&) = delete;
    GlyphAtlas& operator=(GlyphAtlas&&) = delete;

    bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t queueFamily,
                    VkCommandPool commandPool, VkQueue queue, FontManager* fontManager);
    void shutdown();

    const Glyph* getGlyph(uint32_t fontId, uint32_t codepoint, FontManager* fontManager);
    VkImageView getImageView() const { return m_view; }
    VkSampler getSampler() const { return m_sampler; }
    bool uploadToGpu(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue);

private:
    bool createImage(VkPhysicalDevice physicalDevice);
    bool createImageView();
    bool createSampler();
    bool packGlyph(const Glyph& glyph);

    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_uploadPool = VK_NULL_HANDLE;
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkImageView m_view = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    std::vector<uint8_t> m_pixels;
    uint32_t m_width = 2048;
    uint32_t m_height = 2048;
    uint32_t m_cursorX = 0;
    uint32_t m_cursorY = 0;
    uint32_t m_rowHeight = 0;

    std::vector<Glyph> m_glyphs;
    std::map<std::pair<uint32_t, uint32_t>, size_t> m_glyphMap;
    FontManager* m_fontManager = nullptr;
    bool m_needsUpload = false;
    VkImageLayout m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

} // namespace vkapp::Graphics
