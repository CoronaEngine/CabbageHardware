#include "HardwareWrapperVulkan/PipelineVulkan/ComputePipeline.h"

#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

static void incCompute(uint32_t id, const Corona::Kernel::Utils::Storage<ComputePipelineWrap>::WriteHandle& write_handle)
{
    ++write_handle->refCount;
    // CFW_LOG_TRACE("ComputePipeline ref++: id={}, count={}", id, write_handle->refCount);
}

static bool decCompute(uint32_t id, const Corona::Kernel::Utils::Storage<ComputePipelineWrap>::WriteHandle& write_handle) 
{
    int count = --write_handle->refCount;
    //CFW_LOG_TRACE("ComputePipeline ref--: id={}, count={}", id, count);
    if (count == 0) 
    {
        delete write_handle->impl;
        write_handle->impl = nullptr;
        //CFW_LOG_TRACE("ComputePipeline destroyed: id={}", id);
        return true;
    }
    return false;
}

ComputePipeline::ComputePipeline() 
{
    auto const id = gComputePipelineStorage.allocate();
    computePipelineID.store(id, std::memory_order_release);
    auto const handle = gComputePipelineStorage.acquire_write(id);
    handle->impl = new ComputePipelineVulkan();
    //CFW_LOG_TRACE("ComputePipeline created: id={}", id);
}

ComputePipeline::ComputePipeline(const std::string& shaderCode, EmbeddedShader::ShaderLanguage language, const std::source_location& src) 
{
    auto const id = gComputePipelineStorage.allocate();
    computePipelineID.store(id, std::memory_order_release);
    auto const handle = gComputePipelineStorage.acquire_write(id);
    handle->impl = new ComputePipelineVulkan(shaderCode, language, src);
    //CFW_LOG_TRACE("ComputePipeline created: id={}", id);
}

ComputePipeline::ComputePipeline(const ComputePipeline& other) 
{
    std::lock_guard<std::mutex> lock(other.computePipelineMutex);
    auto const other_id = other.computePipelineID.load(std::memory_order_acquire);
    computePipelineID.store(other_id, std::memory_order_release);
    if (other_id > 0)
    {
        auto const write_handle = gComputePipelineStorage.acquire_write(other_id);
        incCompute(other_id, write_handle);
    }
}

ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept
{
    std::lock_guard<std::mutex> lock(other.computePipelineMutex);
    auto const other_id = other.computePipelineID.load(std::memory_order_acquire);
    computePipelineID.store(other_id, std::memory_order_release);
    other.computePipelineID.store(0, std::memory_order_release);
}

ComputePipeline::~ComputePipeline() 
{
    auto const self_id = computePipelineID.load(std::memory_order_acquire);
    if (self_id > 0)
    {
        bool destroy = false;
        if (auto const write_handle = gComputePipelineStorage.acquire_write(self_id); decCompute(self_id, write_handle))
        {
            destroy = true;
        }

        if (destroy)
        {
            gComputePipelineStorage.deallocate(self_id);
        }
        computePipelineID.store(0, std::memory_order_release);
    }
}

ComputePipeline& ComputePipeline::operator=(const ComputePipeline& other) 
{
    if (this == &other) 
    {
        return *this;
    }
    std::scoped_lock lock(computePipelineMutex, other.computePipelineMutex);
    auto const self_id = computePipelineID.load(std::memory_order_acquire);
    auto const other_id = other.computePipelineID.load(std::memory_order_acquire);

    if (self_id == 0 && other_id == 0) 
    {
        return *this;
    }
    if (self_id == other_id) 
    {
        return *this;
    }

    if (other_id == 0) 
    {
        bool should_destroy_self = false;
        if (auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
            decCompute(self_id, self_handle)) 
        {
            should_destroy_self = true;
        }

        if (should_destroy_self) 
        {
            gComputePipelineStorage.deallocate(self_id);
        }
        computePipelineID.store(0, std::memory_order_release);
        return *this;
    }

    if (self_id == 0) 
    {
        computePipelineID.store(other_id, std::memory_order_release);
        auto const other_handle = gComputePipelineStorage.acquire_write(other_id);
        incCompute(other_id, other_handle);
        return *this;
    }

    bool should_destroy_self = false;
    if (self_id < other_id) 
    {
        auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
        auto const other_handle = gComputePipelineStorage.acquire_write(other_id);
        incCompute(other_id, other_handle);
        if (decCompute(self_id, self_handle)) 
        {
            should_destroy_self = true;
        }
    } 
    else 
    {
        auto const other_handle = gComputePipelineStorage.acquire_write(other_id);
        auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
        incCompute(other_id, other_handle);
        if (decCompute(self_id, self_handle)) 
        {
            should_destroy_self = true;
        }
    }

    if (should_destroy_self) 
    {
        gComputePipelineStorage.deallocate(self_id);
    }
    computePipelineID.store(other_id, std::memory_order_release);
    return *this;
}

ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) noexcept 
{
    if (this == &other) 
    {
        return *this;
    }
    std::scoped_lock lock(computePipelineMutex, other.computePipelineMutex);
    auto const self_id = computePipelineID.load(std::memory_order_acquire);
    auto const other_id = other.computePipelineID.load(std::memory_order_acquire);

    if (self_id > 0) 
    {
        bool should_destroy_self = false;
        if (auto const self_handle = gComputePipelineStorage.acquire_write(self_id);
            decCompute(self_id, self_handle)) 
        {
            should_destroy_self = true;
        }
        if (should_destroy_self)
        {
            gComputePipelineStorage.deallocate(self_id);
        }
    }
    computePipelineID.store(other_id, std::memory_order_release);
    other.computePipelineID.store(0, std::memory_order_release);
    return *this;
}

ResourceProxy ComputePipeline::operator[](const std::string& resourceName)
{
    return ResourceProxy(this, resourceName);
}

void ComputePipeline::setPushConstant(const std::string& name, const void* data, size_t size)
{
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    handle->impl->setPushConstant(name, data, size);
}

void ComputePipeline::setResource(const std::string& name, const HardwareBuffer& buffer)
{
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    handle->impl->setResource(name, buffer);
}

void ComputePipeline::setResource(const std::string& name, const HardwareImage& image) 
{
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    handle->impl->setResource(name, image);
}

HardwarePushConstant ComputePipeline::getPushConstant(const std::string& name) const
{
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    return handle->impl->getPushConstant(name);
}

HardwareBuffer ComputePipeline::getBuffer(const std::string& name) const
{
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    return handle->impl->getBuffer(name);
}

HardwareImage ComputePipeline::getImage(const std::string& name) const
{
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    return handle->impl->getImage(name);
}

ComputePipeline& ComputePipeline::operator()(uint16_t x, uint16_t y, uint16_t z)
{
    auto const handle = gComputePipelineStorage.acquire_read(computePipelineID.load(std::memory_order_acquire));
    (*handle->impl)(x, y, z);
    return *this;
}