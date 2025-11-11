#include"CabbageHardware.h"
#include<Hardware/GlobalContext.h>
#include<Hardware/ResourceCommand.h>

Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap> globalBufferStorages;

HardwareBuffer::HardwareBuffer()
{
    this->bufferID = std::make_shared<uintptr_t>(0);
}

HardwareBuffer::HardwareBuffer(uint32_t bufferSize, uint32_t elementSize, BufferUsage usage, const void *data)
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

    auto handle = globalBufferStorages.allocate([&](ResourceManager::BufferHardwareWrap &buffer) {
        buffer = globalHardwareContext.mainDevice->resourceManager.createBuffer(bufferSize, elementSize, vkUsage);
        buffer.refCount = 1;
        if (data != nullptr)
        {
            std::memcpy(buffer.bufferAllocInfo.pMappedData, data, bufferSize * elementSize);
        }
    });

    this->bufferID = std::make_shared<uintptr_t>(handle);
}

HardwareBuffer::HardwareBuffer(const HardwareBuffer &other)
{
    this->bufferID = other.bufferID;
    if (*other.bufferID != 0)
    {
        globalBufferStorages.write(*other.bufferID, [](ResourceManager::BufferHardwareWrap &buffer) {
            buffer.refCount++;
        });
    }
}

HardwareBuffer::~HardwareBuffer()
{
    bool destroySelf = false;
    if (*bufferID != 0)
    {
        globalBufferStorages.write(*bufferID, [&](ResourceManager::BufferHardwareWrap &buffer) {
            buffer.refCount--;
            if (buffer.refCount == 0)
            {
                globalHardwareContext.mainDevice->resourceManager.destroyBuffer(buffer);
                destroySelf = true;
            }
        });
        if (destroySelf)
        {
            globalBufferStorages.deallocate(*bufferID);
        }
    }
}

HardwareBuffer& HardwareBuffer::operator=(const HardwareBuffer &other)
{
    if (*(other.bufferID) != 0)
    {
        globalBufferStorages.write(*other.bufferID, [](ResourceManager::BufferHardwareWrap &buffer) {
            buffer.refCount++;
        });
    }

    bool destroySelf = false;
    if (*bufferID != 0)
    {
        globalBufferStorages.write(*this->bufferID, [&](ResourceManager::BufferHardwareWrap &buffer) {
            buffer.refCount--;
            if (buffer.refCount == 0)
            {
                globalHardwareContext.mainDevice->resourceManager.destroyBuffer(buffer);
                destroySelf = true;
            }
        });
        if (destroySelf)
        {
            globalBufferStorages.deallocate(*this->bufferID);
        }
    }

    *(this->bufferID) = *(other.bufferID);
    return *this;
}

HardwareBuffer::operator bool()
{
    if (bufferID != nullptr && *bufferID != 0)
    {
        bool result = false;
        globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer) {
            if (buffer.bufferHandle != VK_NULL_HANDLE)
                result = true;
        });
        return result;
    }

    return false;
}

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer &inputBuffer, HardwareExecutor *executor)
{
    HardwareExecutor tempExecutor;
    ResourceManager::BufferHardwareWrap srcBuffer;
    ResourceManager::BufferHardwareWrap dstBuffer;
    bool read_srcBuffer_success = globalBufferStorages.read(*inputBuffer.bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer) {
        srcBuffer = buffer;
    });
    bool read_dstBuffer_success = globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer) {
        dstBuffer = buffer;
    });

    CopyBufferCommand copyCmd(srcBuffer, dstBuffer);
    tempExecutor << &copyCmd << tempExecutor.commit();

	return true;
}

uint32_t HardwareBuffer::storeDescriptor()
{
    uint32_t index = 0;
    globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer) {
        index = globalHardwareContext.mainDevice->resourceManager.storeDescriptor(buffer);
    });

    return index;
}

bool HardwareBuffer::copyFromData(const void* inputData, uint64_t size)
{
    globalBufferStorages.write(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer) {
        memcpy(buffer.bufferAllocInfo.pMappedData, inputData, size);
    });

	return true;
}

void* HardwareBuffer::getMappedData()
{
    ResourceManager::BufferHardwareWrap getBuffer;
    globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer) {
        getBuffer = buffer;
    });
    return getBuffer.bufferAllocInfo.pMappedData;
}

uint64_t HardwareBuffer::getBufferSize()
{
    ResourceManager::BufferHardwareWrap getBuffer;
    globalBufferStorages.read(*bufferID, [&](const ResourceManager::BufferHardwareWrap &buffer) {
        getBuffer = buffer;
    });
    return getBuffer.bufferSize * getBuffer.elementSize;
}


ExternalHandle HardwareBuffer::exportBufferMemory()
{
    ExternalHandle handle{};
    globalBufferStorages.write(*bufferID, [&](ResourceManager::BufferHardwareWrap &buffer) {
        ResourceManager::ExternalMemoryHandle mempryHandle = globalHardwareContext.mainDevice->resourceManager.exportBufferMemory(buffer);
#if _WIN32 || _WIN64
        handle.handle = mempryHandle.handle;
#else
        handle.fd = mempryHandle.fd;
#endif
    });
    return handle;
}

HardwareBuffer HardwareBuffer::importBufferMemory(const ExternalHandle& memHandle)
{
    ResourceManager::ExternalMemoryHandle mempryHandle;
#if _WIN32 || _WIN64
    mempryHandle.handle = memHandle.handle;
#else
    mempryHandle.fd = memHandle.fd;
#endif
    
    auto handle = globalBufferStorages.allocate([&](ResourceManager::BufferHardwareWrap &buffer) {
        buffer = globalHardwareContext.mainDevice->resourceManager.importBufferMemory(mempryHandle, buffer);
    });

    return *this;
}