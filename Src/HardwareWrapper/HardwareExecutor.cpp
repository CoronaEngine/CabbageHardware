#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"
#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

static void incExec(const Corona::Kernel::Utils::Storage<ExecutorWrap>::WriteHandle& handle) {
    ++handle->refCount;
}

static bool decExec(const Corona::Kernel::Utils::Storage<ExecutorWrap>::WriteHandle& handle) {
    if (--handle->refCount == 0) {
        delete handle->impl;
        handle->impl = nullptr;
        return true;
    }
    return false;
}

HardwareExecutor::HardwareExecutor() : executorID(gExecutorStorage.allocate()) {
    auto handle = gExecutorStorage.acquire_write(executorID.load(std::memory_order_acquire));
    handle->impl = new HardwareExecutorVulkan();
}

HardwareExecutor::HardwareExecutor(const HardwareExecutor& other)
    : executorID(other.executorID.load(std::memory_order_acquire)) {
    if (executorID.load(std::memory_order_acquire) > 0) {
        auto const handle = gExecutorStorage.acquire_write(executorID.load(std::memory_order_acquire));
        incExec(handle);
    }
}

HardwareExecutor::HardwareExecutor(HardwareExecutor&& other) noexcept
    : executorID(other.executorID.load(std::memory_order_acquire)) {
    other.executorID.store(0, std::memory_order_release);
}

HardwareExecutor::~HardwareExecutor() {
    if (auto const self_id = executorID.load(std::memory_order_acquire);
        self_id > 0) {
        bool should_destroy_self = false;
        if (auto const handle = gExecutorStorage.acquire_write(self_id);
            decExec(handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gExecutorStorage.deallocate(self_id);
        }
        executorID.store(0, std::memory_order_release);
    }
}

HardwareExecutor& HardwareExecutor::operator=(const HardwareExecutor& other) {
    if (this == &other) {
        return *this;
    }
    auto const self_id = executorID.load(std::memory_order_acquire);
    auto const other_id = other.executorID.load(std::memory_order_acquire);
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
        if (auto const self_handle = gExecutorStorage.acquire_write(self_id);
            decExec(self_handle)) {
            gExecutorStorage.deallocate(self_id);
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gExecutorStorage.deallocate(self_id);
        }
        executorID.store(0, std::memory_order_acquire);
        return *this;
    }
    if (self_id == 0) {
        // 直接拷贝
        executorID.store(other_id, std::memory_order_release);
        auto const other_handle = gExecutorStorage.acquire_write(other_id);
        incExec(other_handle);
        return *this;
    }
    if (self_id < other_id) {
        auto const self_handle = gExecutorStorage.acquire_write(self_id);
        auto const other_handle = gExecutorStorage.acquire_write(other_id);
        incExec(other_handle);
        if (decExec(self_handle)) {
            should_destroy_self = true;
        }
    } else {
        auto const other_handle = gExecutorStorage.acquire_write(other_id);
        auto const self_handle = gExecutorStorage.acquire_write(self_id);
        incExec(other_handle);
        if (decExec(self_handle)) {
            should_destroy_self = true;
        }
    }
    if (should_destroy_self) {
        gExecutorStorage.deallocate(self_id);
    }
    executorID.store(other_id, std::memory_order_release);
    return *this;
}

HardwareExecutor& HardwareExecutor::operator=(HardwareExecutor&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (auto const self_id = executorID.load(std::memory_order_acquire);
        self_id > 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = gExecutorStorage.acquire_write(self_id);
            decExec(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            gExecutorStorage.deallocate(self_id);
        }
    }
    executorID.store(other.executorID.load(std::memory_order_acquire), std::memory_order_release);
    other.executorID.store(0, std::memory_order_release);
    return *this;
}

HardwareExecutor& HardwareExecutor::operator<<(ComputePipeline& computePipeline) {
    if (auto const executor_handle = gExecutorStorage.acquire_write(executorID.load(std::memory_order_acquire));
        computePipeline.getComputePipelineID()) {
        if (auto const pipeline_handle = gComputePipelineStorage.acquire_read(computePipeline.getComputePipelineID());
            pipeline_handle.valid()) {
            *executor_handle->impl << static_cast<CommandRecordVulkan*>(pipeline_handle->impl);
        }
    }
    return *this;
}

HardwareExecutor& HardwareExecutor::operator<<(RasterizerPipeline& rasterizerPipeline) {
    if (auto const executor_handle = gExecutorStorage.acquire_write(executorID.load(std::memory_order_acquire));
        rasterizerPipeline.getRasterizerPipelineID()) {
        if (auto const raster_handle = gRasterizerPipelineStorage.acquire_read(rasterizerPipeline.getRasterizerPipelineID());
            raster_handle->impl) {
            *executor_handle->impl << static_cast<CommandRecordVulkan*>(raster_handle->impl);
        }
    }
    return *this;
}

HardwareExecutor& HardwareExecutor::operator<<(HardwareExecutor& other) {
    return other;  // ͸��
}

HardwareExecutor& HardwareExecutor::wait(HardwareExecutor& other) {
    // ��ͬһ��������������в��������⾺̬����
    auto const self_id = executorID.load(std::memory_order_acquire);
    auto const other_id = other.executorID.load(std::memory_order_acquire);
    if (self_id == 0 || other_id == 0) {
        CFW_LOG_WARNING("HardwareExecutor@{} or HardwareExecutor@{} is uninitialized, cannot wait.",
                        reinterpret_cast<std::uintptr_t>(this),
                        reinterpret_cast<std::uintptr_t>(&other));
        return *this;
    }
    if (self_id == other_id) {
        CFW_LOG_WARNING("HardwareExecutor@{} cannot wait self.", reinterpret_cast<std::uintptr_t>(this));
        return *this;
    }

    // 按id排序加锁，避免死锁
    if (self_id < other_id) {
        auto const self_handle = gExecutorStorage.acquire_write(self_id);
        auto const other_handle = gExecutorStorage.acquire_read(other_id);
        if (other_handle->impl && self_handle->impl) {
            self_handle->impl->wait(*other_handle->impl);
        }
    } else {
        auto const other_handle = gExecutorStorage.acquire_read(other_id);
        auto const self_handle = gExecutorStorage.acquire_write(self_id);
        if (other_handle->impl && self_handle->impl) {
            self_handle->impl->wait(*other_handle->impl);
        }
    }
    return *this;
}

HardwareExecutor& HardwareExecutor::commit() {
    auto handle = gExecutorStorage.acquire_write(executorID.load(std::memory_order_acquire));
    handle->impl->commit();
    return *this;
}