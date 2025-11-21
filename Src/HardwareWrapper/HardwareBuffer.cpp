#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareVulkan/GlobalContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"

#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"

HardwareExecutorVulkan *getExecutorImpl(uintptr_t id);

Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap> globalBufferStorages;

VkBufferUsageFlags convertBufferUsage(BufferUsage usage)
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

void incrementBufferRefCount(uintptr_t bufferID)
{
    if (bufferID != 0)
    {
        auto handle = globalBufferStorages.acquire_write(bufferID)->refCount++;
    }
}

void decrementBufferRefCount(uintptr_t bufferID) {
    if (bufferID != 0) {
        bool shouldDestroy = false;
        {
            auto handle = globalBufferStorages.acquire_write(bufferID);
            handle->refCount--;
            if (handle->refCount == 0) {
                globalHardwareContext.getMainDevice()->resourceManager.destroyBuffer(*handle);
                shouldDestroy = true;
            }
        }
        if (shouldDestroy) {
            globalBufferStorages.deallocate(bufferID);
        }
    }
}

HardwareBuffer::HardwareBuffer()
    : bufferID(std::make_shared<uintptr_t>(0))
{
}

HardwareBuffer::HardwareBuffer(uint32_t bufferSize, uint32_t elementSize, BufferUsage usage, const void *data) {
    bufferID = std::make_shared<uintptr_t>(globalBufferStorages.allocate());

    auto handle = globalBufferStorages.acquire_write(*bufferID);

    *handle = globalHardwareContext.getMainDevice()->resourceManager.createBuffer(bufferSize, elementSize, convertBufferUsage(usage), true, true);
    handle->refCount = 1;

    if (data != nullptr && handle->bufferAllocInfo.pMappedData != nullptr) {
        std::memcpy(handle->bufferAllocInfo.pMappedData, data, static_cast<size_t>(bufferSize) * elementSize);
    }
}

HardwareBuffer::HardwareBuffer(const HardwareBuffer &other)
    : bufferID(other.bufferID)
{
    incrementBufferRefCount(*bufferID);
}

HardwareBuffer::~HardwareBuffer()
{
    if (bufferID)
    {
        decrementBufferRefCount(*bufferID);
    }
}

HardwareBuffer &HardwareBuffer::operator=(const HardwareBuffer &other)
{
    if (this != &other)
    {
        incrementBufferRefCount(*other.bufferID);
        decrementBufferRefCount(*bufferID);
        *(this->bufferID) = *(other.bufferID);
    }

    return *this;
}

HardwareBuffer::operator bool() const {
    if (bufferID && *bufferID != 0) {
        return globalBufferStorages.acquire_read(*bufferID)->bufferHandle != VK_NULL_HANDLE;
    } else {
        return false;
    }
}

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer &inputBuffer, HardwareExecutor *executor)
{
    if (!executor || !executor->getExecutorID() || *executor->getExecutorID() == 0)
    {
        return false;  // 必须提供有效的 executor
    }

    auto srcBuffer = globalBufferStorages.acquire_write(*inputBuffer.bufferID);
    auto dstBuffer = globalBufferStorages.acquire_write(*bufferID);

    HardwareExecutorVulkan *executorImpl = getExecutorImpl(*executor->getExecutorID());
    if (!executorImpl)
    {
        return false;
    }

    CopyBufferCommand copyCmd(*srcBuffer, *dstBuffer);
    (*executorImpl) << &copyCmd;

    return true;
}

uint32_t HardwareBuffer::storeDescriptor()
{
    auto bufferHandle = globalBufferStorages.acquire_read(*bufferID);
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(*bufferHandle);
}

bool HardwareBuffer::copyFromData(const void *inputData, uint64_t size) {
    if (inputData == nullptr || size == 0) {
        return false;
    }

    bool success = false;
    auto handle = globalBufferStorages.acquire_write(*bufferID);

    if (handle->bufferAllocInfo.pMappedData != nullptr) {
        std::memcpy(handle->bufferAllocInfo.pMappedData, inputData, size);
        success = true;
    }

    return success;
}

bool HardwareBuffer::copyToData(void *outputData, uint64_t size) {
    if (outputData == nullptr || size == 0) {
        return false;
    }

    bool success = false;
    auto handle = globalBufferStorages.acquire_write(*bufferID);
    if (handle->bufferAllocInfo.pMappedData != nullptr) {
        globalHardwareContext.getMainDevice()->resourceManager.copyBufferToHost(*handle, outputData, size);
        success = true;
    }

    return success;
}

void *HardwareBuffer::getMappedData()
{
    return globalBufferStorages.acquire_read(*bufferID)->bufferAllocInfo.pMappedData;
}

uint64_t HardwareBuffer::getElementCount() const
{
    return globalBufferStorages.acquire_read(*bufferID)->elementCount;
}

uint64_t HardwareBuffer::getElementSize() const
{
    return globalBufferStorages.acquire_read(*bufferID)->elementSize;
}

ExternalHandle HardwareBuffer::exportBufferMemory() {
    ExternalHandle winHandle{};
    auto bufferHandle = globalBufferStorages.acquire_write(*bufferID);

    ResourceManager::ExternalMemoryHandle mempryHandle = globalHardwareContext.getMainDevice()->resourceManager.exportBufferMemory(*bufferHandle);
#if _WIN32 || _WIN64
    winHandle.handle = mempryHandle.handle;
#else
    winHandle.fd = mempryHandle.fd;
#endif

    return winHandle;
}

HardwareBuffer::HardwareBuffer(const ExternalHandle &memHandle, uint32_t bufferSize, uint32_t elementSize, uint32_t allocSize, BufferUsage usage)
{
    ResourceManager::ExternalMemoryHandle mempryHandle;
#if _WIN32 || _WIN64
    mempryHandle.handle = memHandle.handle;
#else
    mempryHandle.fd = memHandle.fd;
#endif

    const VkBufferUsageFlags vkUsage = convertBufferUsage(usage);

    bufferID = std::make_shared<uintptr_t>(globalBufferStorages.allocate());
    auto bufferHandle = globalBufferStorages.acquire_write(*bufferID);

    *bufferHandle = globalHardwareContext.getMainDevice()->resourceManager.importBufferMemory(mempryHandle, bufferSize, elementSize, allocSize, vkUsage);
    bufferHandle->refCount = 1;
}