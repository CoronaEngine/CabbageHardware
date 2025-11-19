#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"
#include "HardwareWrapperVulkan/PipelineVulkan/RasterizerPipeline.h"

#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"

// 前向声明内部访问函数
ComputePipelineVulkan *getComputePipelineImpl(uintptr_t id);
RasterizerPipelineVulkan *getRasterizerPipelineImpl(uintptr_t id);

struct ExecutorWrap
{
    HardwareExecutorVulkan *impl = nullptr;
    uint64_t refCount = 0;
};

static Corona::Kernel::Utils::Storage<ExecutorWrap> gExecutorStorage;

static void incExec(uintptr_t id)
{
    if (id)
        gExecutorStorage.write(id, [](ExecutorWrap &w) { ++w.refCount; });
}
static void decExec(uintptr_t id)
{
    if (!id)
        return;
    bool destroy = false;
    gExecutorStorage.write(id, [&](ExecutorWrap &w) { if(--w.refCount==0){ delete w.impl; w.impl=nullptr; destroy=true; } });
    if (destroy)
        gExecutorStorage.deallocate(id);
}

HardwareExecutor::HardwareExecutor()
{
    const auto handle = gExecutorStorage.allocate([](ExecutorWrap &w) { w.impl = new HardwareExecutorVulkan(); w.refCount=1; });
    executorID = std::make_shared<uintptr_t>(handle);
}

HardwareExecutor::HardwareExecutor(const HardwareExecutor &other)
    : executorID(other.executorID)
{
    incExec(*executorID);
}

HardwareExecutor::~HardwareExecutor()
{
    if (executorID)
        decExec(*executorID);
}

HardwareExecutor &HardwareExecutor::operator=(const HardwareExecutor &other)
{
    if (this != &other)
    {
        incExec(*other.executorID);
        decExec(*executorID);
        *executorID = *other.executorID;
    }
    return *this;
}

HardwareExecutor &HardwareExecutor::operator<<(ComputePipeline &computePipeline)
{
    gExecutorStorage.read(*executorID, [&](const ExecutorWrap &wrap) {
        if (computePipeline.getComputePipelineID())
        {
            if (auto *impl = getComputePipelineImpl(*computePipeline.getComputePipelineID()))
            {
                wrap.impl->operator<<(static_cast<CommandRecordVulkan *>(impl));
            }
        }
    });
    return *this;
}

HardwareExecutor &HardwareExecutor::operator<<(RasterizerPipeline &rasterizerPipeline)
{
    gExecutorStorage.read(*executorID, [&](const ExecutorWrap &wrap) {
        if (rasterizerPipeline.getRasterizerPipelineID())
        {
            if (auto *impl = getRasterizerPipelineImpl(*rasterizerPipeline.getRasterizerPipelineID()))
            {
                wrap.impl->operator<<(static_cast<CommandRecordVulkan *>(impl));
            }
        }
    });
    return *this;
}

HardwareExecutor &HardwareExecutor::operator<<(HardwareExecutor &other)
{
    return other; // 透传
}

HardwareExecutor &HardwareExecutor::wait(HardwareExecutor &other)
{
    ExecutorWrap otherWrap;
    gExecutorStorage.read(*other.executorID, [&](const ExecutorWrap &w) { otherWrap = w; });
    gExecutorStorage.read(*executorID, [&](const ExecutorWrap &wrap) { if(otherWrap.impl) wrap.impl->wait(*otherWrap.impl); });
    return *this;
}

HardwareExecutor &HardwareExecutor::commit()
{
    gExecutorStorage.read(*executorID, [&](const ExecutorWrap &wrap) { wrap.impl->commit(); });
    return *this;
}

// Helper for other wrappers to access impl safely
HardwareExecutorVulkan *getExecutorImpl(uintptr_t id)
{
    HardwareExecutorVulkan *impl = nullptr;
    gExecutorStorage.read(id, [&](const ExecutorWrap &w) { impl = w.impl; });
    return impl;
}
