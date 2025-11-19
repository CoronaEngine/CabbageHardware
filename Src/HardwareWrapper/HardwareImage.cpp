#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareVulkan/GlobalContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"

#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"

HardwareExecutorVulkan *getExecutorImpl(uintptr_t id);

Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap> globalImageStorages;

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
        globalImageStorages.write(imageID, [](ResourceManager::ImageHardwareWrap &image) {
            ++image.refCount;
        });
    }
}

void decrementImageRefCount(uintptr_t imageID)
{
    if (imageID == 0)
    {
        return;
    }

    bool shouldDestroy = false;
    globalImageStorages.write(imageID, [&](ResourceManager::ImageHardwareWrap &image) {
        if (--image.refCount == 0)
        {
            globalHardwareContext.getMainDevice()->resourceManager.destroyImage(image);
            shouldDestroy = true;
        }
    });

    if (shouldDestroy)
    {
        globalImageStorages.deallocate(imageID);
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

    const auto handle = globalImageStorages.allocate([&](ResourceManager::ImageHardwareWrap &image) {
        image = globalHardwareContext.getMainDevice()->resourceManager.createImage(ktm::uvec2(width, height), vkFormat, pixelSize, vkUsage, arrayLayers);
        image.refCount = 1;
    });

    imageID = std::make_shared<uintptr_t>(handle);

    if (imageData != nullptr)
    {
        // 构造函数中需要立即上传数据，创建临时 executor 并立即提交
        HardwareExecutorVulkan tempExecutor;

        uint32_t imgWidth = 0;
        uint32_t imgHeight = 0;
        uint32_t imgPixelSize = 0;

        globalImageStorages.read(*imageID, [&](const ResourceManager::ImageHardwareWrap &image) {
            imgWidth = image.imageSize.x;
            imgHeight = image.imageSize.y;
            imgPixelSize = image.pixelSize;
        });

        const uint64_t bufferSize = static_cast<uint64_t>(imgWidth) * imgHeight * imgPixelSize;
        HardwareBuffer stagingBuffer(bufferSize, BufferUsage::StorageBuffer, imageData);

        ResourceManager::BufferHardwareWrap srcBuffer;
        ResourceManager::ImageHardwareWrap dstImage;

        globalBufferStorages.read(*stagingBuffer.bufferID, [&](const ResourceManager::BufferHardwareWrap &buf) {
            srcBuffer = buf;
        });

        globalImageStorages.read(*imageID, [&](const ResourceManager::ImageHardwareWrap &img) {
            dstImage = img;
        });

        CopyBufferToImageCommand copyCmd(srcBuffer, dstImage);
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
    if (!imageID || *imageID == 0)
    {
        return false;
    }

    bool isValid = false;
    globalImageStorages.read(*imageID, [&](const ResourceManager::ImageHardwareWrap &image) {
        isValid = (image.imageHandle != VK_NULL_HANDLE);
    });

    return isValid;
}

uint32_t HardwareImage::storeDescriptor()
{
    uint32_t index = 0;
    globalImageStorages.read(*imageID, [&](const ResourceManager::ImageHardwareWrap &image) {
        index = globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(image);
    });

    return index;
}

HardwareImage &HardwareImage::copyFromBuffer(const HardwareBuffer &buffer, HardwareExecutor *executor)
{
    if (!executor || !executor->getExecutorID() || *executor->getExecutorID() == 0)
    {
        return *this;  // 必须提供有效的 executor
    }

    ResourceManager::BufferHardwareWrap srcBuffer;
    ResourceManager::ImageHardwareWrap dstImage;

    globalBufferStorages.read(*buffer.bufferID, [&](const ResourceManager::BufferHardwareWrap &buf) {
        srcBuffer = buf;
    });

    globalImageStorages.read(*imageID, [&](const ResourceManager::ImageHardwareWrap &img) {
        dstImage = img;
    });

    HardwareExecutorVulkan *executorImpl = getExecutorImpl(*executor->getExecutorID());
    if (!executorImpl)
    {
        return *this;
    }

    CopyBufferToImageCommand copyCmd(srcBuffer, dstImage);
    (*executorImpl) << &copyCmd;

    return *this;
}

HardwareImage &HardwareImage::copyFromData(const void *inputData, HardwareExecutor *executor)
{
    if (inputData == nullptr)
    {
        return *this;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t pixelSize = 0;

    globalImageStorages.read(*imageID, [&](const ResourceManager::ImageHardwareWrap &image) {
        width = image.imageSize.x;
        height = image.imageSize.y;
        pixelSize = image.pixelSize;
    });

    const uint64_t bufferSize = static_cast<uint64_t>(width) * height * pixelSize;
    HardwareBuffer stagingBuffer(bufferSize, BufferUsage::StorageBuffer, inputData);

    return copyFromBuffer(stagingBuffer, executor);
}