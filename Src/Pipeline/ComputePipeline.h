#pragma once

#include <ktm/ktm.h>

#include"../Hardware/DeviceManager.h"
#include"../Hardware/ResourceManager.h"

#include"Compiler/ShaderCodeCompiler.h"

#include"../CabbageDisplayer.h"


struct ComputePipeline
{
    ComputePipeline()
    {

    }

    ~ComputePipeline()
    {

    }

	
    ComputePipeline(std::string shaderCode, EmbeddedShader::ShaderLanguage language = EmbeddedShader::ShaderLanguage::GLSL, const std::source_location &sourceLocation = std::source_location::current());

	
    std::variant<HardwarePushConstant> operator[](const std::string& resourceName)
    {
        EmbeddedShader::ShaderCodeModule::ShaderResources::ShaderBindInfo *resource = shaderResource.findShaderBindInfo(resourceName);
        if (resource != nullptr && resource->bindType == EmbeddedShader::ShaderCodeModule::ShaderResources::BindType::pushConstantMembers)
        {
            return std::move(HardwarePushConstant(resource->typeSize, resource->byteOffset, &pushConstant));
        }
        else
        {
            throw std::runtime_error("failed to find with name!" + resourceName);
        }
    }

    ComputePipeline &operator()(uint16_t x, uint16_t y, uint16_t z);

    
    HardwareExecutor* executor;


  private:
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;

	HardwarePushConstant pushConstant;
    //HardwarePushConstant tempPushConstantMember;
    EmbeddedShader::ShaderCodeModule::ShaderResources shaderResource;


    //EmbeddedShader::ShaderCodeCompiler shaderCodeCompiler;
    EmbeddedShader::ShaderCodeModule shaderCode;

};

inline HardwareExecutor &operator<<(HardwareExecutor &executor, ComputePipeline &other)
{
    other.executor = &executor;
    return executor;
}
