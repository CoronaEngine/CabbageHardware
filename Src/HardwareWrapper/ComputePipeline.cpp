#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"
#include "CabbageHardware.h"

#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"

HardwareExecutorVulkan *getExecutorImpl(uintptr_t id);

struct ComputePipelineWrap
{
    ComputePipelineVulkan *impl = nullptr; // 具体实现对象
    uint64_t refCount = 0;
};

static Corona::Kernel::Utils::Storage<ComputePipelineWrap> gComputePipelineStorage;

static void incCompute(uintptr_t id)
{
    if (id)
    {
        gComputePipelineStorage.write(id, [](ComputePipelineWrap &w) { ++w.refCount; });
    }
}

static void decCompute(uintptr_t id)
{
    if (!id)
        return;
    bool destroy = false;
    gComputePipelineStorage.write(id, [&](ComputePipelineWrap &w) {
        if (--w.refCount == 0)
        {
            delete w.impl;
            w.impl = nullptr;
            destroy = true;
        }
    });
    if (destroy)
        gComputePipelineStorage.deallocate(id);
}

ComputePipeline::ComputePipeline()
{
    const auto handle = gComputePipelineStorage.allocate([](ComputePipelineWrap &w) { w.impl = new ComputePipelineVulkan(); w.refCount =1; });
    computePipelineID = std::make_shared<uintptr_t>(handle);
}

ComputePipeline::ComputePipeline(std::string shaderCode, EmbeddedShader::ShaderLanguage language, const std::source_location &src)
{
    const auto handle = gComputePipelineStorage.allocate([&](ComputePipelineWrap &w) { w.impl = new ComputePipelineVulkan(shaderCode, language, src); w.refCount =1; });
    computePipelineID = std::make_shared<uintptr_t>(handle);
}

ComputePipeline::ComputePipeline(const ComputePipeline &other)
    : computePipelineID(other.computePipelineID)
{
    incCompute(*computePipelineID);
}

ComputePipeline::~ComputePipeline()
{
    if (computePipelineID)
        decCompute(*computePipelineID);
}

ComputePipeline &ComputePipeline::operator=(const ComputePipeline &other)
{
    if (this != &other)
    {
        incCompute(*other.computePipelineID);
        decCompute(*computePipelineID);
        *computePipelineID = *other.computePipelineID;
    }
    return *this;
}

std::variant<HardwarePushConstant> ComputePipeline::operator[](const std::string &resourceName)
{
    std::variant<HardwarePushConstant> result;
    gComputePipelineStorage.read(*computePipelineID, [&](const ComputePipelineWrap &w) { result = (*w.impl)[resourceName]; });
    return result;
}

ComputePipeline &ComputePipeline::operator()(uint16_t x, uint16_t y, uint16_t z)
{
    gComputePipelineStorage.read(*computePipelineID, [&](const ComputePipelineWrap &w) { (*w.impl)(x, y, z); });
    return *this;
}

// 供执行器内部访问实现
ComputePipelineVulkan *getComputePipelineImpl(uintptr_t id)
{
    ComputePipelineVulkan *ptr = nullptr;
    gComputePipelineStorage.read(id, [&](const ComputePipelineWrap &w) { ptr = w.impl; });
    return ptr;
}
