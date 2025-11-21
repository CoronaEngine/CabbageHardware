#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"

#include "corona/kernel/utils/storage.h"
#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"

#include "HardwareWrapperVulkan/ResourcePool.h"

HardwareExecutorVulkan* getExecutorImpl(uintptr_t id);


static void incCompute(uintptr_t id) {
    if (id) {
        ++gComputePipelineStorage.acquire_write(id)->refCount;
    }
}

static void decCompute(uintptr_t id) {
    if (!id) {
        return;
    }
    bool destroy = false;
    auto handle = gComputePipelineStorage.acquire_write(id);
    if (--handle->refCount == 0) {
        delete handle->impl;
        handle->impl = nullptr;
        destroy = true;
    }
    if (destroy) {
        gComputePipelineStorage.deallocate(id);
    }
}

ComputePipeline::ComputePipeline() {
    auto const id = gComputePipelineStorage.allocate();
    auto handle = gComputePipelineStorage.acquire_write(id);
    handle->impl = new ComputePipelineVulkan();
    handle->refCount = 1;
    computePipelineID = std::make_shared<uintptr_t>(id);
}

ComputePipeline::ComputePipeline(std::string shaderCode, EmbeddedShader::ShaderLanguage language, const std::source_location& src) {
    auto const id = gComputePipelineStorage.allocate();
    auto handle = gComputePipelineStorage.acquire_write(id);
    handle->impl = new ComputePipelineVulkan(shaderCode, language, src);
    handle->refCount = 1;
    computePipelineID = std::make_shared<uintptr_t>(id);
}

ComputePipeline::ComputePipeline(const ComputePipeline& other)
    : computePipelineID(other.computePipelineID) {
    incCompute(*computePipelineID);
}

ComputePipeline::~ComputePipeline() {
    if (computePipelineID) {
        decCompute(*computePipelineID);
    }
}

ComputePipeline& ComputePipeline::operator=(const ComputePipeline& other) {
    if (this != &other) {
        incCompute(*other.computePipelineID);
        decCompute(*computePipelineID);
        *computePipelineID = *other.computePipelineID;
    }
    return *this;
}

std::variant<HardwarePushConstant> ComputePipeline::operator[](const std::string& resourceName) {
    std::variant<HardwarePushConstant> result;
    auto handle = gComputePipelineStorage.acquire_read(*computePipelineID);
    result = (*handle->impl)[resourceName];
    return result;
}

ComputePipeline& ComputePipeline::operator()(uint16_t x, uint16_t y, uint16_t z) {
    auto handle = gComputePipelineStorage.acquire_read(*computePipelineID);
    (*handle->impl)(x, y, z);
    return *this;
}

// ��ִ�����ڲ�����ʵ��
ComputePipelineVulkan* getComputePipelineImpl(uintptr_t id) {
    ComputePipelineVulkan* ptr = nullptr;
    auto handle = gComputePipelineStorage.acquire_read(id);
    ptr = handle->impl;
    return ptr;
}
