#include "CabbageHardware.h"
#include "HardwareCommands.h"
#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#include <cstring>
#include <limits>

VkBufferUsageFlags convertBufferUsage(BufferUsage const usage)
{
    VkBufferUsageFlags vkUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    switch (usage)
    {
    case BufferUsage::VertexBuffer:
        vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case BufferUsage::IndexBuffer:
        vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case BufferUsage::UniformBuffer:
        vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case BufferUsage::StorageBuffer:
        vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    default:
        break;
    }

    return vkUsage;
}

HardwareBuffer::HardwareBuffer() = default;

HardwareBuffer::HardwareBuffer(const HardwareBufferCreateInfo &createInfo, const void *data)
{
    bufferHandle_ = globalBufferStorages.allocate_handle(createInfo.debugName);
    auto const buffer_id = getBufferID();

    bool needsStagedUpload = false;
    const uint64_t dataSize = static_cast<uint64_t>(createInfo.elementCount) * createInfo.elementSize;

    {
        auto const handle = globalBufferStorages.acquire_write(buffer_id);
        *handle = globalHardwareContext.getMainDevice()->resourceManager.createBuffer(createInfo.elementCount,
                                                                                      createInfo.elementSize,
                                                                                      convertBufferUsage(createInfo.usage),
                                                                                      createInfo.hostVisibleMapped,
                                                                                      createInfo.useDedicated);
        handle->initialState = createInfo.initialState;
        handle->currentState = createInfo.initialState;
        handle->keepInitialState = createInfo.keepInitialState;
        handle->hostVisibleMapped = createInfo.hostVisibleMapped;
        handle->debugName = createInfo.debugName;

        if (data != nullptr && handle->bufferAllocInfo.pMappedData != nullptr)
        {
            std::memcpy(handle->bufferAllocInfo.pMappedData, data, static_cast<size_t>(dataSize));
        }
        else if (data != nullptr && dataSize > 0)
        {
            needsStagedUpload = true;
        }
    }

    if (needsStagedUpload)
    {
        copyFromData(data, dataSize);
    }
}

HardwareBuffer::HardwareBuffer(const uint32_t bufferSize, const uint32_t elementSize, const BufferUsage usage, const void *data, bool useDedicated)
{
    bufferHandle_ = globalBufferStorages.allocate_handle();
    auto const buffer_id = getBufferID();

    bool needsStagedUpload = false;
    const uint64_t dataSize = static_cast<uint64_t>(bufferSize) * elementSize;

    {
        auto const handle = globalBufferStorages.acquire_write(buffer_id);
        *handle = globalHardwareContext.getMainDevice()->resourceManager.createBuffer(bufferSize, elementSize, convertBufferUsage(usage), true, useDedicated);

        if (data != nullptr && handle->bufferAllocInfo.pMappedData != nullptr)
        {
            std::memcpy(handle->bufferAllocInfo.pMappedData, data, static_cast<size_t>(dataSize));
        }
        else if (data != nullptr && dataSize > 0)
        {
            needsStagedUpload = true;
        }
    }

    if (needsStagedUpload)
    {
        copyFromData(data, dataSize);
    }
}

HardwareBuffer::HardwareBuffer(const ExternalHandle &memHandle, const uint32_t bufferSize, const uint32_t elementSize, const uint32_t allocSize, const BufferUsage usage)
{
    ResourceManager::ExternalMemoryHandle memory_handle;
#if _WIN32 || _WIN64
    memory_handle.handle = memHandle.handle;
#else
    memory_handle.fd = memHandle.fd;
#endif

    const VkBufferUsageFlags vkUsage = convertBufferUsage(usage);
    bufferHandle_ = globalBufferStorages.allocate_handle();
    auto const buffer_id = getBufferID();
    auto const bufferHandle = globalBufferStorages.acquire_write(buffer_id);
    *bufferHandle = globalHardwareContext.getMainDevice()->resourceManager.importBufferMemory(memory_handle, bufferSize, elementSize, allocSize, vkUsage);
}

HardwareBuffer::HardwareBuffer(const HardwareBuffer &other)
    : bufferHandle_(other.bufferHandle_)
{
}

HardwareBuffer::HardwareBuffer(HardwareBuffer &&other) noexcept
    : bufferHandle_(std::move(other.bufferHandle_))
{
}

HardwareBuffer &HardwareBuffer::operator=(HardwareBuffer &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    bufferHandle_ = std::move(other.bufferHandle_);
    return *this;
}

HardwareBuffer::~HardwareBuffer() = default;

HardwareBuffer &HardwareBuffer::operator=(const HardwareBuffer &other)
{
    if (this == &other)
    {
        return *this;
    }
    bufferHandle_ = other.bufferHandle_;
    return *this;
}

HardwareBuffer::operator bool() const
{
    auto const self_buffer_id = getBufferID();
    if (self_buffer_id == 0 || !bufferHandle_)
    {
        return false;
    }
    auto handle = globalBufferStorages.try_acquire_read(self_buffer_id);
    return handle && handle->bufferHandle != VK_NULL_HANDLE;
}

BufferCopyCommand HardwareBuffer::copyTo(const HardwareBuffer &dst,
                                         uint64_t srcOffset,
                                         uint64_t dstOffset,
                                         uint64_t size) const
{
    return BufferCopyCommand(*this, dst, srcOffset, dstOffset, size);
}

BufferToImageCommand HardwareBuffer::copyTo(const HardwareImage &dst,
                                            uint64_t bufferOffset,
                                            uint32_t imageLayer,
                                            uint32_t imageMip) const
{
    return BufferToImageCommand(*this, dst, bufferOffset, imageLayer, imageMip);
}

uint32_t HardwareBuffer::storeDescriptor() const
{
    auto bufferHandle = globalBufferStorages.acquire_write(getBufferID());
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(bufferHandle);
}

bool HardwareBuffer::copyFromData(const void *inputData, const uint64_t size) const
{
    if (inputData == nullptr || size == 0)
    {
        return false;
    }
    auto const self_buffer_id = getBufferID();
    if (self_buffer_id == 0)
    {
        CFW_LOG_WARNING("Cannot copy data to an uninitialized HardwareBuffer.");
        return false;
    }
    {
        const auto handle = globalBufferStorages.acquire_write(self_buffer_id);
        if (handle->bufferAllocInfo.pMappedData != nullptr)
        {
            std::memcpy(handle->bufferAllocInfo.pMappedData, inputData, static_cast<size_t>(size));
            return true;
        }
    }

    if (size > std::numeric_limits<uint32_t>::max())
    {
        CFW_LOG_WARNING("Cannot stage upload larger than 4 GiB with the current HardwareBuffer API.");
        return false;
    }

    HardwareBuffer stagingBuffer(static_cast<uint32_t>(size), 1, BufferUsage::StorageBuffer, nullptr, false);
    void *mappedData = stagingBuffer.getMappedData();
    if (mappedData == nullptr)
    {
        CFW_LOG_WARNING("Cannot stage upload because the staging buffer is not host-visible.");
        return false;
    }
    std::memcpy(mappedData, inputData, static_cast<size_t>(size));

    HardwareExecutor executor;
    executor << stagingBuffer.copyTo(*this, 0, 0, size) << executor.commit();
    executor.waitForDeferredResources();
    return true;
}

bool HardwareBuffer::copyToData(void *outputData, const uint64_t size) const
{
    if (outputData == nullptr || size == 0)
    {
        return false;
    }
    auto const self_buffer_id = getBufferID();
    if (self_buffer_id == 0)
    {
        CFW_LOG_WARNING("Cannot copy uninitialized HardwareBuffer to out data.");
        return false;
    }
    if (const auto handle = globalBufferStorages.acquire_write(self_buffer_id);
        handle->bufferAllocInfo.pMappedData != nullptr)
    {
        globalHardwareContext.getMainDevice()->resourceManager.copyBufferToHost(*handle, outputData, size);
        return true;
    }
    return false;
}

bool HardwareBuffer::readback(void *outputData, const uint64_t size) const
{
    if (copyToData(outputData, size))
    {
        return true;
    }

    if (outputData == nullptr || size == 0)
    {
        return false;
    }

    if (size > std::numeric_limits<uint32_t>::max())
    {
        CFW_LOG_WARNING("Cannot stage readback larger than 4 GiB with the current HardwareBuffer API.");
        return false;
    }

    HardwareBuffer stagingBuffer(static_cast<uint32_t>(size), 1, BufferUsage::StorageBuffer, nullptr, false);
    HardwareExecutor executor;
    executor << copyTo(stagingBuffer, 0, 0, size) << executor.commit();
    executor.waitForDeferredResources();
    return stagingBuffer.copyToData(outputData, size);
}

void *HardwareBuffer::getMappedData() const
{
    return globalBufferStorages.acquire_read(getBufferID())->bufferAllocInfo.pMappedData;
}

uint64_t HardwareBuffer::getElementCount() const
{
    return globalBufferStorages.acquire_read(getBufferID())->elementCount;
}

uint64_t HardwareBuffer::getElementSize() const
{
    return globalBufferStorages.acquire_read(getBufferID())->elementSize;
}

ExternalHandle HardwareBuffer::exportBufferMemory()
{
    ExternalHandle winHandle{};
    const auto bufferHandle = globalBufferStorages.acquire_write(getBufferID());

    ResourceManager::ExternalMemoryHandle memory_handle = globalHardwareContext.getMainDevice()->resourceManager.exportBufferMemory(*bufferHandle);
#if _WIN32 || _WIN64
    winHandle.handle = memory_handle.handle;
#else
    winHandle.fd = memory_handle.fd;
#endif

    return winHandle;
}
