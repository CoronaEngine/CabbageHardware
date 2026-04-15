#pragma once

#include "HardwareWrapperVulkan/DisplayVulkan/DisplayManager.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceManager.h"
#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"
#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"
#include "HardwareWrapperVulkan/ResourceRegistry.h"

#include <cstdlib>

struct BufferResourceTraits
{
    static void destroy(ResourceManager::BufferHardwareWrap &buffer)
    {
        if (buffer.resourceManager)
        {
            buffer.resourceManager->destroyBuffer(buffer);
        }
    }
};

struct ImageResourceTraits
{
    static void destroy(ResourceManager::ImageHardwareWrap &image)
    {
        if (image.resourceManager)
        {
            image.resourceManager->destroyImage(image);
        }
    }
};

struct SamplerResourceTraits
{
    static void destroy(ResourceManager::SamplerHardwareWrap &sampler)
    {
        if (sampler.resourceManager)
        {
            sampler.resourceManager->destroySampler(sampler);
        }
    }
};

using BufferResourceRegistry = CabbageHardwareInternal::ResourceRegistry<ResourceManager::BufferHardwareWrap,
                                                                         CabbageHardwareInternal::BufferTag,
                                                                         BufferResourceTraits>;
using ImageResourceRegistry = CabbageHardwareInternal::ResourceRegistry<ResourceManager::ImageHardwareWrap,
                                                                        CabbageHardwareInternal::ImageTag,
                                                                        ImageResourceTraits>;
using SamplerResourceRegistry = CabbageHardwareInternal::ResourceRegistry<ResourceManager::SamplerHardwareWrap,
                                                                          CabbageHardwareInternal::SamplerTag,
                                                                          SamplerResourceTraits>;

extern BufferResourceRegistry globalBufferStorages;
extern ImageResourceRegistry globalImageStorages;
extern SamplerResourceRegistry globalSamplerStorages;

struct RasterizerPipelineWrap
{
    RasterizerPipelineVulkan *impl = nullptr;
};

struct RasterizerPipelineResourceTraits
{
    static void destroy(RasterizerPipelineWrap &pipeline)
    {
        delete pipeline.impl;
        pipeline.impl = nullptr;
    }
};

struct ComputePipelineWrap
{
    ComputePipelineVulkan *impl = nullptr;
};

struct ComputePipelineResourceTraits
{
    static void destroy(ComputePipelineWrap &pipeline)
    {
        delete pipeline.impl;
        pipeline.impl = nullptr;
    }
};

struct DisplayerHardwareWrap
{
    void *displaySurface = nullptr;
    std::shared_ptr<DisplayManager> displayManager;
};

struct DisplayerResourceTraits
{
    static void destroy(DisplayerHardwareWrap &displayer)
    {
        displayer.displayManager.reset();
        displayer.displaySurface = nullptr;
    }
};

struct ExecutorWrap
{
    HardwareExecutorVulkan *impl = nullptr;
};

struct ExecutorResourceTraits
{
    static void destroy(ExecutorWrap &executor)
    {
        delete executor.impl;
        executor.impl = nullptr;
    }
};

struct PushConstantWrap
{
    uint8_t *data{nullptr};
    uint64_t size{0};
    bool isSub{false};
    CabbageHardwareInternal::ResourceHandle<CabbageHardwareInternal::PushConstantTag> parentRef{};
};

struct PushConstantResourceTraits
{
    static void destroy(PushConstantWrap &pushConstant)
    {
        if (pushConstant.data != nullptr && !pushConstant.isSub)
        {
            std::free(pushConstant.data);
            pushConstant.data = nullptr;
        }
        pushConstant.parentRef.reset();
    }
};

using RasterizerPipelineRegistry = CabbageHardwareInternal::ResourceRegistry<RasterizerPipelineWrap,
                                                                             CabbageHardwareInternal::RasterizerPipelineTag,
                                                                             RasterizerPipelineResourceTraits>;
using ComputePipelineRegistry = CabbageHardwareInternal::ResourceRegistry<ComputePipelineWrap,
                                                                          CabbageHardwareInternal::ComputePipelineTag,
                                                                          ComputePipelineResourceTraits>;
using DisplayerResourceRegistry = CabbageHardwareInternal::ResourceRegistry<DisplayerHardwareWrap,
                                                                            CabbageHardwareInternal::DisplayerTag,
                                                                            DisplayerResourceTraits>;
using ExecutorResourceRegistry = CabbageHardwareInternal::ResourceRegistry<ExecutorWrap,
                                                                           CabbageHardwareInternal::ExecutorTag,
                                                                           ExecutorResourceTraits>;
using PushConstantResourceRegistry = CabbageHardwareInternal::ResourceRegistry<PushConstantWrap,
                                                                               CabbageHardwareInternal::PushConstantTag,
                                                                               PushConstantResourceTraits>;

extern RasterizerPipelineRegistry gRasterizerPipelineStorage;
extern ComputePipelineRegistry gComputePipelineStorage;
extern DisplayerResourceRegistry globalDisplayerStorages;
extern ExecutorResourceRegistry gExecutorStorage;
extern PushConstantResourceRegistry globalPushConstantStorages;
