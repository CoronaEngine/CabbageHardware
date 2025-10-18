#include"CabbageDisplayer.h"
#include<Hardware/GlobalContext.h>

std::unordered_map<uint64_t, ResourceManager::ImageHardwareWrap> imageGlobalPool;
std::unordered_map<uint64_t, uint64_t> imageRefCount;
uint64_t currentImageID = 0;

std::mutex imageMutex;

HardwareImage& HardwareImage::operator= (const HardwareImage& other)
{
    std::unique_lock<std::mutex> lock(imageMutex);

    if (imageGlobalPool.count(*other.imageID))
    {
        imageRefCount[*other.imageID]++;
    }
	if (imageGlobalPool.count(*this->imageID))
	{
        imageRefCount[*imageID]--;
        if (imageRefCount[*imageID] == 0)
        {
            globalHardwareContext.mainDevice->resourceManager.destroyImage(imageGlobalPool[*imageID]);
            imageGlobalPool.erase(*imageID);
            imageRefCount.erase(*imageID);
        }
	}
	*(this->imageID) = *(other.imageID);
	return *this;
}

HardwareImage::HardwareImage()
{
    std::unique_lock<std::mutex> lock(imageMutex);

    this->imageID = std::make_shared<uint64_t>(std::numeric_limits<uint64_t>::max());
}

HardwareImage::HardwareImage(const HardwareImage &other)
{
    std::unique_lock<std::mutex> lock(imageMutex);

    this->imageID = other.imageID;
    if (imageGlobalPool.count(*other.imageID))
    {
        imageRefCount[*other.imageID]++;
    }
}

HardwareImage::~HardwareImage()
{
    std::unique_lock<std::mutex> lock(imageMutex);

    if (imageGlobalPool.count(*imageID))
    {
        imageRefCount[*imageID]--;
        if (imageRefCount[*imageID] == 0)
        {
            globalHardwareContext.mainDevice->resourceManager.destroyImage(imageGlobalPool[*imageID]);
            imageGlobalPool.erase(*imageID);
            imageRefCount.erase(*imageID);
        }
    }
}

HardwareImage::operator bool()
{
    std::unique_lock<std::mutex> lock(imageMutex);

    return imageID != nullptr &&
           imageGlobalPool.count(*imageID) &&
           imageGlobalPool[*imageID].imageHandle != VK_NULL_HANDLE;
}

uint32_t HardwareImage::storeDescriptor()
{
    std::unique_lock<std::mutex> lock(imageMutex);

    return globalHardwareContext.mainDevice->resourceManager.storeDescriptor(imageGlobalPool[*imageID]);
}


bool HardwareImage::copyFromBuffer(const HardwareBuffer& buffer)
{
    globalHardwareContext.mainDevice->resourceManager.copyBufferToImage(bufferGlobalPool[*buffer.bufferID].bufferHandle, imageGlobalPool[*imageID].imageHandle, imageGlobalPool[*imageID].imageSize.x, imageGlobalPool[*imageID].imageSize.y);
	return true;
}

bool HardwareImage::copyFromData(const void* inputData)
{
	HardwareBuffer stagingBuffer = HardwareBuffer(imageGlobalPool[*imageID].imageSize.x * imageGlobalPool[*imageID].imageSize.y * imageGlobalPool[*imageID].pixelSize, BufferUsage::StorageBuffer, inputData);
	copyFromBuffer(stagingBuffer);
	return true;
}


HardwareImage::HardwareImage(ktm::uvec2 imageSize, ImageFormat imageFormat, ImageUsage imageUsage, int arrayLayers, void* imageData)
{
    std::unique_lock<std::mutex> lock(imageMutex);

    this->imageID = std::make_shared<uint64_t>(currentImageID++);

	imageRefCount[*this->imageID] = 1;

	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	switch (imageUsage)
	{
	case ImageUsage::SampledImage:
		vkImageUsageFlags = vkImageUsageFlags | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	case ImageUsage::StorageImage:
		vkImageUsageFlags = vkImageUsageFlags | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	case ImageUsage::DepthImage:
		vkImageUsageFlags = vkImageUsageFlags | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	default:
		break;
	}

	uint32_t pixelSize;
	VkFormat vkImageFormat;
	switch (imageFormat)
	{
	case ImageFormat::RGBA8_UINT:
		vkImageFormat = VK_FORMAT_R8G8B8A8_UINT;
		pixelSize = 8 * 4 / 8;
		break;
	case ImageFormat::RGBA8_SINT:
		vkImageFormat = VK_FORMAT_R8G8B8A8_SINT;
		pixelSize = 8 * 4 / 8;
		break;
	case ImageFormat::RGBA8_SRGB:
		vkImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		pixelSize = 8 * 4 / 8;
		break;
	case ImageFormat::RGBA16_UINT:
		vkImageFormat = VK_FORMAT_R16G16B16A16_UINT;
		pixelSize = 16 * 4 / 8;
		break;
	case ImageFormat::RGBA16_SINT:
		vkImageFormat = VK_FORMAT_R16G16B16A16_SINT;
		pixelSize = 16 * 4 / 8;
		break;
	case ImageFormat::RGBA16_FLOAT:
		vkImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		pixelSize = 16 * 4 / 8;
		break;
	case ImageFormat::RGBA32_UINT:
		vkImageFormat = VK_FORMAT_R32G32B32A32_UINT;
		pixelSize = 32 * 4 / 8;
		break;
	case ImageFormat::RGBA32_SINT:
		vkImageFormat = VK_FORMAT_R32G32B32A32_SINT;
		pixelSize = 32 * 4 / 8;
		break;
	case ImageFormat::RGBA32_FLOAT:
		vkImageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		pixelSize = 32 * 4 / 8;
		break;
	case ImageFormat::RG32_FLOAT:
		vkImageFormat = VK_FORMAT_R32G32_SFLOAT;
		pixelSize = 32 * 2 / 8;
		break;
	case ImageFormat::D16_UNORM:
		vkImageFormat = VK_FORMAT_D16_UNORM;
		pixelSize = 16 / 8;
		break;
	case ImageFormat::D32_FLOAT:
		vkImageFormat = VK_FORMAT_D32_SFLOAT;
		pixelSize = 32 / 8;
		break;
	default:
		break;
	}

	imageGlobalPool[*imageID] = globalHardwareContext.mainDevice->resourceManager.createImage(imageSize, vkImageFormat, pixelSize, vkImageUsageFlags, arrayLayers);

	if (imageData != nullptr)
	{
		copyFromData(imageData);
	}
}
