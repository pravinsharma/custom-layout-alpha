#include "TextRenderer.h"

#include "FontManager.h"
#include "GlyphAtlas.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>

namespace vkapp::Graphics {

namespace {

constexpr uint32_t MAX_TEXT_VERTICES = 65536;

} // namespace

TextRenderer::~TextRenderer() {
    shutdown();
}

bool TextRenderer::initialize(VkDevice device, VkPhysicalDevice physicalDevice,
                               uint32_t queueFamily, VkCommandPool commandPool,
                               VkQueue queue, VkRenderPass renderPass) {
    m_device = device;
    m_physicalDevice = physicalDevice;
    m_queueFamily = queueFamily;
    m_commandPool = commandPool;
    m_queue = queue;
    m_renderPass = renderPass;

    if (!m_fontManager.initialize()) {
        std::cerr << "Failed to initialize font manager\n";
        return false;
    }

    uint32_t defaultFont = m_fontManager.loadFont("Roboto", 400);
    if (defaultFont == UINT32_MAX) {
        std::cerr << "Failed to load default font\n";
    }

    if (!m_atlas.initialize(device, physicalDevice, queueFamily, commandPool, queue, &m_fontManager)) {
        std::cerr << "Failed to initialize glyph atlas\n";
        return false;
    }

    if (!createPipeline()) return false;

    if (!createDescriptorPool()) return false;

    {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_atlas.getImageView();
        imageInfo.sampler = m_atlas.getSampler();

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
    }

    if (!createVertexBuffer()) return false;

    return true;
}

void TextRenderer::shutdown() {
    if (m_vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        m_vertexBuffer = VK_NULL_HANDLE;
    }
    if (m_vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
        m_vertexBufferMemory = VK_NULL_HANDLE;
    }
    if (m_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }
    if (m_descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
        m_descriptorSetLayout = VK_NULL_HANDLE;
    }
    if (m_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    m_atlas.shutdown();
    m_fontManager.shutdown();
    m_device = VK_NULL_HANDLE;
    m_cpuVertices.clear();
}

uint32_t TextRenderer::getFontId(const std::string& family, int weight) {
    return m_fontManager.getFontId(family, weight);
}

TextLayout TextRenderer::layoutText(const std::string& text, uint32_t fontId, float fontSize,
                                     float maxWidth, TextAlign align) {
    return m_layoutEngine.layout(text, fontId, fontSize, maxWidth, align, &m_fontManager, &m_atlas);
}

void TextRenderer::processTextCommands(RenderCommandList& commands) {
    m_cpuVertices.clear();
    uint32_t vertexOffset = 0;

    for (auto& cmd : commands) {
        if (cmd.type != RenderCommand::Type::Text) continue;

        if (cmd.textContent.empty()) {
            cmd.firstGlyph = 0;
            cmd.glyphCount = 0;
            continue;
        }

        if (cmd.fontId == UINT32_MAX && !cmd.fontFamily.empty()) {
            cmd.fontId = m_fontManager.getFontId(cmd.fontFamily, cmd.fontWeight);
        }

        TextAlign align = TextAlign::Left;
        if (cmd.textAlign == 1) align = TextAlign::Center;
        else if (cmd.textAlign == 2) align = TextAlign::Right;

        TextLayout layout = m_layoutEngine.layout(
            cmd.textContent,
            cmd.fontId,
            cmd.fontSize,
            cmd.rect.width,
            align,
            &m_fontManager,
            &m_atlas
        );

        float originX = std::floor(cmd.rect.x);
        float originY = std::floor(cmd.rect.y);

        for (const auto& g : layout.glyphs) {
            float x0 = originX + g.x;
            float y0 = originY + g.y;
            float x1 = x0 + g.width;
            float y1 = y0 + g.height;

            m_cpuVertices.push_back({x0, y0, g.u0, g.v0});
            m_cpuVertices.push_back({x1, y0, g.u1, g.v0});
            m_cpuVertices.push_back({x1, y1, g.u1, g.v1});
            m_cpuVertices.push_back({x0, y0, g.u0, g.v0});
            m_cpuVertices.push_back({x1, y1, g.u1, g.v1});
            m_cpuVertices.push_back({x0, y1, g.u0, g.v1});
        }

        cmd.firstGlyph = vertexOffset;
        cmd.glyphCount = static_cast<uint32_t>(layout.glyphs.size() * 6);
        vertexOffset += cmd.glyphCount;
    }

    if (m_atlas.getImageView() != VK_NULL_HANDLE) {
        m_atlas.uploadToGpu(m_device, m_physicalDevice, m_commandPool, m_queue);
    }

    uploadVertexData();
}

void TextRenderer::render(VkCommandBuffer cmd, const RenderCommandList& commands) {
    if (m_cpuVertices.empty() || m_pipeline == VK_NULL_HANDLE) return;

    VkDescriptorSet descriptorSets[] = {m_descriptorSet};
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout, 0, 1, descriptorSets, 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &m_vertexBuffer, offsets);

    for (const auto& command : commands) {
        if (command.type != RenderCommand::Type::Text || command.glyphCount == 0) continue;

        float pushData[6] = {
            static_cast<float>(m_extent.width),
            static_cast<float>(m_extent.height),
            command.color.r,
            command.color.g,
            command.color.b,
            command.color.a
        };

        vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushData), pushData);
        vkCmdDraw(cmd, command.glyphCount, 1, command.firstGlyph, 0);
    }
}

bool TextRenderer::createPipeline() {
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float) * 6;

    VkDescriptorSetLayoutBinding samplerBinding = {};
    samplerBinding.binding = 0;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo setLayoutInfo = {};
    setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setLayoutInfo.bindingCount = 1;
    setLayoutInfo.pBindings = &samplerBinding;

    if (vkCreateDescriptorSetLayout(m_device, &setLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create text descriptor set layout\n";
        return false;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        std::cerr << "Failed to create text pipeline layout\n";
        return false;
    }

    VkShaderModuleCreateInfo vertShaderInfo = {};
    vertShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    std::string vertPath = std::string(SHADER_BINARY_DIR) + "/text.vert.spv";
    std::ifstream vertFile(vertPath, std::ios::binary);
    if (!vertFile) {
        std::cerr << "Failed to load text vertex shader: " << vertPath << "\n";
        return false;
    }
    vertFile.seekg(0, std::ios::end);
    size_t vertSize = vertFile.tellg();
    vertFile.seekg(0, std::ios::beg);
    std::vector<char> vertCode(vertSize);
    vertFile.read(vertCode.data(), vertSize);

    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    vertShaderInfo.codeSize = vertSize;
    vertShaderInfo.pCode = reinterpret_cast<const uint32_t*>(vertCode.data());
    if (vkCreateShaderModule(m_device, &vertShaderInfo, nullptr, &vertShaderModule) != VK_SUCCESS) {
        std::cerr << "Failed to create text vertex shader module\n";
        return false;
    }

    std::string fragPath = std::string(SHADER_BINARY_DIR) + "/text.frag.spv";
    std::ifstream fragFile(fragPath, std::ios::binary);
    if (!fragFile) {
        std::cerr << "Failed to load text fragment shader: " << fragPath << "\n";
        vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
        return false;
    }
    fragFile.seekg(0, std::ios::end);
    size_t fragSize = fragFile.tellg();
    fragFile.seekg(0, std::ios::beg);
    std::vector<char> fragCode(fragSize);
    fragFile.read(fragCode.data(), fragSize);

    VkShaderModule fragShaderModule = VK_NULL_HANDLE;
    VkShaderModuleCreateInfo fragShaderInfo = {};
    fragShaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderInfo.codeSize = fragSize;
    fragShaderInfo.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());
    if (vkCreateShaderModule(m_device, &fragShaderInfo, nullptr, &fragShaderModule) != VK_SUCCESS) {
        std::cerr << "Failed to create text fragment shader module\n";
        vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
        return false;
    }

    VkPipelineShaderStageCreateInfo vertStageInfo = {};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = vertShaderModule;
    vertStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragStageInfo = {};
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = fragShaderModule;
    fragStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertStageInfo, fragStageInfo};

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(GlyphVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(GlyphVertex, x);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(GlyphVertex, u);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
        std::cerr << "Failed to create text graphics pipeline\n";
        vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
        vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
        return false;
    }

    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);

    return true;
}

bool TextRenderer::createDescriptorPool() {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    VkResult result = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create text descriptor pool\n";
        return false;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_descriptorSetLayout;

    VkResult allocResult = vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet);
    if (allocResult != VK_SUCCESS) {
        std::cerr << "Failed to allocate text descriptor set\n";
        return false;
    }

    return true;
}

bool TextRenderer::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(GlyphVertex) * MAX_TEXT_VERTICES;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS) {
        std::cerr << "Failed to create text vertex buffer\n";
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

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

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_vertexBufferMemory) != VK_SUCCESS) {
        std::cerr << "Failed to allocate text vertex buffer memory\n";
        return false;
    }

    vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexBufferMemory, 0);
    m_vertexBufferSize = bufferSize;

    return true;
}

void TextRenderer::uploadVertexData() {
    if (m_cpuVertices.empty() || m_vertexBuffer == VK_NULL_HANDLE) return;

    VkDeviceSize bufferSize = sizeof(GlyphVertex) * m_cpuVertices.size();
    if (bufferSize > m_vertexBufferSize) {
        std::cerr << "Text vertex buffer overflow\n";
        return;
    }

    void* data = nullptr;
    vkMapMemory(m_device, m_vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_cpuVertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device, m_vertexBufferMemory);
}

} // namespace vkapp::Graphics
