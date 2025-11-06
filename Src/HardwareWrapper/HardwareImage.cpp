#include"CabbageHardware.h"
#include<Hardware/GlobalContext.h>
#include<Hardware/ResourceCommand.h>

Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap> globalImageStorages;

HardwareImage& HardwareImage::operator=(const HardwareImage& other)
{
    auto otherHandle = static_cast<uintptr_t>(*other.imageIdPtr);
    auto thisHandle = static_cast<uintptr_t>(*this->imageIdPtr);

    if (otherHandle != thisHandle)
    {
        bool write_success = globalImageStorages.write(otherHandle, [](ResourceManager::ImageHardwareWrap &image) {
            image.refCount++;
        });

        if (!write_success)
        {
            throw std::runtime_error("Failed to write HardwareBuffer!");
        }
    }
    else
    {
        bool write_success = globalImageStorages.write(thisHandle, [&](ResourceManager::ImageHardwareWrap &image) {
            image.refCount--;
            if (image.refCount == 0)
            {
                globalHardwareContext.mainDevice->resourceManager.destroyImage(image);
                globalImageStorages.deallocate(thisHandle);
            }
        });

        if (!write_success)
        {
            throw std::runtime_error("Failed to write HardwareBuffer!");
        }
    }

    *(this->imageIdPtr) = *(other.imageIdPtr);
    return *this;
}

HardwareImage::HardwareImage()
{
    this->imageIdPtr = std::make_shared<uint64_t>(std::numeric_limits<uint64_t>::max());
}

HardwareImage::HardwareImage(const HardwareImage &other)
{
    this->imageIdPtr = other.imageIdPtr;

    auto otherHandle = static_cast<uintptr_t>(*other.imageIdPtr);
    bool write_success = globalImageStorages.write(otherHandle, [](ResourceManager::ImageHardwareWrap &image) {
        image.refCount++;
    });

    if (!write_success)
    {
        throw std::runtime_error("Failed to write HardwareBuffer!");
    }
}

HardwareImage::~HardwareImage()
{
    auto thisHandle = static_cast<uintptr_t>(*this->imageIdPtr);
    bool write_success = globalImageStorages.write(thisHandle, [&](ResourceManager::ImageHardwareWrap &image) {
        image.refCount--;
        if (image.refCount == 0)
        {
            globalHardwareContext.mainDevice->resourceManager.destroyImage(image);
            globalImageStorages.deallocate(thisHandle);
        }
    });

    if (!write_success)
    {
        throw std::runtime_error("Failed to write HardwareBuffer!");
    }
}

HardwareImage::operator bool()
{
    if (imageIdPtr != nullptr)
    {
        bool read_success = globalImageStorages.read(static_cast<uintptr_t>(*imageIdPtr), [](const ResourceManager::ImageHardwareWrap &image) {
            return image.imageHandle != VK_NULL_HANDLE;
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

uint32_t HardwareImage::storeDescriptor()
{
    ResourceManager::ImageHardwareWrap storeImage;
    bool read_success = globalImageStorages.read(static_cast<uintptr_t>(*imageIdPtr), [&](const ResourceManager::ImageHardwareWrap &image) {
        storeImage = image;
    });

    return globalHardwareContext.mainDevice->resourceManager.storeDescriptor(storeImage);
}

HardwareImage &HardwareImage::copyFromBuffer(const HardwareBuffer &buffer)
{
    HardwareExecutor tempExecutor;

    ResourceManager::BufferHardwareWrap srcBuffer;
    ResourceManager::ImageHardwareWrap dstImage;

    bool read_srcBuffer_success = globalBufferStorages.read(static_cast<uintptr_t>(*buffer.bufferIdPtr), [&](const ResourceManager::BufferHardwareWrap &buffer) {
        srcBuffer = buffer;
    });
    bool read_dstImage_success = globalImageStorages.read(static_cast<uintptr_t>(*imageIdPtr), [&](const ResourceManager::ImageHardwareWrap &image) {
        dstImage = image;
    });

    // 使用栅栏确保提交完成后再返回，避免临时 staging buffer 在 GPU 仍然读取时被析构
    CopyBufferToImageCommand copyCmd(srcBuffer, dstImage);

    //VkFenceCreateInfo fenceInfo{};
    //fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    //fenceInfo.flags = 0; // 未信号状态

    //VkDevice device = globalHardwareContext.mainDevice->deviceManager.logicalDevice;
    //VkFence fence = VK_NULL_HANDLE;
    //if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    //{
    //    // 创建失败则退化为同步 DeviceIdle，保证安全性
    //    tempExecutor << &copyCmd << tempExecutor.commit();
    //    vkDeviceWaitIdle(device);
    //    return *this;
    //}

    tempExecutor << &copyCmd << tempExecutor.commit();

    // 等待 GPU 完成该次提交，确保源 buffer 生命周期安全
    //vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
    //vkDestroyFence(device, fence, nullptr);

    return *this;
}

HardwareImage &HardwareImage::copyFromData(const void *inputData)
{
    ResourceManager::ImageHardwareWrap copyImage;
    bool read_success = globalImageStorages.read(static_cast<uintptr_t>(*imageIdPtr), [&](const ResourceManager::ImageHardwareWrap &image) {
        copyImage = image;
    });

    HardwareBuffer stagingBuffer = HardwareBuffer(copyImage.imageSize.x * copyImage.imageSize.y * copyImage.pixelSize, BufferUsage::StorageBuffer, inputData);
    copyFromBuffer(stagingBuffer);
    return *this;
}


HardwareImage::HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage, int arrayLayers, void *imageData)
{
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

    auto handle = globalImageStorages.allocate([&](ResourceManager::ImageHardwareWrap &image) {
        image = globalHardwareContext.mainDevice->resourceManager.createImage(ktm::uvec2(width, height), vkImageFormat, pixelSize, vkImageUsageFlags, arrayLayers);
        image.refCount = 1;
        if (imageData != nullptr)
        {
            copyFromData(imageData);
        }
    });
}
