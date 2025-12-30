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

ComputePipeline::ComputePipeline() {
    auto const id = gComputePipelineStorage.allocate();
    computePipelineID.store(id, std::memory_order_release);
    auto const handle = gComputePipelineStorage.acquire_write(id);
    handle->impl = new ComputePipelineVulkan();
}

ComputePipeline::ComputePipeline(const std::string& shaderCode, EmbeddedShader::ShaderLanguage language, const std::source_location& src) {
    auto const id = gComputePipelineStorage.allocate();
    computePipelineID.store(id, std::memory_order_release);
    auto const handle = gComputePipelineStorage.acquire_write(id);
    handle->impl = new ComputePipelineVulkan(shaderCode, language, src);
}

ComputePipeline::ComputePipeline(const ComputePipeline& other) {
    auto const other_id = other.computePipelineID.load(std::memory_order_acquire);
    computePipelineID.store(other_id, std::memory_order_release);
    if (other_id > 0) {
        auto const write_handle = gComputePipelineStorage.acquire_write(other_id);
        incCompute(write_handle);
    }
}

ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept {
    auto const other_id = other.computePipelineID.load(std::memory_order_acquire);
    computePipelineID.store(other_id, std::memory_order_release);
    other.computePipelineID.store(0, std::memory_order_release);
}

ComputePipeline::~ComputePipeline() {
    auto const self_id = computePipelineID.load(std::memory_order_acquire);
    if (self_id > 0) {
        bool destroy = false;
        if (auto const write_handle = gComputePipelineStorage.acquire_write(self_id); decCompute(write_handle)) {
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
        return *this;
    }
    if (self_id == other_id) {
        return *this;
    }

    if (other_id == 0) {
        bool should_destroy_self = false;
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
        computePipelineID.store(other_id, std::memory_order_release);
        auto const other_handle = gComputePipelineStorage.acquire_write(other_id);
        incCompute(other_handle);
        return *this;
    }

    bool should_destroy_self = false;
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
    auto const self_id = computePipelineID.load(std::memory_order_acquire);
    auto const other_id = other.computePipelineID.load(std::memory_order_acquire);

    if (self_id > 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
            decCompute(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gComputePipelineStorage.deallocate(self_id);
        }
    }
    computePipelineID.store(other_id, std::memory_order_release);
    other.computePipelineID.store(0, std::memory_order_release);
    return *this;
}

ResourceProxy ComputePipeline::operator[](const std::string& resourceName) {
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    auto variant_result = (*handle->impl)[resourceName];
    if (std::holds_alternative<HardwarePushConstant>(variant_result)) {
        return ResourceProxy(std::get<HardwarePushConstant>(variant_result));
    }
    throw std::runtime_error("Unknown resource type in ComputePipeline");
}

ComputePipeline& ComputePipeline::operator()(uint16_t x, uint16_t y, uint16_t z) {
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    (*handle->impl)(x, y, z);
    return *this;
}