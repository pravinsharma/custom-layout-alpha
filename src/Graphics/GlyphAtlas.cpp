#include "GlyphAtlas.h"

#include "FontManager.h"

#include <algorithm>
#include <cstring>
#include <iostream>

namespace vkapp::Graphics {

GlyphAtlas::~GlyphAtlas() {
    shutdown();
}

bool GlyphAtlas::initialize(VkDevice device, VkPhysicalDevice physicalDevice,
                            uint32_t queueFamily, VkCommandPool commandPool,
                            VkQueue queue, FontManager* fontManager) {
    m_device = device;
    m_fontManager = fontManager;

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_uploadPool) != VK_SUCCESS) {
        std::cerr << "Failed to create atlas upload command pool\n";
        return false;
    }

    m_pixels.resize(m_width * m_height * 4);
    std::fill(m_pixels.begin(), m_pixels.end(), 0);

    if (!createImage(physicalDevice)) return false;
    if (!createImageView()) return false;
    if (!createSampler()) return false;

    if (!uploadToGpu(device, physicalDevice, m_uploadPool, queue)) {
        std::cerr << "Failed to upload glyph atlas\n";
        return false;
    }

    if (fontManager) {
        FT_Face face = fontManager->getFace(0);
        if (face) {
            FT_Set_Char_Size(face, 0, 16 * 64, 96, 96);
        }
    }

    return true;
}

void GlyphAtlas::shutdown() {
    if (m_sampler != VK_NULL_HANDLE) {
        vkDestroySampler(m_device, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
    if (m_view != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, m_view, nullptr);
        m_view = VK_NULL_HANDLE;
    }
    if (m_memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_memory, nullptr);
        m_memory = VK_NULL_HANDLE;
    }
    if (m_image != VK_NULL_HANDLE) {
        vkDestroyImage(m_device, m_image, nullptr);
        m_image = VK_NULL_HANDLE;
    }
    if (m_uploadPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_uploadPool, nullptr);
        m_uploadPool = VK_NULL_HANDLE;
    }
    m_device = VK_NULL_HANDLE;
    m_pixels.clear();
    m_glyphs.clear();
    m_glyphMap.clear();
}

const Glyph* GlyphAtlas::getGlyph(uint32_t fontId, uint32_t codepoint, FontManager* fontManager) {
    auto key = std::make_pair(fontId, codepoint);
    auto it = m_glyphMap.find(key);
    if (it != m_glyphMap.end()) {
        return &m_glyphs[it->second];
    }

    FT_Face face = fontManager->getFace(fontId);
    if (!face) return nullptr;

    FT_UInt glyphIndex = FT_Get_Char_Index(face, codepoint);
    if (glyphIndex == 0) return nullptr;

    FT_Error err = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    if (err != 0) return nullptr;

    FT_GlyphSlot slot = face->glyph;
    err = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
    if (err != 0) return nullptr;

    FT_Bitmap bitmap = slot->bitmap;

    Glyph glyph{};
    glyph.fontId = fontId;
    glyph.codepoint = codepoint;
    glyph.width = static_cast<float>(bitmap.width);
    glyph.height = static_cast<float>(bitmap.rows);
    glyph.bearingX = static_cast<float>(slot->metrics.horiBearingX) / 64.0f;
    glyph.bearingY = static_cast<float>(slot->metrics.horiBearingY) / 64.0f;
    glyph.advance = static_cast<float>(slot->advance.x) / 64.0f;

    if (glyph.width == 0 || glyph.height == 0) {
        size_t idx = m_glyphs.size();
        m_glyphMap[key] = idx;
        m_glyphs.push_back(glyph);
        return &m_glyphs.back();
    }

    uint32_t w = static_cast<uint32_t>(glyph.width);
    uint32_t h = static_cast<uint32_t>(glyph.height);

    if (m_cursorX + w > m_width) {
        m_cursorX = 0;
        m_cursorY += m_rowHeight;
        m_rowHeight = 0;
    }

    if (m_cursorY + h > m_height) {
        std::cerr << "Glyph atlas full\n";
        return nullptr;
    }

    if (h > m_rowHeight) {
        m_rowHeight = h;
    }

    glyph.u0 = static_cast<float>(m_cursorX) / static_cast<float>(m_width);
    glyph.v0 = static_cast<float>(m_cursorY) / static_cast<float>(m_height);
    glyph.u1 = static_cast<float>(m_cursorX + w) / static_cast<float>(m_width);
    glyph.v1 = static_cast<float>(m_cursorY + h) / static_cast<float>(m_height);

    if (bitmap.buffer) {
        for (uint32_t row = 0; row < bitmap.rows; ++row) {
            if (m_cursorY + row >= m_height) break;
            uint32_t dstOffset = ((m_cursorY + row) * m_width + m_cursorX) * 4;
            const uint8_t* srcRow = bitmap.buffer + row * bitmap.pitch;
            for (uint32_t col = 0; col < bitmap.width; ++col) {
                if (m_cursorX + col >= m_width) break;
                uint8_t alpha = srcRow[col];
                m_pixels[dstOffset + 0] = alpha;
                m_pixels[dstOffset + 1] = alpha;
                m_pixels[dstOffset + 2] = alpha;
                m_pixels[dstOffset + 3] = 255;
                dstOffset += 4;
            }
        }
    }

    size_t idx = m_glyphs.size();
    m_glyphMap[key] = idx;
    m_glyphs.push_back(glyph);
    m_cursorX += w;
    m_needsUpload = true;

    return &m_glyphs.back();
}

bool GlyphAtlas::createImage(VkPhysicalDevice physicalDevice) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(m_device, &imageInfo, nullptr, &m_image) != VK_SUCCESS) {
        std::cerr << "Failed to create glyph atlas image\n";
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            memoryTypeIndex = i;
            break;
        }
    }

    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_memory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate glyph atlas memory\n";
        return false;
    }

    vkBindImageMemory(m_device, m_image, m_memory, 0);
    return true;
}

bool GlyphAtlas::createImageView() {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_view) != VK_SUCCESS) {
        std::cerr << "Failed to create glyph atlas image view\n";
        return false;
    }

    return true;
}

bool GlyphAtlas::createSampler() {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        std::cerr << "Failed to create glyph atlas sampler\n";
        return false;
    }

    return true;
}

bool GlyphAtlas::uploadToGpu(VkDevice device, VkPhysicalDevice physicalDevice,
                             VkCommandPool commandPool, VkQueue queue) {
    (void)commandPool;
    if (!m_needsUpload || m_pixels.empty()) return true;
    m_needsUpload = false;

    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(m_pixels.size());

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
        std::cerr << "Failed to create staging buffer\n";
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
            memoryTypeIndex = i;
            break;
        }
    }

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate staging memory\n";
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        return false;
    }

    vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

    void* data = nullptr;
    vkMapMemory(device, stagingMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_pixels.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingMemory);

    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = m_uploadPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmd) != VK_SUCCESS) {
        std::cerr << "Failed to allocate atlas upload command buffer\n";
        vkFreeMemory(device, stagingMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        return false;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &beginInfo);

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags srcAccess = 0;
    if (m_currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        srcAccess = VK_ACCESS_SHADER_READ_BIT;
    }

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = m_currentLayout;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(cmd, srcStage,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = m_width;
    region.imageExtent.height = m_height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(cmd, stagingBuffer, m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);

    m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkResult submitResult = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    if (submitResult != VK_SUCCESS) {
        std::cerr << "[ATLAS] vkQueueSubmit for upload failed: " << submitResult << "\n";
    }
    VkResult idleResult = vkQueueWaitIdle(queue);
    if (idleResult != VK_SUCCESS) {
        std::cerr << "[ATLAS] vkQueueWaitIdle after upload failed: " << idleResult << "\n";
        if (idleResult == VK_ERROR_DEVICE_LOST) std::abort();
    }

    vkFreeCommandBuffers(device, m_uploadPool, 1, &cmd);
    vkFreeMemory(device, stagingMemory, nullptr);
    vkDestroyBuffer(device, stagingBuffer, nullptr);

    return true;
}

} // namespace vkapp::Graphics
