#include "CabbageHardware.h"

#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

namespace
{
void incrementSamplerRefCount(uint64_t, const Corona::Kernel::Utils::Storage<ResourceManager::SamplerHardwareWrap>::WriteHandle &handle)
{
    ++handle->refCount;
}

bool decrementSamplerRefCount(uint64_t, const Corona::Kernel::Utils::Storage<ResourceManager::SamplerHardwareWrap>::WriteHandle &handle)
{
    const uint64_t count = --handle->refCount;
    if (count == 0)
    {
        if (handle->resourceManager)
        {
            handle->resourceManager->destroySampler(*handle);
        }
        return true;
    }
    return false;
}
} // namespace

HardwareSampler::HardwareSampler()
    : samplerID(0)
{
}

HardwareSampler::HardwareSampler(const SamplerDesc &desc)
{
    auto const sampler_id = globalSamplerStorages.allocate();
    samplerID.store(sampler_id, std::memory_order_release);
    auto handle = globalSamplerStorages.acquire_write(sampler_id);
    *handle = globalHardwareContext.getMainDevice()->resourceManager.createSampler(desc);
}

HardwareSampler::HardwareSampler(const HardwareSampler &other)
{
    std::lock_guard<std::mutex> lock(other.samplerMutex);
    auto const other_id = other.samplerID.load(std::memory_order_acquire);
    samplerID.store(other_id, std::memory_order_release);
    if (other_id > 0)
    {
        auto handle = globalSamplerStorages.acquire_write(other_id);
        incrementSamplerRefCount(other_id, handle);
    }
}

HardwareSampler::HardwareSampler(HardwareSampler &&other) noexcept
{
    std::lock_guard<std::mutex> lock(other.samplerMutex);
    auto const other_id = other.samplerID.load(std::memory_order_acquire);
    samplerID.store(other_id, std::memory_order_release);
    other.samplerID.store(0, std::memory_order_release);
}

HardwareSampler::~HardwareSampler()
{
    auto const self_id = samplerID.load(std::memory_order_acquire);
    if (self_id == 0)
    {
        return;
    }

    bool shouldDestroy = false;
    if (auto handle = globalSamplerStorages.acquire_write(self_id);
        decrementSamplerRefCount(self_id, handle))
    {
        shouldDestroy = true;
    }
    if (shouldDestroy)
    {
        globalSamplerStorages.deallocate(self_id);
    }
    samplerID.store(0, std::memory_order_release);
}

HardwareSampler &HardwareSampler::operator=(const HardwareSampler &other)
{
    if (this == &other)
    {
        return *this;
    }

    std::scoped_lock lock(samplerMutex, other.samplerMutex);
    auto const self_id = samplerID.load(std::memory_order_acquire);
    auto const other_id = other.samplerID.load(std::memory_order_acquire);

    if (self_id == other_id)
    {
        return *this;
    }

    if (other_id > 0)
    {
        auto other_handle = globalSamplerStorages.acquire_write(other_id);
        incrementSamplerRefCount(other_id, other_handle);
    }

    if (self_id > 0)
    {
        bool shouldDestroy = false;
        if (auto self_handle = globalSamplerStorages.acquire_write(self_id);
            decrementSamplerRefCount(self_id, self_handle))
        {
            shouldDestroy = true;
        }
        if (shouldDestroy)
        {
            globalSamplerStorages.deallocate(self_id);
        }
    }

    samplerID.store(other_id, std::memory_order_release);
    return *this;
}

HardwareSampler &HardwareSampler::operator=(HardwareSampler &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    std::scoped_lock lock(samplerMutex, other.samplerMutex);
    auto const self_id = samplerID.load(std::memory_order_acquire);
    auto const other_id = other.samplerID.load(std::memory_order_acquire);

    if (self_id > 0)
    {
        bool shouldDestroy = false;
        if (auto self_handle = globalSamplerStorages.acquire_write(self_id);
            decrementSamplerRefCount(self_id, self_handle))
        {
            shouldDestroy = true;
        }
        if (shouldDestroy)
        {
            globalSamplerStorages.deallocate(self_id);
        }
    }

    samplerID.store(other_id, std::memory_order_release);
    other.samplerID.store(0, std::memory_order_release);
    return *this;
}

HardwareSampler::operator bool() const
{
    auto const self_id = samplerID.load(std::memory_order_acquire);
    return self_id > 0 && globalSamplerStorages.acquire_read(self_id)->sampler != VK_NULL_HANDLE;
}
