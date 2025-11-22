#pragma once

#include "HardwareWrapperVulkan/DisplayVulkan/DisplayManager.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceManager.h"
#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"
#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"
#include "corona/kernel/utils/storage.h"

extern Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap> globalBufferStorages;
extern Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap> globalImageStorages;

struct RasterizerPipelineWrap {
    RasterizerPipelineVulkan* impl = nullptr;
    uint64_t refCount = 0;
};

extern Corona::Kernel::Utils::Storage<RasterizerPipelineWrap> gRasterizerPipelineStorage;

struct ComputePipelineWrap {
    ComputePipelineVulkan* impl = nullptr;  // ����ʵ�ֶ���
    uint64_t refCount = 0;
};

struct DisplayerHardwareWrap {
    void* displaySurface = nullptr;
    std::shared_ptr<DisplayManager> displayManager;
    uint64_t refCount = 0;
};

extern Corona::Kernel::Utils::Storage<DisplayerHardwareWrap> globalDisplayerStorages;

extern Corona::Kernel::Utils::Storage<ComputePipelineWrap> gComputePipelineStorage;

struct ExecutorWrap {
    HardwareExecutorVulkan* impl = nullptr;
    uint64_t refCount = 0;
};

extern Corona::Kernel::Utils::Storage<ExecutorWrap> gExecutorStorage;

struct PushConstantWrap {
    uint8_t* data = nullptr;
    uint64_t size = 0;
    uint64_t refCount = 0;
    bool isSub = false;
};

extern Corona::Kernel::Utils::Storage<PushConstantWrap> globalPushConstantStorages;