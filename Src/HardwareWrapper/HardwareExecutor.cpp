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

HardwareExecutor::HardwareExecutor() {
    auto id = gExecutorStorage.allocate();
    auto handle = gExecutorStorage.acquire_write(id);
    handle->impl = new HardwareExecutorVulkan();
    handle->refCount = 1;
    executorID = std::make_shared<uintptr_t>(id);
}

HardwareExecutor::HardwareExecutor(const HardwareExecutor& other)
    : executorID(other.executorID) {
    if (*executorID > 0) {
        auto const handle = gExecutorStorage.acquire_write(*executorID);
        incExec(handle);
    }
}

HardwareExecutor::~HardwareExecutor() {
    if (executorID && *executorID > 0) {
        bool destroy = false;
        if (auto const handle = gExecutorStorage.acquire_write(*executorID); decExec(handle)) {
            destroy = true;
        }
        if (destroy) {
            gExecutorStorage.deallocate(*executorID);
        }
    }
}

HardwareExecutor& HardwareExecutor::operator=(const HardwareExecutor& other) {
    if (this != &other) {
        {
            auto const handle = gExecutorStorage.acquire_write(*other.executorID);
            incExec(handle);
        }
        {
            if (executorID && *executorID > 0) {
                bool destroy = false;
                if (auto const handle = gExecutorStorage.acquire_write(*executorID); decExec(handle)) {
                    destroy = true;
                }
                if (destroy) {
                    gExecutorStorage.deallocate(*executorID);
                }
            }
        }
        executorID = other.executorID;
    }
    return *this;
}

HardwareExecutor& HardwareExecutor::operator<<(ComputePipeline& computePipeline) {
    if (auto const executor_handle = gExecutorStorage.acquire_read(*executorID);
        computePipeline.getComputePipelineID()) {
        if (auto const pipeline_handle = gComputePipelineStorage.acquire_write(*computePipeline.getComputePipelineID());
            pipeline_handle.valid()) {
            *executor_handle->impl << static_cast<CommandRecordVulkan*>(pipeline_handle->impl);
        }
    }
    return *this;
}

HardwareExecutor& HardwareExecutor::operator<<(RasterizerPipeline& rasterizerPipeline) {
    if (auto const executor_handle = gExecutorStorage.acquire_write(*executorID);
        rasterizerPipeline.getRasterizerPipelineID()) {
        if (auto const raster_handle = gRasterizerPipelineStorage.acquire_read(*rasterizerPipeline.getRasterizerPipelineID());
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
    auto const handle = gExecutorStorage.acquire_write(*executorID);
    auto const other_handle = gExecutorStorage.acquire_read(*other.executorID);
    if (other_handle->impl && handle->impl) {
        handle->impl->wait(*other_handle->impl);
    }
    return *this;
}

HardwareExecutor& HardwareExecutor::commit() {
    auto handle = gExecutorStorage.acquire_read(*executorID);
    handle->impl->commit();
    return *this;
}