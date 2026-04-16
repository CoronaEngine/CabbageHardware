#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"

#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#include <utility>

namespace
{
ComputePipelineVulkan *compute_impl(std::uintptr_t id)
{
    if (id == 0)
    {
        return nullptr;
    }

    auto const handle = gComputePipelineStorage.acquire_read(id);
    return handle->impl;
}
} // namespace

void computePipelineInitFromCompiler(ComputePipelineBase &pipeline,
                                      const EmbeddedShader::ShaderCodeCompiler &compiler,
                                      const std::source_location &src)
{
    std::lock_guard<std::mutex> lock(pipeline.computePipelineMutex);
    pipeline.computePipelineHandle_ = gComputePipelineStorage.allocate_handle();
    auto const handle = gComputePipelineStorage.acquire_write(pipeline.getComputePipelineID());
    handle->impl = new ComputePipelineVulkan(compiler, src);
}

ComputePipelineBase::ComputePipelineBase()
{
    computePipelineHandle_ = gComputePipelineStorage.allocate_handle();
    auto const handle = gComputePipelineStorage.acquire_write(getComputePipelineID());
    handle->impl = new ComputePipelineVulkan();
}

ComputePipelineBase::ComputePipelineBase(const std::string &shaderCode,
                                         EmbeddedShader::ShaderLanguage language,
                                         const std::source_location &src)
{
    computePipelineHandle_ = gComputePipelineStorage.allocate_handle();
    auto const handle = gComputePipelineStorage.acquire_write(getComputePipelineID());
    handle->impl = new ComputePipelineVulkan(shaderCode, language, src);
}

ComputePipelineBase::ComputePipelineBase(const std::vector<uint32_t> &spirV,
                                         const std::source_location &src)
{
    computePipelineHandle_ = gComputePipelineStorage.allocate_handle();
    auto const handle = gComputePipelineStorage.acquire_write(getComputePipelineID());
    handle->impl = new ComputePipelineVulkan(spirV, src);
}

ComputePipelineBase::ComputePipelineBase(const ComputePipelineBase &other)
{
    std::lock_guard<std::mutex> lock(other.computePipelineMutex);
    computePipelineHandle_ = other.computePipelineHandle_;
    autoBindEntries_ = other.autoBindEntries_;
}

ComputePipelineBase::ComputePipelineBase(ComputePipelineBase &&other) noexcept
{
    std::lock_guard<std::mutex> lock(other.computePipelineMutex);
    computePipelineHandle_ = std::move(other.computePipelineHandle_);
    autoBindEntries_ = std::move(other.autoBindEntries_);
}

ComputePipelineBase::~ComputePipelineBase() = default;

ComputePipelineBase &ComputePipelineBase::operator=(const ComputePipelineBase &other)
{
    if (this == &other)
    {
        return *this;
    }

    std::scoped_lock lock(computePipelineMutex, other.computePipelineMutex);
    computePipelineHandle_ = other.computePipelineHandle_;
    autoBindEntries_ = other.autoBindEntries_;
    return *this;
}

ComputePipelineBase &ComputePipelineBase::operator=(ComputePipelineBase &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    std::scoped_lock lock(computePipelineMutex, other.computePipelineMutex);
    computePipelineHandle_ = std::move(other.computePipelineHandle_);
    autoBindEntries_ = std::move(other.autoBindEntries_);
    return *this;
}

void ComputePipelineBase::setPushConstantDirect(uint64_t byteOffset, const void *data, size_t size, int32_t bindType)
{
    if (auto *impl = compute_impl(getComputePipelineID()))
    {
        impl->setPushConstantDirect(byteOffset, data, size, bindType);
    }
}

void ComputePipelineBase::setResourceDirect(uint64_t byteOffset,
                                            uint32_t typeSize,
                                            const HardwareBuffer &buffer,
                                            int32_t bindType)
{
    if (auto *impl = compute_impl(getComputePipelineID()))
    {
        impl->setResourceDirect(byteOffset, typeSize, buffer, bindType);
    }
}

void ComputePipelineBase::setResourceDirect(uint64_t byteOffset,
                                            uint32_t typeSize,
                                            const HardwareImage &image,
                                            int32_t bindType)
{
    if (auto *impl = compute_impl(getComputePipelineID()))
    {
        impl->setResourceDirect(byteOffset, typeSize, image, bindType);
    }
}

ComputePipelineBase &ComputePipelineBase::operator()(uint16_t x, uint16_t y, uint16_t z)
{
    for (const auto &entry : autoBindEntries_)
    {
        if (void *res = *entry.boundResourceRef)
        {
            setResourceDirect(entry.byteOffset, entry.typeSize, *static_cast<HardwareImage *>(res), entry.bindType);
        }
    }

    if (auto *impl = compute_impl(getComputePipelineID()))
    {
        (*impl)(x, y, z);
    }
    return *this;
}
