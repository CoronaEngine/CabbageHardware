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
    auto const handle = gExecutorStorage.acquire_read(*executorID);
    if (computePipeline.getComputePipelineID()) {
        if (auto* impl = getComputePipelineImpl(*computePipeline.getComputePipelineID())) {
            handle->impl->operator<<(static_cast<CommandRecordVulkan*>(impl));
        }
    }
    return *this;
}

HardwareExecutor& HardwareExecutor::operator<<(RasterizerPipeline& rasterizerPipeline) {
    auto handle = gExecutorStorage.acquire_read(*executorID);
    if (rasterizerPipeline.getRasterizerPipelineID()) {
        if (auto* impl = getRasterizerPipelineImpl(*rasterizerPipeline.getRasterizerPipelineID())) {
            handle->impl->operator<<(static_cast<CommandRecordVulkan*>(impl));
        }
    }
    return *this;
}

HardwareExecutor& HardwareExecutor::operator<<(HardwareExecutor& other) {
    return other;  // ͸��
}

HardwareExecutor& HardwareExecutor::wait(HardwareExecutor& other) {
    // ��ͬһ��������������в��������⾺̬����
    auto const handle = gExecutorStorage.acquire_read(*executorID);
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

// Helper for other wrappers to access impl safely
HardwareExecutorVulkan* getExecutorImpl(uintptr_t id) {
    HardwareExecutorVulkan* impl = nullptr;
    auto handle = gExecutorStorage.acquire_read(id);
    impl = handle->impl;
    return impl;
}
