#include "CabbageHardware.h"

#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

HardwareSampler::HardwareSampler() = default;

HardwareSampler::HardwareSampler(const SamplerDesc &desc)
{
    samplerHandle_ = globalSamplerStorages.allocate_handle(desc.debugName);
    auto const sampler_id = getSamplerID();
    auto handle = globalSamplerStorages.acquire_write(sampler_id);
    *handle = globalHardwareContext.getMainDevice()->resourceManager.createSampler(desc);
}

HardwareSampler::HardwareSampler(const HardwareSampler &other)
    : samplerHandle_(other.samplerHandle_)
{
}

HardwareSampler::HardwareSampler(HardwareSampler &&other) noexcept
    : samplerHandle_(std::move(other.samplerHandle_))
{
}

HardwareSampler::~HardwareSampler() = default;

HardwareSampler &HardwareSampler::operator=(const HardwareSampler &other)
{
    if (this == &other)
    {
        return *this;
    }
    samplerHandle_ = other.samplerHandle_;
    return *this;
}

HardwareSampler &HardwareSampler::operator=(HardwareSampler &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    samplerHandle_ = std::move(other.samplerHandle_);
    return *this;
}

HardwareSampler::operator bool() const
{
    auto const self_id = getSamplerID();
    if (self_id == 0 || !samplerHandle_)
    {
        return false;
    }
    auto handle = globalSamplerStorages.try_acquire_read(self_id);
    return handle && handle->sampler != VK_NULL_HANDLE;
}
