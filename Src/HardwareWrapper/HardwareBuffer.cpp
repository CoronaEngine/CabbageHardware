#include"CabbageHardware.h"
#include<Hardware/GlobalContext.h>
#include<Hardware/ResourceCommand.h>

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
        globalBufferStorages.write(bufferID, [](ResourceManager::BufferHardwareWrap &buffer)
        {
            ++buffer.refCount;
        });
    }
}

void decrementBufferRefCount(uintptr_t bufferID)
{
    if (bufferID == 0)
    {
        return;
    }

    bool shouldDestroy = false;
    globalBufferStorages.write(bufferID, [&](ResourceManager::BufferHardwareWrap &buffer)
    {
        if (--buffer.refCount == 0)
        {
            globalHardwareContext.getMainDevice()->resourceManager.destroyBuffer(buffer);
            shouldDestroy = true;
        }
    });

    if (shouldDestroy)
    {
        globalBufferStorages.deallocate(bufferID);
    }
}

HardwareBuffer::HardwareBuffer()
    : bufferID(std::make_shared<uintptr_t>(0))
{
}

HardwareBuffer::HardwareBuffer(uint32_t bufferSize, uint32_t elementSize, BufferUsage usage, const void *data)
{
    const VkBufferUsageFlags vkUsage = convertBufferUsage(usage);

    const auto handle = globalBufferStorages.allocate([&](ResourceManager::BufferHardwareWrap &buffer)
    {
        buffer = globalHardwareContext.getMainDevice()->resourceManager.createBuffer(bufferSize, elementSize, vkUsage);
        buffer.refCount = 1;

        if (data != nullptr && buffer.bufferAllocInfo.pMappedData != nullptr)
        {
            std::memcpy(buffer.bufferAllocInfo.pMappedData, data, static_cast<size_t>(bufferSize) * elementSize);
        }
    });

    bufferID = std::make_shared<uintptr_t>(handle);
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

HardwareBuffer::operator bool() const
{
    if (!bufferID || *bufferID == 0)
    {
        return false;
    }

    bool isValid = false;
    globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer)
    {
        isValid = (buffer.bufferHandle != VK_NULL_HANDLE);
    });

    return isValid;
}

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer &inputBuffer, HardwareExecutor *executor)
{
    ResourceManager::BufferHardwareWrap srcBuffer;
    ResourceManager::BufferHardwareWrap dstBuffer;

    const bool srcValid = globalBufferStorages.read(*inputBuffer.bufferID,
        [&](const ResourceManager::BufferHardwareWrap &buffer)
        {
            srcBuffer = buffer;
        });

    const bool dstValid = globalBufferStorages.read(*bufferID,
        [&](const ResourceManager::BufferHardwareWrap &buffer)
        {
            dstBuffer = buffer;
        });

    if (!srcValid || !dstValid)
    {
        return false;
    }

    HardwareExecutor tempExecutor;
    CopyBufferCommand copyCmd(srcBuffer, dstBuffer);
    tempExecutor << &copyCmd << tempExecutor.commit();

    return true;
}

uint32_t HardwareBuffer::storeDescriptor()
{
    uint32_t index = 0;
    globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer)
    {
        index = globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(buffer);
    });

    return index;
}

bool HardwareBuffer::copyFromData(const void *inputData, uint64_t size)
{
    if (inputData == nullptr || size == 0)
    {
        return false;
    }

    bool success = false;
    globalBufferStorages.write(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer)
    {
        if (buffer.bufferAllocInfo.pMappedData != nullptr)
        {
            std::memcpy(buffer.bufferAllocInfo.pMappedData, inputData, size);
            success = true;
        }
    });

    return success;
}


bool HardwareBuffer::copyToData(void *outputData, uint64_t size)
{
    if (outputData == nullptr || size == 0)
    {
        return false;
    }

    bool success = false;
    globalBufferStorages.write(*bufferID, [&](ResourceManager::BufferHardwareWrap &buffer) {
        if (buffer.bufferAllocInfo.pMappedData != nullptr)
        {
            globalHardwareContext.getMainDevice()->resourceManager.copyBufferToHost(buffer, outputData, size);
            success = true;
        }
    });

    return success;
}


void *HardwareBuffer::getMappedData()
{
    void *mappedData = nullptr;
    globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer)
    {
        mappedData = buffer.bufferAllocInfo.pMappedData;
    });

    return mappedData;
}

uint64_t HardwareBuffer::getElementCount() const
{
    uint64_t totalSize = 0;
    globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer)
    {
        totalSize = buffer.elementCount;
    });

    return totalSize;
}

uint64_t HardwareBuffer::getElementSize() const
{
    uint64_t totalSize = 0;
    globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer) {
        totalSize = buffer.elementSize;
    });

    return totalSize;
}

ExternalHandle HardwareBuffer::exportBufferMemory()
{
    ExternalHandle handle{};
    globalBufferStorages.write(*bufferID, [&](ResourceManager::BufferHardwareWrap &buffer)
    {
        ResourceManager::ExternalMemoryHandle mempryHandle = globalHardwareContext.getMainDevice()->resourceManager.exportBufferMemory(buffer);
#if _WIN32 || _WIN64
        handle.handle = mempryHandle.handle;
#else
        handle.fd = mempryHandle.fd;
#endif
    });
    return handle;
}

HardwareBuffer::HardwareBuffer(const ExternalHandle &memHandle, const ResourceManager::BufferHardwareWrap &sourceBuffer)
{
    ResourceManager::ExternalMemoryHandle mempryHandle;
#if _WIN32 || _WIN64
    mempryHandle.handle = memHandle.handle;
#else
    mempryHandle.fd = memHandle.fd;
#endif

    const auto handle = globalBufferStorages.allocate([&](ResourceManager::BufferHardwareWrap &buffer)
    {
        buffer = globalHardwareContext.getMainDevice()->resourceManager.importBufferMemory(mempryHandle, sourceBuffer);
        buffer.refCount = 1;
    });

    bufferID = std::make_shared<uintptr_t>(handle);
}