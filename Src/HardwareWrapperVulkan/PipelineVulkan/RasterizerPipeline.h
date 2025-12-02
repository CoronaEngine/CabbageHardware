#pragma once

#include <vector>
#include "CabbageHardware.h"
#include "Compiler/ShaderCodeCompiler.h"
#include "HardwareWrapperVulkan/HardwareVulkan/DeviceManager.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceManager.h"

struct RasterizerPipelineVulkan : public CommandRecordVulkan {
   public:
    RasterizerPipelineVulkan();
    ~RasterizerPipelineVulkan() override;

    RasterizerPipelineVulkan(std::string vertexShaderCode,
                             std::string fragmentShaderCode,
                             uint32_t multiviewCount = 1,
                             EmbeddedShader::ShaderLanguage vertexShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                             EmbeddedShader::ShaderLanguage fragmentShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                             const std::source_location& sourceLocation = std::source_location::current());

    void setDepthImage(HardwareImage& depthImage) {
        this->depthImage = depthImage;
    }

    [[nodiscard]] HardwareImage& getDepthImage() {
        return depthImage;
    }

    std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> operator[](const std::string& resourceName);

    RasterizerPipelineVulkan* operator()(uint16_t width, uint16_t height);

    CommandRecordVulkan* record(const HardwareBuffer& indexBuffer, const HardwareBuffer& vertexBuffer);

    ExecutorType getExecutorType() override {
        return CommandRecordVulkan::ExecutorType::Graphics;
    }

    void commitCommand(HardwareExecutorVulkan& hardwareExecutorVulkan) override;
    RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutorVulkan) override;

   private:

    struct TriangleGeomMesh {
        HardwareBuffer indexBuffer;
        HardwareBuffer vertexBuffer;
        VkDeviceSize vertexOffset;

        HardwarePushConstant pushConstant;
    };
    std::vector<TriangleGeomMesh> geomMeshesRecord;

    void createRenderPass(int multiviewCount = 1);
    void createGraphicsPipeline(EmbeddedShader::ShaderCodeModule& vertShaderCode,
                                EmbeddedShader::ShaderCodeModule& fragShaderCode);
    void createFramebuffers(ktm::uvec2 imageSize);

    [[nodiscard]] VkFormat getVkFormatFromType(const std::string& typeName, uint32_t elementCount) const;

    ktm::uvec2 imageSize = {0, 0};
    uint32_t pushConstantSize = 0;
    int multiviewCount = 1;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkFramebuffer frameBuffers = VK_NULL_HANDLE;

    HardwareImage depthImage;
    std::vector<HardwareImage> renderTargets;

    EmbeddedShader::ShaderCodeModule vertShaderCode;
    EmbeddedShader::ShaderCodeModule fragShaderCode;

    CommandRecordVulkan dumpCommandRecordVulkan;
    HardwarePushConstant tempPushConstant;
    //std::vector<HardwareBuffer> tempVertexBuffers;

    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> vertexStageInputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> vertexStageOutputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> fragmentStageInputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> fragmentStageOutputs;

    EmbeddedShader::ShaderCodeModule::ShaderResources vertexResource;
    EmbeddedShader::ShaderCodeModule::ShaderResources fragmentResource;
};