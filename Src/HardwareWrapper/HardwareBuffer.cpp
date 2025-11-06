#include"CabbageHardware.h"
#include<Hardware/GlobalContext.h>
#include<Hardware/ResourceCommand.h>

Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap> globalBufferStorages;

HardwareBuffer& HardwareBuffer::operator=(const HardwareBuffer &other)
{
    auto otherHandle = static_cast<uintptr_t>(*other.bufferIdPtr);
    auto thisHandle = static_cast<uintptr_t>(*this->bufferIdPtr);

    bool write_success = globalBufferStorages.write(otherHandle, [](ResourceManager::BufferHardwareWrap &buffer) {
        buffer.refCount++;
    });

    if (!write_success)
    {
        throw std::runtime_error("Failed to write HardwareBuffer!");
    }

        bool write_success = globalBufferStorages.write(thisHandle, [&](ResourceManager::BufferHardwareWrap &buffer) {
            buffer.refCount--;
            if (buffer.refCount == 0)
            {
                globalHardwareContext.mainDevice->resourceManager.destroyBuffer(buffer);
                globalBufferStorages.deallocate(thisHandle);
            }
        });

        if (!write_success)
        {
            throw std::runtime_error("Failed to write HardwareBuffer!");
        }


    *(this->bufferIdPtr) = *(other.bufferIdPtr);
    return *this;
}

HardwareBuffer::HardwareBuffer()
{
    this->bufferIdPtr = std::make_shared<uint64_t>(std::numeric_limits<uint64_t>::max());
}

HardwareBuffer::HardwareBuffer(const HardwareBuffer &other)
{
    this->bufferIdPtr = other.bufferIdPtr;

    auto otherHandle = static_cast<uintptr_t>(*other.bufferIdPtr);
    bool write_success = globalBufferStorages.write(otherHandle, [](ResourceManager::BufferHardwareWrap &buffer) {
        buffer.refCount++;
    });

    if (!write_success)
    {
        throw std::runtime_error("Failed to write HardwareBuffer!");
    }
}

HardwareBuffer::~HardwareBuffer()
{
    auto thisHandle = static_cast<uintptr_t>(*this->bufferIdPtr);
    bool write_success = globalBufferStorages.write(thisHandle, [&](ResourceManager::BufferHardwareWrap &buffer) {
        buffer.refCount--;
        if (buffer.refCount == 0)
        {
            globalHardwareContext.mainDevice->resourceManager.destroyBuffer(buffer);
            globalBufferStorages.deallocate(thisHandle);
        }
    });

    if (!write_success)
    {
        throw std::runtime_error("Failed to write HardwareBuffer!");
    }
}

HardwareBuffer::operator bool()
{
    if (bufferIdPtr != nullptr)
    {
        bool read_success = globalBufferStorages.read(static_cast<uintptr_t>(*bufferIdPtr), [](const ResourceManager::BufferHardwareWrap &buffer) {
            return buffer.bufferHandle != VK_NULL_HANDLE;
        });

        if (read_success)
        {
            throw std::runtime_error("Failed to read HardwareBuffer!");
        }
    }
    else
    {
        return false;
    }
}

HardwareBuffer::HardwareBuffer(uint64_t bufferSize, BufferUsage usage, const void* data)
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
        buffer = globalHardwareContext.mainDevice->resourceManager.createBuffer(bufferSize, vkUsage);
        buffer.refCount = 1;
        if (data != nullptr)
        {
            std::memcpy(buffer.bufferAllocInfo.pMappedData, data, bufferSize);
        }
    });

    this->bufferIdPtr = std::make_shared<uint64_t>(static_cast<uint64_t>(handle));
}

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer &inputBuffer, HardwareExecutor *executor)
{
    HardwareExecutor tempExecutor;

    ResourceManager::BufferHardwareWrap srcBuffer;
    ResourceManager::BufferHardwareWrap dstBuffer;
    bool read_srcBuffer_success = globalBufferStorages.read(static_cast<uintptr_t>(*inputBuffer.bufferIdPtr), [&](const ResourceManager::BufferHardwareWrap &buffer) {
        srcBuffer = buffer;
    });
    bool read_dstBuffer_success = globalBufferStorages.read(static_cast<uintptr_t>(*bufferIdPtr), [&](const ResourceManager::BufferHardwareWrap &buffer) {
        dstBuffer = buffer;
    });

    CopyBufferCommand copyCmd(srcBuffer, dstBuffer);
    tempExecutor << &copyCmd << tempExecutor.commit();

	return true;
}

uint32_t HardwareBuffer::storeDescriptor()
{
    ResourceManager::BufferHardwareWrap storeBuffer;
    bool read_success = globalBufferStorages.read(static_cast<uintptr_t>(*bufferIdPtr), [&](const ResourceManager::BufferHardwareWrap &buffer) {
        storeBuffer = buffer;
    });

    return globalHardwareContext.mainDevice->resourceManager.storeDescriptor(storeBuffer);
}

bool HardwareBuffer::copyFromData(const void* inputData, uint64_t size)
{
    ResourceManager::BufferHardwareWrap copyBuffer;
    bool read_success = globalBufferStorages.read(static_cast<uintptr_t>(*bufferIdPtr), [&](const ResourceManager::BufferHardwareWrap &buffer) {
        copyBuffer = buffer;
    });

	memcpy(copyBuffer.bufferAllocInfo.pMappedData, inputData, size);
	return true;
}

void* HardwareBuffer::getMappedData()
{
    ResourceManager::BufferHardwareWrap getBuffer;
    bool read_success = globalBufferStorages.read(static_cast<uintptr_t>(*bufferIdPtr), [&](const ResourceManager::BufferHardwareWrap &buffer) {
        getBuffer = buffer;
    });

	return getBuffer.bufferAllocInfo.pMappedData;
}

uint64_t HardwareBuffer::getBufferSize()
{
    ResourceManager::BufferHardwareWrap getBuffer;
    bool read_success = globalBufferStorages.read(static_cast<uintptr_t>(*bufferIdPtr), [&](const ResourceManager::BufferHardwareWrap &buffer) {
        getBuffer = buffer;
    });

	return getBuffer.bufferAllocInfo.size;
}
