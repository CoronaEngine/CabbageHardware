#include"ComputePipeline.h"

#include<Hardware/GlobalContext.h>

//using namespace EmbeddedShader;

ComputePipeline::ComputePipeline(std::string shaderCode, EmbeddedShader::ShaderLanguage language, const std::source_location &sourceLocation)
{
    EmbeddedShader::ShaderCodeCompiler shaderCodeCompiler(EmbeddedShader::ShaderCodeCompiler(shaderCode, EmbeddedShader::ShaderStage::ComputeShader, language, EmbeddedShader::CompilerOption(), sourceLocation));

    this->shaderCode = shaderCodeCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV);
    this->pushConstant = HardwarePushConstant(shaderCodeCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV).shaderResources.pushConstantSize, 0);

	shaderResource = shaderCodeCompiler.getShaderCode(EmbeddedShader::ShaderLanguage::SpirV).shaderResources;
}

ComputePipeline::~ComputePipeline()
{
    VkDevice device = VK_NULL_HANDLE;
    if (globalHardwareContext.mainDevice)
    {
        device = globalHardwareContext.mainDevice->deviceManager.logicalDevice;
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

ComputePipeline* ComputePipeline::operator()(uint16_t groupCountX, uint16_t groupCountY, uint16_t groupCountZ)
{
    groupCount = {groupCountX, groupCountY, groupCountZ};
	return this;
}


CommandRecord::RequiredBarriers ComputePipeline::getRequiredBarriers(HardwareExecutor &hardwareExecutor)
{
    CommandRecord::RequiredBarriers requiredBarriers;
    requiredBarriers.memoryBarriers.resize(1);

    requiredBarriers.memoryBarriers[0].sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    requiredBarriers.memoryBarriers[0].srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    requiredBarriers.memoryBarriers[0].srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    requiredBarriers.memoryBarriers[0].dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    requiredBarriers.memoryBarriers[0].dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    requiredBarriers.memoryBarriers[0].pNext = nullptr;

    return requiredBarriers;
}


void ComputePipeline::commitCommand(HardwareExecutor &hardwareExecutor)
{
    if (pipelineLayout == VK_NULL_HANDLE && pipeline == VK_NULL_HANDLE)
    {
        // EmbededShaderCodeGenerator::addPreprocess("layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;");

        // std::string shaderFile = EmbededShader::runDslParse(shaderFunc);
        // std::vector<unsigned int> mSPIRVCode = PipelineBasics::RealtimeShaderCompiler(shaderFile);
        // std::vector<unsigned int> mSPIRVCode = EmbededShaderCodeGenerator::runPipelineParse(shaderFunc, fileName, lineIndex);

        VkComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

        VkShaderModule shaderModule = globalHardwareContext.mainDevice->resourceManager.createShaderModule(shaderCode);

        computePipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        // computePipelineCreateInfo.stage.flags = VK_SHADER_STAGE_COMPUTE_BIT;
        computePipelineCreateInfo.stage.module = shaderModule;
        computePipelineCreateInfo.stage.pName = "main";
        computePipelineCreateInfo.stage.flags = 0;
        computePipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;

        VkPushConstantRange pushConstant{};
        pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstant.offset = 0;
        // pushConstant.size = EmbededShader::testAst.getFinalPushConstSize();
        pushConstant.size = shaderCode.shaderResources.pushConstantSize;

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
        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

        if (vkCreatePipelineLayout(globalHardwareContext.mainDevice->deviceManager.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        computePipelineCreateInfo.layout = pipelineLayout;

        VkResult result = vkCreateComputePipelines(globalHardwareContext.mainDevice->deviceManager.logicalDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create compute pipeline!");
        }

        vkDestroyShaderModule(globalHardwareContext.mainDevice->deviceManager.logicalDevice, shaderModule, nullptr);
    }

    if (pipelineLayout != VK_NULL_HANDLE && pipeline != VK_NULL_HANDLE)
    {
        // 在绑定/调度前加入一次通用内存屏障，确保此前写入对计算着色器可见
        //{
        //    VkMemoryBarrier barrier{};
        //    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        //    barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
        //    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        //    vkCmdPipelineBarrier(
        //        hardwareExecutor.currentRecordQueue->commandBuffer,
        //        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        //        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        //        0,
        //        1, &barrier,
        //        0, nullptr,
        //        0, nullptr);
        //}

        //auto runCommand = [&](const VkCommandBuffer &commandBuffer) {
        vkCmdBindPipeline(hardwareExecutor.currentRecordQueue->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

            std::vector<VkDescriptorSet> descriptorSets;
            for (size_t i = 0; i < 4; i++)
            {
                descriptorSets.push_back(globalHardwareContext.mainDevice->resourceManager.bindlessDescriptors[i].descriptorSet);
            }

            vkCmdBindDescriptorSets(hardwareExecutor.currentRecordQueue->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, (uint32_t)descriptorSets.size(), descriptorSets.data(), 0, nullptr);

            // void* pushContastValue = EmbededShader::testAst.getFinalPushConstBytes();
            void *data = pushConstant.getData();
            if (data != nullptr)
            {
                vkCmdPushConstants(hardwareExecutor.currentRecordQueue->commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, shaderCode.shaderResources.pushConstantSize, data);
            }

            vkCmdDispatch(hardwareExecutor.currentRecordQueue->commandBuffer, groupCount.x, groupCount.y, groupCount.z);
        ////};

        //executor << runCommand;
    }
}