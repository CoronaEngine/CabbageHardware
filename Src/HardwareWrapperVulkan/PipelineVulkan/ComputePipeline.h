#pragma once

#include <ktm/ktm.h>

#include "CabbageHardware.h"
#include "Compiler/ShaderCodeCompiler.h"
#include "HardwareWrapperVulkan/HardwareVulkan/DeviceManager.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceManager.h"

struct ComputePipelineVulkan : public CommandRecordVulkan {
   public:
    ComputePipelineVulkan();
    ~ComputePipelineVulkan() override;

    ComputePipelineVulkan(std::string shaderCode, EmbeddedShader::ShaderLanguage language = EmbeddedShader::ShaderLanguage::GLSL, const std::source_location& sourceLocation = std::source_location::current());

    std::variant<HardwarePushConstant> operator[](const std::string& resourceName);

    ComputePipelineVulkan* operator()(uint16_t x, uint16_t y, uint16_t z);

    ExecutorType getExecutorType() override {
        return CommandRecordVulkan::ExecutorType::Compute;
    }

    void commitCommand(HardwareExecutorVulkan& hardwareExecutor) override;
    RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) override;

   private:
    void createComputePipeline();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    HardwarePushConstant pushConstant;
    EmbeddedShader::ShaderCodeModule::ShaderResources shaderResource;
    EmbeddedShader::ShaderCodeModule shaderCode;

    ktm::uvec3 groupCount = {0, 0, 0};
};