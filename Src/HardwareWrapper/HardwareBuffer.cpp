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

static void incrementBufferRefCount(uint32_t id, const Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap>::WriteHandle& handle) {
    ++handle->refCount;
    // CFW_LOG_TRACE("HardwareBuffer ref++: id={}, count={}", id, handle->refCount);
}

static bool decrementBufferRefCount(uint32_t id, const Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap>::WriteHandle& handle) {
    if (handle->refCount == 0) {
        CFW_LOG_ERROR("HardwareBuffer ref count underflow! id={}", id);
    }
    int count = --handle->refCount;
    // CFW_LOG_TRACE("HardwareBuffer ref--: id={}, count={}", id, count);
    if (count == 0) {
        globalHardwareContext.getMainDevice()->resourceManager.destroyBuffer(*handle);
        // CFW_LOG_TRACE("HardwareBuffer destroyed: id={}", id);
        return true;
    }
    return false;
}

HardwareBuffer::HardwareBuffer()
    : bufferID(0) {
}

HardwareBuffer::HardwareBuffer(const uint32_t bufferSize, const uint32_t elementSize, const BufferUsage usage, const void* data, bool useDedicated) {
    auto const buffer_id = globalBufferStorages.allocate();
    bufferID.store(buffer_id, std::memory_order_release);
    auto const handle = globalBufferStorages.acquire_write(buffer_id);
    *handle = globalHardwareContext.getMainDevice()->resourceManager.createBuffer(bufferSize, elementSize, convertBufferUsage(usage), true, useDedicated);
    CFW_LOG_TRACE("HardwareBuffer created: id={}", buffer_id);

    if (data != nullptr && handle->bufferAllocInfo.pMappedData != nullptr) {
        std::memcpy(handle->bufferAllocInfo.pMappedData, data, static_cast<size_t>(bufferSize) * elementSize);
    }
}

HardwareBuffer::HardwareBuffer(const ExternalHandle& memHandle, const uint32_t bufferSize, const uint32_t elementSize, const uint32_t allocSize, const BufferUsage usage) {
    ResourceManager::ExternalMemoryHandle memory_handle;
#if _WIN32 || _WIN64
    memory_handle.handle = memHandle.handle;
#else
    memory_handle.fd = memHandle.fd;
#endif

    const VkBufferUsageFlags vkUsage = convertBufferUsage(usage);
    auto const buffer_id = globalBufferStorages.allocate();
    bufferID.store(buffer_id, std::memory_order_release);
    auto const bufferHandle = globalBufferStorages.acquire_write(buffer_id);
    *bufferHandle = globalHardwareContext.getMainDevice()->resourceManager.importBufferMemory(memory_handle, bufferSize, elementSize, allocSize, vkUsage);
    CFW_LOG_TRACE("HardwareBuffer created from external: id={}", buffer_id);
}

HardwareBuffer::HardwareBuffer(const HardwareBuffer& other) {
    std::lock_guard<std::mutex> lock(other.bufferMutex);
    auto const other_buffer_id = other.bufferID.load(std::memory_order_acquire);
    bufferID.store(other_buffer_id, std::memory_order_release);
    if (other_buffer_id > 0) {
        auto const handle = globalBufferStorages.acquire_write(other_buffer_id);
        incrementBufferRefCount(other_buffer_id, handle);
    }
}

HardwareBuffer::HardwareBuffer(HardwareBuffer&& other) noexcept {
    std::lock_guard<std::mutex> lock(other.bufferMutex);
    auto const other_buffer_id = other.bufferID.load(std::memory_order_relaxed);
    other.bufferID.store(0, std::memory_order_release);
    bufferID.store(other_buffer_id, std::memory_order_release);
    // CFW_LOG_DEBUG("HardwareBuffer ctor(move): this={}, src={}, id={}", (void*)this, (void*)&other, other_buffer_id);
}

HardwareBuffer& HardwareBuffer::operator=(HardwareBuffer&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    std::scoped_lock lock(bufferMutex, other.bufferMutex);
    auto const self_id = bufferID.load(std::memory_order_acquire);
    auto const other_id = other.bufferID.load(std::memory_order_acquire);

    if (self_id > 0) {
        bool should_destroy_self = false;
        if (auto const handle = globalBufferStorages.acquire_write(self_id);
            decrementBufferRefCount(self_id, handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            globalBufferStorages.deallocate(self_id);
        }
    }
    bufferID.store(other_id, std::memory_order_release);
    other.bufferID.store(0, std::memory_order_release);
    return *this;
}

HardwareBuffer::~HardwareBuffer() {
    // NOTE: 不要修改写法，避免死锁
    auto const self_buffer_id = bufferID.load(std::memory_order_acquire);
    // CFW_LOG_DEBUG("HardwareBuffer dtor: this={}, id={}", (void*)this, self_buffer_id);
    if (self_buffer_id > 0) {
        bool destroy = false;
        if (auto const handle = globalBufferStorages.acquire_write(self_buffer_id);
            decrementBufferRefCount(self_buffer_id, handle)) {
            destroy = true;
        }
        if (destroy) {
            globalBufferStorages.deallocate(self_buffer_id);
        }
    }
}

HardwareBuffer& HardwareBuffer::operator=(const HardwareBuffer& other) {
    if (this == &other) {
        return *this;
    }
    std::scoped_lock lock(bufferMutex, other.bufferMutex);
    auto const self_buffer_id = bufferID.load(std::memory_order_acquire);
    auto const other_buffer_id = other.bufferID.load(std::memory_order_acquire);

    if (self_buffer_id == 0 && other_buffer_id == 0) {
        return *this;
    }

    if (self_buffer_id == other_buffer_id) {
        return *this;
    }

    if (other_buffer_id == 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = globalBufferStorages.acquire_write(self_buffer_id);
            decrementBufferRefCount(self_buffer_id, self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            globalBufferStorages.deallocate(self_buffer_id);
        }
        bufferID.store(0, std::memory_order_release);
        CFW_LOG_WARNING("Copying from an uninitialized HardwareBuffer.");
        return *this;
    }

    if (self_buffer_id == 0) {
        bufferID.store(other_buffer_id, std::memory_order_release);
        auto const other_handle = globalBufferStorages.acquire_write(other_buffer_id);
        incrementBufferRefCount(other_buffer_id, other_handle);
        return *this;
    }

    bool should_destroy_self = false;
    if (self_buffer_id < other_buffer_id) {
        auto const self_handle = globalBufferStorages.acquire_write(self_buffer_id);
        auto const other_handle = globalBufferStorages.acquire_write(other_buffer_id);
        incrementBufferRefCount(other_buffer_id, other_handle);
        if (decrementBufferRefCount(self_buffer_id, self_handle)) {
            should_destroy_self = true;
        }
    } else {
        auto const other_handle = globalBufferStorages.acquire_write(other_buffer_id);
        auto const self_handle = globalBufferStorages.acquire_write(self_buffer_id);
        incrementBufferRefCount(other_buffer_id, other_handle);
        if (decrementBufferRefCount(self_buffer_id, self_handle)) {
            should_destroy_self = true;
        }
    }
    if (should_destroy_self) {
        globalBufferStorages.deallocate(self_buffer_id);
    }
    bufferID.store(other_buffer_id, std::memory_order_release);
    return *this;
}

HardwareBuffer::operator bool() const {
    auto const self_buffer_id = bufferID.load(std::memory_order_acquire);
    return self_buffer_id > 0 &&
           globalBufferStorages.acquire_read(self_buffer_id)->bufferHandle != VK_NULL_HANDLE;
}

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer& inputBuffer, const HardwareExecutor* executor) const {
    if (!executor || executor->getExecutorID() == 0) {
        return false;
    }
    auto const input_buffer_id = inputBuffer.bufferID.load(std::memory_order_acquire);
    auto const self_buffer_id = bufferID.load(std::memory_order_acquire);

    if (executor->getExecutorID() == 0) {
        CFW_LOG_WARNING("Invalid HardwareExecutor provided for buffer copy.");
        return false;
    }

    if (input_buffer_id == 0 || self_buffer_id == 0) {
        CFW_LOG_WARNING("Copy operation failed due to uninitialized HardwareBuffer.");
        return false;
    }

    if (input_buffer_id < self_buffer_id) {
        auto const executor_handle = gExecutorStorage.acquire_write(executor->getExecutorID());
        auto const srcBuffer = globalBufferStorages.acquire_write(input_buffer_id);
        auto const dstBuffer = globalBufferStorages.acquire_write(self_buffer_id);
        CopyBufferCommand copyCmd(*srcBuffer, *dstBuffer);

        if (!executor_handle->impl) {
            return false;
        }
        *executor_handle->impl << &copyCmd;
    } else {
        auto const executor_handle = gExecutorStorage.acquire_write(executor->getExecutorID());
        auto const dstBuffer = globalBufferStorages.acquire_write(self_buffer_id);
        auto const srcBuffer = globalBufferStorages.acquire_write(input_buffer_id);
        CopyBufferCommand copyCmd(*srcBuffer, *dstBuffer);

        if (!executor_handle->impl) {
            return false;
        }
        *executor_handle->impl << &copyCmd;
    }

    return true;
}

uint32_t HardwareBuffer::storeDescriptor() const {
    auto bufferHandle = globalBufferStorages.acquire_write(bufferID);
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(bufferHandle);
}

bool HardwareBuffer::copyFromData(const void* inputData, const uint64_t size) const {
    if (inputData == nullptr || size == 0) {
        return false;
    }
    auto const self_buffer_id = bufferID.load(std::memory_order_acquire);
    if (self_buffer_id == 0) {
        CFW_LOG_WARNING("Cannot copy data to an uninitialized HardwareBuffer.");
        return false;
    }
    if (const auto handle = globalBufferStorages.acquire_write(self_buffer_id);
        handle->bufferAllocInfo.pMappedData != nullptr) {
        std::memcpy(handle->bufferAllocInfo.pMappedData, inputData, size);
        return true;
    }
    return false;
}

bool HardwareBuffer::copyToData(void* outputData, const uint64_t size) const {
    if (outputData == nullptr || size == 0) {
        return false;
    }
    auto const self_buffer_id = bufferID.load(std::memory_order_acquire);
    if (self_buffer_id == 0) {
        CFW_LOG_WARNING("Cannot copy uninitialized HardwareBuffer to out data.");
        return false;
    }
    if (const auto handle = globalBufferStorages.acquire_write(self_buffer_id);
        handle->bufferAllocInfo.pMappedData != nullptr) {
        globalHardwareContext.getMainDevice()->resourceManager.copyBufferToHost(*handle, outputData, size);
        return true;
    }
    return false;
}

void* HardwareBuffer::getMappedData() const {
    return globalBufferStorages.acquire_read(bufferID.load(std::memory_order_acquire))->bufferAllocInfo.pMappedData;
}

uint64_t HardwareBuffer::getElementCount() const {
    return globalBufferStorages.acquire_read(bufferID.load(std::memory_order_acquire))->elementCount;
}

uint64_t HardwareBuffer::getElementSize() const {
    return globalBufferStorages.acquire_read(bufferID.load(std::memory_order_acquire))->elementSize;
}

ExternalHandle HardwareBuffer::exportBufferMemory() {
    ExternalHandle winHandle{};
    const auto bufferHandle = globalBufferStorages.acquire_write(bufferID.load(std::memory_order_acquire));

    ResourceManager::ExternalMemoryHandle memory_handle = globalHardwareContext.getMainDevice()->resourceManager.exportBufferMemory(*bufferHandle);
#if _WIN32 || _WIN64
    winHandle.handle = memory_handle.handle;
#else
    winHandle.fd = memory_handle.fd;
#endif

    return winHandle;
}