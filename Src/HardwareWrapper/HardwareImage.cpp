#include "CabbageHardware.h"

#include "corona/kernel/utils/storage.h"
#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"

#include "HardwareWrapperVulkan/ResourcePool.h"

HardwareExecutorVulkan *getExecutorImpl(uintptr_t id);


struct ImageFormatInfo
{
    VkFormat vkFormat;
    uint32_t pixelSize;
};

ImageFormatInfo convertImageFormat(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::RGBA8_UINT:
        return {VK_FORMAT_R8G8B8A8_UINT, 4};
    case ImageFormat::RGBA8_SINT:
        return {VK_FORMAT_R8G8B8A8_SINT, 4};
    case ImageFormat::RGBA8_SRGB:
        return {VK_FORMAT_R8G8B8A8_SRGB, 4};
    case ImageFormat::RGBA16_UINT:
        return {VK_FORMAT_R16G16B16A16_UINT, 8};
    case ImageFormat::RGBA16_SINT:
        return {VK_FORMAT_R16G16B16A16_SINT, 8};
    case ImageFormat::RGBA16_FLOAT:
        return {VK_FORMAT_R16G16B16A16_SFLOAT, 8};
    case ImageFormat::RGBA32_UINT:
        return {VK_FORMAT_R32G32B32A32_UINT, 16};
    case ImageFormat::RGBA32_SINT:
        return {VK_FORMAT_R32G32B32A32_SINT, 16};
    case ImageFormat::RGBA32_FLOAT:
        return {VK_FORMAT_R32G32B32A32_SFLOAT, 16};
    case ImageFormat::RG32_FLOAT:
        return {VK_FORMAT_R32G32_SFLOAT, 8};
    case ImageFormat::D16_UNORM:
        return {VK_FORMAT_D16_UNORM, 2};
    case ImageFormat::D32_FLOAT:
        return {VK_FORMAT_D32_SFLOAT, 4};
    default:
        return {VK_FORMAT_R8G8B8A8_UNORM, 4};
    }
}

VkImageUsageFlags convertImageUsage(ImageUsage usage)
{
    VkImageUsageFlags vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    switch (usage)
    {
    case ImageUsage::SampledImage:
        vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    case ImageUsage::StorageImage:
        vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    case ImageUsage::DepthImage:
        vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        break;
    default:
        break;
    }

    return vkUsage;
}

void incrementImageRefCount(uintptr_t imageID)
{
    if (imageID != 0)
    {
        auto imageHandle = globalImageStorages.acquire_write(imageID);
        imageHandle->refCount++;
    }
}

void decrementImageRefCount(uintptr_t imageID)
{
    if (imageID != 0) 
    {
        bool shouldDestroy = false;

        {
            auto imageHandle = globalImageStorages.acquire_write(imageID);
            --imageHandle->refCount;
            if (imageHandle->refCount == 0) 
            {
                globalHardwareContext.getMainDevice()->resourceManager.destroyImage(*imageHandle);
                shouldDestroy = true;
            }
        }

        if (shouldDestroy) 
        {
            globalImageStorages.deallocate(imageID);
        }
    }
}

HardwareImage::HardwareImage()
    : imageID(std::make_shared<uintptr_t>(0))
{
}

HardwareImage::HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage, int arrayLayers, void *imageData)
{
    const auto [vkFormat, pixelSize] = convertImageFormat(imageFormat);
    const VkImageUsageFlags vkUsage = convertImageUsage(imageUsage);

    imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());

    auto handle = globalImageStorages.acquire_write(*imageID);

    *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(ktm::uvec2(width, height), vkFormat, pixelSize, vkUsage, arrayLayers);
    handle->refCount = 1;

    if (imageData != nullptr)
    {
        HardwareExecutorVulkan tempExecutor;

        auto imageHandle = globalImageStorages.acquire_write(*imageID);

        HardwareBuffer stagingBuffer(imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize, BufferUsage::StorageBuffer, imageData);
        auto bufferHandle = globalBufferStorages.acquire_write(*stagingBuffer.bufferID);

        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle);
        tempExecutor << &copyCmd << tempExecutor.commit();
    }
}

HardwareImage::HardwareImage(const HardwareImage &other)
    : imageID(other.imageID)
{
    incrementImageRefCount(*imageID);
}

HardwareImage::~HardwareImage()
{
    if (imageID)
    {
        decrementImageRefCount(*imageID);
    }
}

HardwareImage &HardwareImage::operator=(const HardwareImage &other)
{
    if (this != &other)
    {
        incrementImageRefCount(*other.imageID);
        decrementImageRefCount(*imageID);
        *(this->imageID) = *(other.imageID);
    }
    return *this;
}

HardwareImage::operator bool() const
{
    if (imageID && *imageID != 0)
    {
        return globalImageStorages.acquire_read(*imageID)->imageHandle != VK_NULL_HANDLE;
    }
    else
    {
        return false;
    }
}

uint32_t HardwareImage::storeDescriptor()
{
    auto imageHandle = globalImageStorages.acquire_read(*imageID);
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(*imageHandle);
}

HardwareImage &HardwareImage::copyFromBuffer(const HardwareBuffer &buffer, HardwareExecutor *executor)
{
    if (!executor || !executor->getExecutorID() || *executor->getExecutorID() == 0)
    {
        return *this;  // 必须提供有效的 executor
    }

    auto imageHandle = globalImageStorages.acquire_write(*imageID);
    auto bufferHandle = globalBufferStorages.acquire_write(*buffer.bufferID);

    HardwareExecutorVulkan *executorImpl = getExecutorImpl(*executor->getExecutorID());
    if (!executorImpl)
    {
        return *this;
    }

    CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle);
    (*executorImpl) << &copyCmd;

    return *this;
}

HardwareImage &HardwareImage::copyFromData(const void *inputData, HardwareExecutor *executor)
{
    if (inputData != nullptr)
    {
        uint64_t bufferSize = 0;
        {
            auto imageHandle = globalImageStorages.acquire_read(*imageID);
            bufferSize = imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize;
        }

        HardwareBuffer stagingBuffer(bufferSize, BufferUsage::StorageBuffer, inputData);

        copyFromBuffer(stagingBuffer, executor);
    }
    return *this;
}