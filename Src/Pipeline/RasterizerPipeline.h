#pragma once
#include <vector>

#include "../Hardware/DeviceManager.h"
#include "../Hardware/ResourceManager.h"

#include "Compiler/ShaderCodeCompiler.h"

#include "../CabbageHardware.h"


struct RasterizerPipeline : public CommandRecord
{
    //struct GeomMeshDrawIndexed
    //{
    //    uint32_t indexOffset;
    //    uint32_t indexCount;
    //    ResourceManager::BufferHardwareWrap indexBuffer;
    //    std::vector<ResourceManager::BufferHardwareWrap> vertexBuffers;

    //    HardwarePushConstant pushConstant;
    //};

    RasterizerPipeline()
    {
        executorType = CommandRecord::ExecutorType::Graphics;
    }

    ~RasterizerPipeline();

    RasterizerPipeline(std::string vertexShaderCode, std::string fragmentShaderCode, uint32_t multiviewCount = 1,
                       EmbeddedShader::ShaderLanguage vertexShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL, EmbeddedShader::ShaderLanguage fragmentShaderLanguage = EmbeddedShader::ShaderLanguage::GLSL,
                       const std::source_location &sourceLocation = std::source_location::current());

    void setDepthImage(HardwareImage& depthImage)
    {
        this->depthImage = depthImage;
    }

    HardwareImage &getDepthImage()
    {
        return depthImage;
    }

    std::variant<HardwarePushConstant, HardwareBuffer, HardwareImage> operator[](const std::string &resourceName)
    {
        EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo *vertexResource = this->vertexResource.findShaderBindInfo(resourceName);
        if (vertexResource != nullptr)
        {
            switch (vertexResource->bindType)
            {
            case EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::pushConstantMembers:
                return std::move(HardwarePushConstant(vertexResource->typeSize, vertexResource->byteOffset, &tempPushConstant));
            case EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::stageInputs:
                return tempVertexBuffers[vertexResource->location];
            }
        }
        else
        {
            EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo *fragmentResource = this->fragmentResource.findShaderBindInfo(resourceName);
            switch (fragmentResource->bindType)
            {
            case EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::pushConstantMembers:
                return std::move(HardwarePushConstant(fragmentResource->typeSize, fragmentResource->byteOffset, &tempPushConstant));
            case EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::stageOutputs:
                return renderTargets[fragmentResource->location];
            }
        }
        return std::move(HardwarePushConstant());
    }

    RasterizerPipeline* operator()(uint16_t x, uint16_t y);

    CommandRecord *record(const HardwareBuffer &indexBuffer);

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Graphics;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override;

    RequiredBarriers getRequiredBarriers(HardwareExecutor &hardwareExecutor) override;

  private:

    ktm::uvec2 imageSize = {0, 0};

    void createRenderPass(int multiviewCount = 1);
    void createGraphicsPipeline(EmbeddedShader::ShaderCodeModule vertShaderCode, EmbeddedShader::ShaderCodeModule fragShaderCode);
    void createFramebuffers(ktm::uvec2 imageSize);

    uint32_t pushConstantSize;

    int multiviewCount = 1;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    VkFramebuffer frameBuffers = VK_NULL_HANDLE;

    // std::vector<ResourceManager::ImageHardwareWrap> renderTarget;
    HardwareImage depthImage;

    EmbeddedShader::ShaderCodeModule vertShaderCode;
    EmbeddedShader::ShaderCodeModule fragShaderCode;

    // HardwareImage depthImage;
    std::vector<HardwareImage> renderTargets;

    // EmbeddedShader::ShaderCodeCompiler vertexShaderCompiler;
    // EmbeddedShader::ShaderCodeCompiler fragmentShaderCompiler;

    struct TriangleGeomMesh
    {
        HardwareBuffer indexBuffer;
        std::vector<HardwareBuffer> vertexBuffers;

        HardwarePushConstant pushConstant;
    };
    std::vector<TriangleGeomMesh> geomMeshes;

    CommandRecord dumpCommandRecord;

    HardwarePushConstant tempPushConstant;

    std::vector<HardwareBuffer> tempVertexBuffers;

    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> vertexStageInputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> vertexStageOutputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> fragmentStageInputs;
    std::vector<EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo> fragmentStageOutputs;

    EmbeddedShader::ShaderCodeModule::ShaderResources vertexResource;
    EmbeddedShader::ShaderCodeModule::ShaderResources fragmentResource;
};