#include"CabbageHardware.h"
#include<Hardware/GlobalContext.h>

std::unordered_map<uint64_t, ResourceManager::BufferHardwareWrap> bufferGlobalPool;
std::unordered_map<uint64_t, uint64_t> bufferRefCount;
uint64_t currentBufferID = 0;

std::mutex bufferMutex;

HardwareBuffer& HardwareBuffer::operator= (const HardwareBuffer& other)
{
    std::unique_lock<std::mutex> lock(bufferMutex);

    if (bufferGlobalPool.count(*other.bufferID))
    {
        bufferRefCount[*other.bufferID]++;
    }
    if (bufferGlobalPool.count(*this->bufferID))
    {
        bufferRefCount[*bufferID]--;
        if (bufferRefCount[*bufferID] == 0)
        {
            globalHardwareContext.mainDevice->resourceManager.destroyBuffer(bufferGlobalPool[*bufferID]);
            bufferGlobalPool.erase(*bufferID);
            bufferRefCount.erase(*bufferID);
        }
    }
    *(this->bufferID) = *(other.bufferID);
    return *this;
}

HardwareBuffer::HardwareBuffer()
{
    std::unique_lock<std::mutex> lock(bufferMutex);

    this->bufferID = std::make_shared<uint64_t>(std::numeric_limits<uint64_t>::max());
}

HardwareBuffer::HardwareBuffer(const HardwareBuffer &other)
{
    std::unique_lock<std::mutex> lock(bufferMutex);

    this->bufferID = other.bufferID;
    if (bufferGlobalPool.count(*other.bufferID))
    {
        bufferRefCount[*other.bufferID]++;
    }
}

HardwareBuffer::~HardwareBuffer()
{
    std::unique_lock<std::mutex> lock(bufferMutex);

    if (bufferGlobalPool.count(*bufferID))
    {
        bufferRefCount[*bufferID]--;
        if (bufferRefCount[*bufferID] == 0)
        {
            globalHardwareContext.mainDevice->resourceManager.destroyBuffer(bufferGlobalPool[*bufferID]);
            bufferGlobalPool.erase(*bufferID);
            bufferRefCount.erase(*bufferID);
        }
    }
}

HardwareBuffer::operator bool()
{
    std::unique_lock<std::mutex> lock(bufferMutex);

    return bufferID != nullptr &&
           bufferGlobalPool.count(*bufferID) &&
           bufferGlobalPool[*bufferID].bufferHandle != VK_NULL_HANDLE;
}

HardwareBuffer::HardwareBuffer(uint64_t bufferSize, BufferUsage usage, const void* data)
{
    std::unique_lock<std::mutex> lock(bufferMutex);

    this->bufferID = std::make_shared<uint64_t>(currentBufferID++);
	bufferRefCount[*this->bufferID] = 1;

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

	bufferGlobalPool[*bufferID] = globalHardwareContext.mainDevice->resourceManager.createBuffer(bufferSize, vkUsage);

	if (data != nullptr)
	{
        memcpy(bufferGlobalPool[*bufferID].bufferAllocInfo.pMappedData, data, bufferSize);
	}
}

bool HardwareBuffer::copyFromBuffer(const HardwareBuffer& inputBuffer, uint64_t size)
{
    std::unique_lock<std::mutex> lock(bufferMutex);

    globalHardwareContext.mainDevice->resourceManager.copyBuffer(bufferGlobalPool[*inputBuffer.bufferID].bufferHandle, bufferGlobalPool[*bufferID].bufferHandle, size);
	return true;
}

uint32_t HardwareBuffer::storeDescriptor()
{
    std::unique_lock<std::mutex> lock(bufferMutex);

    return globalHardwareContext.mainDevice->resourceManager.storeDescriptor(bufferGlobalPool[*bufferID]);
}

bool HardwareBuffer::copyFromData(const void* inputData, uint64_t size)
{
    std::unique_lock<std::mutex> lock(bufferMutex);

	memcpy(bufferGlobalPool[*bufferID].bufferAllocInfo.pMappedData, inputData, size);
	return true;
}


void* HardwareBuffer::getMappedData()
{
    std::unique_lock<std::mutex> lock(bufferMutex);

	return bufferGlobalPool[*bufferID].bufferAllocInfo.pMappedData;
}

uint64_t HardwareBuffer::getBufferSize()
{
    std::unique_lock<std::mutex> lock(bufferMutex);

	return bufferGlobalPool[*bufferID].bufferAllocInfo.size;
}
