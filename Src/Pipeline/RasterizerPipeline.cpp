#include "RasterizerPipeline.h"

#include <Hardware/GlobalContext.h>

RasterizerPipeline::RasterizerPipeline(std::string vertexShaderCode, std::string fragmentShaderCode, uint32_t multiviewCount,
                                       EmbeddedShader::ShaderLanguage vertexShaderLanguage, EmbeddedShader::ShaderLanguage fragmentShaderLanguage, const std::source_location &sourceLocation)
{
    EmbeddedShader::ShaderCodeCompiler vertexShaderCompiler(EmbeddedShader::ShaderCodeCompiler(vertexShaderCode, EmbeddedShader::ShaderStage::VertexShader, vertexShaderLanguage, EmbeddedShader::CompilerOption(), sourceLocation));
    EmbeddedShader::ShaderCodeCompiler fragmentShaderCompiler(EmbeddedShader::ShaderCodeCompiler(fragmentShaderCode, EmbeddedShader::ShaderStage::FragmentShader, fragmentShaderLanguage, EmbeddedShader::CompilerOption(), sourceLocation));

    vertShaderCode = vertexShaderCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV);
    fragShaderCode = fragmentShaderCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV);

    vertexResource = vertexShaderCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV).shaderResources;
    fragmentResource = fragmentShaderCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV).shaderResources;

    tempPushConstant = HardwarePushConstant(vertexShaderCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV).shaderResources.pushConstantSize, 0);

    auto vertexResources = vertexShaderCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV).shaderResources;
    auto fragmentResources = fragmentShaderCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV).shaderResources;

    for (auto &[name, bindInfo] : vertexResources.bindInfoPool)
    {
        if (bindInfo.bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::stageInputs)
        {
            vertexStageInputs.push_back(bindInfo);
        }
        if (bindInfo.bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::stageOutputs)
        {
            vertexStageOutputs.push_back(bindInfo);
        }
    }
    for (auto &[name, bindInfo] : fragmentResources.bindInfoPool)
    {
        if (bindInfo.bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::stageInputs)
        {
            fragmentStageInputs.push_back(bindInfo);
        }
        if (bindInfo.bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::stageOutputs)
        {
            fragmentStageOutputs.push_back(bindInfo);
        }
    }
    tempVertexBuffers.resize(vertexStageInputs.size());
    renderTargets.resize(fragmentStageOutputs.size());

    this->multiviewCount = multiviewCount;

    this->pushConstantSize = (std::max)(vertShaderCode.shaderResources.pushConstantSize, fragShaderCode.shaderResources.pushConstantSize);

    if (vertShaderCode.shaderResources.pushConstantSize != fragShaderCode.shaderResources.pushConstantSize && (vertShaderCode.shaderResources.pushConstantSize != 0 && (fragShaderCode.shaderResources.pushConstantSize != 0)))
    {
        throw "shader error";
    }
}

void RasterizerPipeline::createRenderPass(int multiviewCount)
{
    std::vector<VkAttachmentDescription> attachments;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    for (size_t i = 0; i < renderTargets.size(); i++)
    {
        VkAttachmentDescription attachment{};
        attachment.format = imageGlobalPool[*renderTargets[i].imageID].imageFormat;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

        attachments.push_back(attachment);
    }
    attachments.push_back(depthAttachment);

    std::vector<VkAttachmentReference> colorReferences;
    for (uint32_t i = 0; i < renderTargets.size(); i++)
    {
        colorReferences.push_back({i, VK_IMAGE_LAYOUT_GENERAL});
    }

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = (uint32_t)colorReferences.size();
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = (uint32_t)colorReferences.size();
    subpass.pColorAttachments = colorReferences.data();
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    const uint32_t viewMask = (0b1 << multiviewCount) - 1;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCI{};
    renderPassMultiviewCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
    renderPassMultiviewCI.subpassCount = 1;
    renderPassMultiviewCI.pViewMasks = &viewMask;
    renderPassMultiviewCI.correlationMaskCount = 0;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    renderPassInfo.pNext = &renderPassMultiviewCI;

    if (vkCreateRenderPass(globalHardwareContext.mainDevice->deviceManager.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RasterizerPipeline::createGraphicsPipeline(EmbeddedShader::ShaderCodeModule vertShaderCode, EmbeddedShader::ShaderCodeModule fragShaderCode)
{
    auto getVkFormat = [](const std::string &typeName, uint32_t elementCount) -> VkFormat
        {
        VkFormat temp;
        if (typeName == "float")
        {
            switch (elementCount)
            {
            case 1:
                temp = VK_FORMAT_R32_SFLOAT;
                break;
            case 2:
                temp = VK_FORMAT_R32G32_SFLOAT;
                break;
            case 3:
                temp = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case 4:
                temp = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                break;
            }
        }
        if (typeName == "int")
        {
            switch (elementCount)
            {
            case 1:
                temp = VK_FORMAT_R32_SINT;
                break;
            case 2:
                temp = VK_FORMAT_R32G32_SINT;
                break;
            case 3:
                temp = VK_FORMAT_R32G32B32_SINT;
                break;
            case 4:
                temp = VK_FORMAT_R32G32B32A32_SINT;
                break;
            default:
                break;
            }
        }
        if (typeName == "uint")
        {
            switch (elementCount)
            {
            case 1:
                temp = VK_FORMAT_R32_UINT;
                break;
            case 2:
                temp = VK_FORMAT_R32G32_UINT;
                break;
            case 3:
                temp = VK_FORMAT_R32G32B32_UINT;
                break;
            case 4:
                temp = VK_FORMAT_R32G32B32A32_UINT;
                break;
            default:
                break;
            }
        }

        return temp; 
        };
    

    VkShaderModule vertShaderModule = globalHardwareContext.mainDevice->resourceManager.createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = globalHardwareContext.mainDevice->resourceManager.createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // VkVertexInputBindingDescription bindingDescriptions[1] = {};
    // bindingDescriptions[0].binding = 0;
    // bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // bindingDescriptions[0].stride = 0;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions(vertexStageInputs.size());
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(vertexStageInputs.size());
    //for (uint32_t index = 0; index < vertShaderCode.shaderResources.stageInputs.size(); index++)
    for (auto item : vertexStageInputs)
    {
        bindingDescriptions[item.location].binding = item.location;
        bindingDescriptions[item.location].stride = item.typeSize;
        bindingDescriptions[item.location].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        attributeDescriptions[item.location].binding = item.location;
        attributeDescriptions[item.location].location = item.location;
        attributeDescriptions[item.location].format = getVkFormat(item.typeName,item.elementCount);
        attributeDescriptions[item.location].offset = 0;
    }

    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    for (uint32_t i = 0; i < renderTargets.size(); i++)
    {
        colorBlendAttachments.push_back(colorBlendAttachment);
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = (uint32_t)colorBlendAttachments.size();
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPushConstantRange push_constant_ranges = {};
    push_constant_ranges.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_ranges.offset = 0;
    push_constant_ranges.size = pushConstantSize;

    std::vector<VkDescriptorSetLayout> setLayouts;
    for (size_t i = 0; i < 4; i++)
    {
        setLayouts.push_back(globalHardwareContext.mainDevice->resourceManager.bindlessDescriptors[i].descriptorSetLayout);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayouts.size();
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &push_constant_ranges;

    if (vkCreatePipelineLayout(globalHardwareContext.mainDevice->deviceManager.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkResult result = vkCreateGraphicsPipelines(globalHardwareContext.mainDevice->deviceManager.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(globalHardwareContext.mainDevice->deviceManager.logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(globalHardwareContext.mainDevice->deviceManager.logicalDevice, vertShaderModule, nullptr);
}

void RasterizerPipeline::createFramebuffers(ktm::uvec2 imageSize)
{
    std::vector<VkImageView> attachments;
    for (int i = 0; i < renderTargets.size(); i++)
    {
        attachments.push_back(imageGlobalPool[*renderTargets[i].imageID].imageView);
    }
    attachments.push_back(imageGlobalPool[*depthImage.imageID].imageView);

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = imageSize.x;
    framebufferInfo.height = imageSize.y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(globalHardwareContext.mainDevice->deviceManager.logicalDevice, &framebufferInfo, nullptr, &frameBuffers) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

RasterizerPipeline* RasterizerPipeline::operator()(uint16_t imageSizeX, uint16_t imageSizeY)
{
    this->imageSize = {imageSizeX, imageSizeY};
    return this;
}


RasterizerPipeline* RasterizerPipeline::record(const HardwareBuffer &indexBuffer)
{
    return this;
}


void RasterizerPipeline::commitCommand(HardwareExecutor &hardwareExecutor)
{

    if (!depthImage)
    {
        depthImage = HardwareImage(imageSize.x, imageSize.y, ImageFormat::D32_FLOAT, ImageUsage::DepthImage);

        createRenderPass(multiviewCount);

        createGraphicsPipeline(vertShaderCode, fragShaderCode);
        createFramebuffers(imageGlobalPool[*depthImage.imageID].imageSize);
    }

    //auto runCommand = [&](const VkCommandBuffer &commandBuffer) {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffers;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent.width = imageGlobalPool[*depthImage.imageID].imageSize.x;
        renderPassInfo.renderArea.extent.height = imageGlobalPool[*depthImage.imageID].imageSize.y;

        std::vector<VkClearValue> clearValues;
        for (size_t i = 0; i < renderTargets.size(); i++)
        {
            clearValues.push_back(imageGlobalPool[*renderTargets[i].imageID].clearValue);
        }
        clearValues.push_back(imageGlobalPool[*depthImage.imageID].clearValue);

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(hardwareExecutor.currentRecordQueue->commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)imageGlobalPool[*depthImage.imageID].imageSize.x;
        viewport.height = (float)imageGlobalPool[*depthImage.imageID].imageSize.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(hardwareExecutor.currentRecordQueue->commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent.width = imageGlobalPool[*depthImage.imageID].imageSize.x;
        scissor.extent.height = imageGlobalPool[*depthImage.imageID].imageSize.y;
        vkCmdSetScissor(hardwareExecutor.currentRecordQueue->commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(hardwareExecutor.currentRecordQueue->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        //std::vector<VkBuffer> vertexBuffers;
        //std::vector<VkDeviceSize> offsets;
        //for (size_t vertexBufferIndex = 0; vertexBufferIndex < tempVertexBuffers.size(); vertexBufferIndex++)
        //{
        //    vertexBuffers.push_back(bufferGlobalPool[*tempVertexBuffers[vertexBufferIndex].bufferID].bufferHandle);
        //    offsets.push_back(0);
        //}

        //vkCmdBindVertexBuffers(commandBuffer, 0, (uint32_t)tempVertexBuffers.size(), vertexBuffers.data(), offsets.data());

        //vkCmdBindIndexBuffer(commandBuffer, bufferGlobalPool[*indexBuffer.bufferID].bufferHandle, 0 * sizeof(uint32_t), VK_INDEX_TYPE_UINT32);

        //std::vector<VkDescriptorSet> descriptorSets;
        //for (size_t i = 0; i < 4; i++)
        //{
        //    descriptorSets.push_back(globalHardwareContext.mainDevice->resourceManager.bindlessDescriptors[i].descriptorSet);
        //}

        //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), 0, nullptr);

        //void *data = tempPushConstant.getData();
        //vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstantSize, data);

        //uint32_t indexCount = bufferGlobalPool[*indexBuffer.bufferID].bufferAllocInfo.size / sizeof(uint32_t);
        //vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);


        vkCmdEndRenderPass(hardwareExecutor.currentRecordQueue->commandBuffer);
    //};

    //executor << runCommand;
}