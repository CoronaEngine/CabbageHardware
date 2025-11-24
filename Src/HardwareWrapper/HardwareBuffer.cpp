#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

VkBufferUsageFlags convertBufferUsage(BufferUsage const usage) {
    VkBufferUsageFlags vkUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    switch (usage) {
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

static void incrementBufferRefCount(const Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap>::WriteHandle& handle) {
    ++handle->refCount;
}

static bool decrementBufferRefCount(const Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap>::WriteHandle& handle) {
    if (--handle->refCount == 0) {
        globalHardwareContext.getMainDevice()->resourceManager.destroyBuffer(*handle);
        return true;
    }
    return false;
}

HardwareBuffer::HardwareBuffer()
    : bufferID(std::make_shared<uintptr_t>(0)) {
}

HardwareBuffer::HardwareBuffer(const uint32_t bufferSize, const uint32_t elementSize, const BufferUsage usage, const void* data) {
    bufferID = std::make_shared<uintptr_t>(globalBufferStorages.allocate());

    auto const handle = globalBufferStorages.acquire_write(*bufferID);
    *handle = globalHardwareContext.getMainDevice()->resourceManager.createBuffer(bufferSize, elementSize, convertBufferUsage(usage), true, true);
    handle->refCount = 1;

    if (data != nullptr && handle->bufferAllocInfo.pMappedData != nullptr) {
        std::memcpy(handle->bufferAllocInfo.pMappedData, data, static_cast<size_t>(bufferSize) * elementSize);
    }
}

HardwareBuffer::HardwareBuffer(const HardwareBuffer& other)
    : bufferID(other.bufferID) {
    if (*bufferID > 0) {
        auto const handle = globalBufferStorages.acquire_write(*bufferID);
        incrementBufferRefCount(handle);
    }
}

HardwareBuffer::~HardwareBuffer() {
    // NOTE: 不要修改写法，避免死锁
    if (bufferID && *bufferID > 0) {
        bool destroy = false;
        if (auto const handle = globalBufferStorages.acquire_write(*bufferID); decrementBufferRefCount(handle)) {
            destroy = true;
        }
        if (destroy) {
            globalBufferStorages.deallocate(*bufferID);
        }
    }
}

HardwareBuffer& HardwareBuffer::operator=(const HardwareBuffer& other) {
    if (this != &other) {
        {
            auto const handle = globalBufferStorages.acquire_write(*other.bufferID);
            incrementBufferRefCount(handle);
        }
        {  // NOTE: 不要修改写法，避免死锁
            if (bufferID && *bufferID > 0) {
                bool destroy = false;
                if (auto const handle = globalBufferStorages.acquire_write(*bufferID); decrementBufferRefCount(handle)) {
                    destroy = true;
                }
                if (destroy) {
                    globalBufferStorages.deallocate(*bufferID);
                }
            }
        }
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

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer& inputBuffer, const HardwareExecutor* executor) const {
    if (!executor || !executor->getExecutorID() || *executor->getExecutorID() == 0) {
        return false;  // 必须提供有效的 executor
    }

    auto const srcBuffer = globalBufferStorages.acquire_write(*inputBuffer.bufferID);
    auto const dstBuffer = globalBufferStorages.acquire_write(*bufferID);

    {
        auto const executor_handle = gExecutorStorage.acquire_write(*executor->getExecutorID());
        if (!executor_handle->impl) {
            return false;
        }

        CopyBufferCommand copyCmd(*srcBuffer, *dstBuffer);
        *executor_handle->impl << &copyCmd;
    }

    return true;
}

uint32_t HardwareBuffer::storeDescriptor() const {
    auto bufferHandle = globalBufferStorages.acquire_write(*bufferID);
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(bufferHandle);
}

bool HardwareBuffer::copyFromData(const void* inputData, const uint64_t size) const {
    if (inputData == nullptr || size == 0) {
        return false;
    }

    bool success = false;

    if (const auto handle = globalBufferStorages.acquire_write(*bufferID); handle->bufferAllocInfo.pMappedData != nullptr) {
        std::memcpy(handle->bufferAllocInfo.pMappedData, inputData, size);
        success = true;
    }

    return success;
}

bool HardwareBuffer::copyToData(void* outputData, const uint64_t size) const {
    if (outputData == nullptr || size == 0) {
        return false;
    }

    bool success = false;
    if (const auto handle = globalBufferStorages.acquire_write(*bufferID); handle->bufferAllocInfo.pMappedData != nullptr) {
        globalHardwareContext.getMainDevice()->resourceManager.copyBufferToHost(*handle, outputData, size);
        success = true;
    }

    return success;
}

void* HardwareBuffer::getMappedData() const {
    return globalBufferStorages.acquire_read(*bufferID)->bufferAllocInfo.pMappedData;
}

uint64_t HardwareBuffer::getElementCount() const {
    return globalBufferStorages.acquire_read(*bufferID)->elementCount;
}

uint64_t HardwareBuffer::getElementSize() const {
    return globalBufferStorages.acquire_read(*bufferID)->elementSize;
}

ExternalHandle HardwareBuffer::exportBufferMemory() {
    ExternalHandle winHandle{};
    const auto bufferHandle = globalBufferStorages.acquire_write(*bufferID);

    ResourceManager::ExternalMemoryHandle memory_handle = globalHardwareContext.getMainDevice()->resourceManager.exportBufferMemory(*bufferHandle);
#if _WIN32 || _WIN64
    winHandle.handle = memory_handle.handle;
#else
    winHandle.fd = memory_handle.fd;
#endif

    return winHandle;
}

HardwareBuffer::HardwareBuffer(const ExternalHandle& memHandle, const uint32_t bufferSize, const uint32_t elementSize, const uint32_t allocSize, const BufferUsage usage) {
    ResourceManager::ExternalMemoryHandle memory_handle;
#if _WIN32 || _WIN64
    memory_handle.handle = memHandle.handle;
#else
    memory_handle.fd = memHandle.fd;
#endif

    const VkBufferUsageFlags vkUsage = convertBufferUsage(usage);

    bufferID = std::make_shared<uintptr_t>(globalBufferStorages.allocate());
    const auto bufferHandle = globalBufferStorages.acquire_write(*bufferID);

    *bufferHandle = globalHardwareContext.getMainDevice()->resourceManager.importBufferMemory(memory_handle, bufferSize, elementSize, allocSize, vkUsage);
    bufferHandle->refCount = 1;
}