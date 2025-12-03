#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"

#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

static void incCompute(const Corona::Kernel::Utils::Storage<ComputePipelineWrap>::WriteHandle& write_handle) {
    ++write_handle->refCount;
}

static bool decCompute(const Corona::Kernel::Utils::Storage<ComputePipelineWrap>::WriteHandle& write_handle) {
    if (--write_handle->refCount == 0) {
        delete write_handle->impl;
        write_handle->impl = nullptr;
        return true;
    }
    return false;
}

ComputePipeline::ComputePipeline()
    : computePipelineID(gComputePipelineStorage.allocate()) {
    auto const self_handle = gComputePipelineStorage.acquire_write(computePipelineID.load(std::memory_order_acquire));
    self_handle->impl = new ComputePipelineVulkan();
}

ComputePipeline::ComputePipeline(const std::string& shaderCode, EmbeddedShader::ShaderLanguage language, const std::source_location& src)
    : computePipelineID(gComputePipelineStorage.allocate()) {
    auto const handle = gComputePipelineStorage.acquire_write(computePipelineID.load(std::memory_order_acquire));
    handle->impl = new ComputePipelineVulkan(shaderCode, language, src);
}

ComputePipeline::ComputePipeline(const ComputePipeline& other)
    : computePipelineID(other.computePipelineID.load(std::memory_order_acquire)) {
    if (computePipelineID.load(std::memory_order_acquire) > 0) {
        auto const write_handle = gComputePipelineStorage.acquire_write(computePipelineID.load(std::memory_order_acquire));
        incCompute(write_handle);
    }
}

ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept
    : computePipelineID(other.computePipelineID.load(std::memory_order_acquire)) {
    other.computePipelineID.store(0, std::memory_order_release);
}

ComputePipeline::~ComputePipeline() {
    if (auto const self_id = computePipelineID.load(std::memory_order_acquire);
        self_id > 0) {
        // NOTE: 不要修改写法，避免死锁
        bool destroy = false;
        if (auto const write_handle = gComputePipelineStorage.acquire_write(self_id);
            decCompute(write_handle)) {
            destroy = true;
        }
        if (destroy) {
            gComputePipelineStorage.deallocate(self_id);
        }
        computePipelineID.store(0, std::memory_order_release);
    }
}

ComputePipeline& ComputePipeline::operator=(const ComputePipeline& other) {
    if (this == &other) {
        return *this;
    }
    auto const self_id = computePipelineID.load(std::memory_order_acquire);
    auto const other_id = other.computePipelineID.load(std::memory_order_acquire);
    if (self_id == 0 && other_id == 0) {
        // 都未初始化，直接返回
        return *this;
    }
    if (self_id == other_id) {
        // 已经指向同一个资源，无需操作
        return *this;
    }
    bool should_destroy_self = false;
    if (other_id == 0) {
        // 释放自身资源
        if (auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
            decCompute(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gComputePipelineStorage.deallocate(self_id);
        }
        computePipelineID.store(0, std::memory_order_release);
        return *this;
    }
    if (self_id == 0) {
        // 直接拷贝
        computePipelineID.store(other_id, std::memory_order_release);
        auto const other_handle = gComputePipelineStorage.acquire_write(other_id);
        incCompute(other_handle);
        return *this;
    }
    if (self_id < other_id) {
        auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
        auto const other_handle = gComputePipelineStorage.acquire_write(other_id);
        incCompute(other_handle);
        if (decCompute(self_handle)) {
            should_destroy_self = true;
        }
    } else {
        auto const other_handle = gComputePipelineStorage.acquire_write(other_id);
        auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
        incCompute(other_handle);
        if (decCompute(self_handle)) {
            should_destroy_self = true;
        }
    }
    if (should_destroy_self) {
        gComputePipelineStorage.deallocate(self_id);
    }
    computePipelineID.store(other_id, std::memory_order_release);
    return *this;
}

ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (auto const self_id = computePipelineID.load(std::memory_order_acquire);
        self_id > 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
            decCompute(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gComputePipelineStorage.deallocate(self_id);
        }
    }
    other.computePipelineID.store(0, std::memory_order_release);
    computePipelineID.store(other.computePipelineID.load(std::memory_order_acquire), std::memory_order_release);
    return *this;
}

ResourceProxy ComputePipeline::operator[](const std::string& resourceName) {
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    return (*handle->impl)[resourceName];
}

ComputePipeline& ComputePipeline::operator()(uint16_t x, uint16_t y, uint16_t z) {
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    (*handle->impl)(x, y, z);
    return *this;
}