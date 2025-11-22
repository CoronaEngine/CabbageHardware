#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"

#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

/**
 * @brief 计算管线引用计数加一
 *
 * @param write_handle
 */
static void incCompute(const Corona::Kernel::Utils::Storage<ComputePipelineWrap>::WriteHandle& write_handle) {
    ++write_handle->refCount;
}

/**
 * @brief 计算管线引用计数减一
 *
 * @param write_handle
 * @return true 如果引用计数为零，需要销毁
 * @return false 如果引用计数不为零，不需要销毁
 */
static bool decCompute(const Corona::Kernel::Utils::Storage<ComputePipelineWrap>::WriteHandle& write_handle) {
    if (--write_handle->refCount == 0) {
        delete write_handle->impl;
        write_handle->impl = nullptr;
        return true;
    }
    return false;
}

ComputePipeline::ComputePipeline() {
    auto const id = gComputePipelineStorage.allocate();
    auto const handle = gComputePipelineStorage.acquire_write(id);
    handle->impl = new ComputePipelineVulkan();
    handle->refCount = 1;
    computePipelineID = std::make_shared<uintptr_t>(id);
}

ComputePipeline::ComputePipeline(const std::string& shaderCode, EmbeddedShader::ShaderLanguage language, const std::source_location& src) {
    auto const id = gComputePipelineStorage.allocate();
    auto const handle = gComputePipelineStorage.acquire_write(id);
    handle->impl = new ComputePipelineVulkan(shaderCode, language, src);
    handle->refCount = 1;
    computePipelineID = std::make_shared<uintptr_t>(id);
}

ComputePipeline::ComputePipeline(const ComputePipeline& other)
    : computePipelineID(other.computePipelineID) {
    auto const write_handle = gComputePipelineStorage.acquire_write(*computePipelineID);
    incCompute(write_handle);
}

ComputePipeline::~ComputePipeline() {
    if (computePipelineID) {
        // NOTE: 不要修改写法，避免死锁
        bool destroy = false;
        if (auto const write_handle = gComputePipelineStorage.acquire_write(*computePipelineID); decCompute(write_handle)) {
            destroy = true;
        }
        if (destroy) {
            gComputePipelineStorage.deallocate(*computePipelineID);
        }
    }
}

ComputePipeline& ComputePipeline::operator=(const ComputePipeline& other) {
    if (this != &other) {
        {
            auto const other_write_handle = gComputePipelineStorage.acquire_write(*other.computePipelineID);
            incCompute(other_write_handle);
        }
        {
            bool destroy = false;
            if (auto const write_handle = gComputePipelineStorage.acquire_write(*computePipelineID); decCompute(write_handle)) {
                destroy = true;
            }
            if (destroy) {
                gComputePipelineStorage.deallocate(*computePipelineID);
            }
        }
        *computePipelineID = *other.computePipelineID;
    }
    return *this;
}

std::variant<HardwarePushConstant> ComputePipeline::operator[](const std::string& resourceName) {
    auto const handle = gComputePipelineStorage.acquire_read(*computePipelineID);
    std::variant<HardwarePushConstant> result = (*handle->impl)[resourceName];
    return result;
}

ComputePipeline& ComputePipeline::operator()(uint16_t x, uint16_t y, uint16_t z) {
    auto const handle = gComputePipelineStorage.acquire_read(*computePipelineID);
    (*handle->impl)(x, y, z);
    return *this;
}

// ��ִ�����ڲ�����ʵ��
ComputePipelineVulkan* getComputePipelineImpl(uintptr_t id) {
    ComputePipelineVulkan* ptr = nullptr;
    auto const handle = gComputePipelineStorage.acquire_read(id);
    ptr = handle->impl;
    return ptr;
}
