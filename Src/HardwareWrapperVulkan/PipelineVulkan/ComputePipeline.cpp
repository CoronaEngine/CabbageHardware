#include "ComputePipeline.h"

#include "HardwareWrapperVulkan/HardwareUtils.h"

ComputePipelineVulkan::ComputePipelineVulkan()
{
    executorType = CommandRecordVulkan::ExecutorType::Compute;
}

ComputePipelineVulkan::ComputePipelineVulkan(std::string shaderCode,
                                             EmbeddedShader::ShaderLanguage language,
                                             const std::source_location &sourceLocation)
    : ComputePipelineVulkan()
{
    EmbeddedShader::ShaderCodeCompiler compiler(shaderCode,
                                                EmbeddedShader::ShaderStage::ComputeShader,
                                                language,
                                                EmbeddedShader::CompilerOption(),
                                                sourceLocation);

    this->shaderCode = compiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV);
    this->shaderResource = this->shaderCode.shaderResources;

    const uint32_t pushConstantSize = this->shaderCode.shaderResources.pushConstantSize;
    if (pushConstantSize > 0)
    {
        this->pushConstant = HardwarePushConstant(pushConstantSize, 0);
    }
}

ComputePipelineVulkan::~ComputePipelineVulkan()
{
    VkDevice device = VK_NULL_HANDLE;
    if (const auto mainDevice = globalHardwareContext.getMainDevice())
    {
        device = mainDevice->deviceManager.getLogicalDevice();
    }

    if (device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(device);

        if (pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }

        if (pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }
    }
}

std::variant<HardwarePushConstant> ComputePipelineVulkan::operator[](const std::string &resourceName)
{
    auto *resource = shaderResource.findShaderBindInfo(resourceName);

    if (resource == nullptr)
    {
        throw std::runtime_error("Failed to find resource with name: " + resourceName);
    }

    if (resource->bindType != EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::pushConstantMembers)
    {
        throw std::runtime_error("Resource '" + resourceName + "' is not a push constant member");
    }

    return HardwarePushConstant(resource->typeSize, resource->byteOffset, &pushConstant);
}

ComputePipelineVulkan *ComputePipelineVulkan::operator()(uint16_t x, uint16_t y, uint16_t z)
{
    groupCount = {x, y, z};
    return this;
}

CommandRecordVulkan::RequiredBarriers ComputePipelineVulkan::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    RequiredBarriers requiredBarriers;
    requiredBarriers.memoryBarriers.resize(1);

    auto &barrier = requiredBarriers.memoryBarriers[0];
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

    return requiredBarriers;
}

void ComputePipelineVulkan::createComputePipeline()
{
    const auto mainDevice = globalHardwareContext.getMainDevice();
    if (!mainDevice)
    {
        throw std::runtime_error("No main device available");
    }

    const VkDevice device = mainDevice->deviceManager.getLogicalDevice();

    // 创建着色器模块
    VkShaderModule shaderModule = mainDevice->resourceManager.createShaderModule(shaderCode);

    // 配置计算着色器阶段
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";

    // 配置推送常量
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = shaderCode.shaderResources.pushConstantSize;

    // 获取描述符集布局
    std::vector<VkDescriptorSetLayout> setLayouts;
    setLayouts.reserve(4);
    for (size_t i = 0; i < 4; ++i)
    {
        setLayouts.push_back(mainDevice->resourceManager.bindlessDescriptors[i].descriptorSetLayout);
    }

    // 创建管线布局
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRange.size > 0 ? 1 : 0;
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRange.size > 0 ? &pushConstantRange : nullptr;

    coronaHardwareCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    // 创建计算管线
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = pipelineLayout;

    coronaHardwareCheck(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

    // 清理着色器模块
    vkDestroyShaderModule(device, shaderModule, nullptr);
}

void ComputePipelineVulkan::commitCommand(HardwareExecutorVulkan &hardwareExecutor)
{
    // 延迟创建管线
    if (pipelineLayout == VK_NULL_HANDLE || pipeline == VK_NULL_HANDLE)
    {
        createComputePipeline();
    }

    const VkCommandBuffer commandBuffer = hardwareExecutor.currentRecordQueue->commandBuffer;

    // 绑定管线
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    // 绑定描述符集
    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.reserve(4);
    for (size_t i = 0; i < 4; ++i)
    {
        descriptorSets.push_back(globalHardwareContext.getMainDevice()->resourceManager.bindlessDescriptors[i].descriptorSet);
    }

    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout,
                            0,
                            static_cast<uint32_t>(descriptorSets.size()),
                            descriptorSets.data(),
                            0,
                            nullptr);

    // 推送常量
    if (const void *data = pushConstant.getData(); data != nullptr)
    {
        const uint32_t pushConstantSize = shaderCode.shaderResources.pushConstantSize;
        if (pushConstantSize > 0)
        {
            vkCmdPushConstants(commandBuffer,
                               pipelineLayout,
                               VK_SHADER_STAGE_COMPUTE_BIT,
                               0,
                               pushConstantSize,
                               data);
        }
    }

    // 调度计算任务
    vkCmdDispatch(commandBuffer, groupCount.x, groupCount.y, groupCount.z);
}