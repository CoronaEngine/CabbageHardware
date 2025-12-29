#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

struct ImageFormatInfo {
    VkFormat vkFormat;
    float pixelSize;
    bool isCompressed;
};

ImageFormatInfo convertImageFormat(ImageFormat format) {
    switch (format) {
        // Uncompressed formats
        case ImageFormat::RGBA8_UINT:
            return {VK_FORMAT_R8G8B8A8_UINT, 4, false};
        case ImageFormat::RGBA8_SINT:
            return {VK_FORMAT_R8G8B8A8_SINT, 4, false};
        case ImageFormat::RGBA8_SRGB:
            return {VK_FORMAT_R8G8B8A8_SRGB, 4, false};
        case ImageFormat::RGBA16_UINT:
            return {VK_FORMAT_R16G16B16A16_UINT, 8, false};
        case ImageFormat::RGBA16_SINT:
            return {VK_FORMAT_R16G16B16A16_SINT, 8, false};
        case ImageFormat::RGBA16_FLOAT:
            return {VK_FORMAT_R16G16B16A16_SFLOAT, 8, false};
        case ImageFormat::RGBA32_UINT:
            return {VK_FORMAT_R32G32B32A32_UINT, 16, false};
        case ImageFormat::RGBA32_SINT:
            return {VK_FORMAT_R32G32B32A32_SINT, 16, false};
        case ImageFormat::RGBA32_FLOAT:
            return {VK_FORMAT_R32G32B32A32_SFLOAT, 16, false};
        case ImageFormat::RG32_FLOAT:
            return {VK_FORMAT_R32G32_SFLOAT, 8, false};
        case ImageFormat::D16_UNORM:
            return {VK_FORMAT_D16_UNORM, 2, false};
        case ImageFormat::D32_FLOAT:
            return {VK_FORMAT_D32_SFLOAT, 4, false};

        // BC compressed formats - Desktop
        case ImageFormat::BC1_RGB_UNORM:
            return {VK_FORMAT_BC1_RGB_UNORM_BLOCK, 0.5, true};
        case ImageFormat::BC1_RGB_SRGB:
            return {VK_FORMAT_BC1_RGB_SRGB_BLOCK, 0.5, true};
        case ImageFormat::BC2_RGBA_UNORM:
            return {VK_FORMAT_BC2_UNORM_BLOCK, 16, true};
        case ImageFormat::BC2_RGBA_SRGB:
            return {VK_FORMAT_BC2_SRGB_BLOCK, 16, true};
        case ImageFormat::BC3_RGBA_UNORM:
            return {VK_FORMAT_BC3_UNORM_BLOCK, 16, true};
        case ImageFormat::BC3_RGBA_SRGB:
            return {VK_FORMAT_BC3_SRGB_BLOCK, 16, true};
        case ImageFormat::BC4_R_UNORM:
            return {VK_FORMAT_BC4_UNORM_BLOCK, 8, true};
        case ImageFormat::BC4_R_SNORM:
            return {VK_FORMAT_BC4_SNORM_BLOCK, 8, true};
        case ImageFormat::BC5_RG_UNORM:
            return {VK_FORMAT_BC5_UNORM_BLOCK, 16, true};
        case ImageFormat::BC5_RG_SNORM:
            return {VK_FORMAT_BC5_SNORM_BLOCK, 16, true};

        // ASTC compressed formats - Mobile
        case ImageFormat::ASTC_4x4_UNORM:
            return {VK_FORMAT_ASTC_4x4_UNORM_BLOCK, 16, true};
        case ImageFormat::ASTC_4x4_SRGB:
            return {VK_FORMAT_ASTC_4x4_SRGB_BLOCK, 16, true};

        default:
            return {VK_FORMAT_R8G8B8A8_UNORM, 4, false};
    }
}

VkImageUsageFlags convertImageUsage(ImageUsage usage, bool isCompressed) {
    VkImageUsageFlags vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    switch (usage) {
        case ImageUsage::SampledImage:
            vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            // 压缩格式不支持 COLOR_ATTACHMENT_BIT
            if (!isCompressed) {
                vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            break;
        case ImageUsage::StorageImage:
            vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
            // 压缩格式不支持 COLOR_ATTACHMENT_BIT 和 STORAGE_BIT
            if (!isCompressed) {
                vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
            break;
        case ImageUsage::DepthImage:
            vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            break;
        default:
            break;
    }

    return vkUsage;
}

static void incrementImageRefCount(const Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap>::WriteHandle& handle) {
    ++handle->refCount;
}

static bool decrementImageRefCount(const Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap>::WriteHandle& handle) {
    if (--handle->refCount == 0) {
        globalHardwareContext.getMainDevice()->resourceManager.destroyImage(*handle);
        return true;
    }
    return false;
}

HardwareImage::HardwareImage()
    : imageID(std::make_shared<uintptr_t>(0)) {
}

//HardwareImage::HardwareImage(const HardwareImageCreateInfo& createInfo) {
//    const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(createInfo.format);
//    const VkImageUsageFlags vkUsage = convertImageUsage(createInfo.usage, isCompressed);
//
//    imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());
//
//    {
//        const auto handle = globalImageStorages.acquire_write(*imageID);
//
//        *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(
//            ktm::uvec2(createInfo.width, createInfo.height),
//            vkFormat,
//            pixelSize,
//            vkUsage,
//            createInfo.arrayLayers,
//            createInfo.mipLevels);
//        handle->refCount = 1;
//    }
//
//    if (createInfo.initialData != nullptr) {
//        HardwareExecutorVulkan tempExecutor;
//
//        auto imageHandle = globalImageStorages.acquire_write(*imageID);
//        HardwareBuffer stagingBuffer(imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize,
//                                     BufferUsage::StorageBuffer,
//                                     createInfo.initialData);
//
//        auto bufferHandle = globalBufferStorages.acquire_write(stagingBuffer.bufferID.load(std::memory_order_acquire));
//
//        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, 0);
//        tempExecutor << &copyCmd << tempExecutor.commit();
//    }
//}

HardwareImage::HardwareImage(const HardwareImageCreateInfo& createInfo) {
    const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(createInfo.format);
    const VkImageUsageFlags vkUsage = convertImageUsage(createInfo.usage, isCompressed);

    imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());

    {
        const auto handle = globalImageStorages.acquire_write(*imageID);

        *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(
            ktm::uvec2(createInfo.width, createInfo.height),
            vkFormat,
            pixelSize,
            vkUsage,
            createInfo.arrayLayers,
            createInfo.mipLevels);
        handle->refCount = 1;
    }

    if (createInfo.initialData != nullptr) {
        HardwareExecutorVulkan tempExecutor;

        auto imageHandle = globalImageStorages.acquire_write(*imageID);
        HardwareBuffer stagingBuffer(static_cast<uint32_t>(imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize * imageHandle->arrayLayers),
                                     1,
                                     BufferUsage::StorageBuffer,
                                     createInfo.initialData,
                                     false);

        auto bufferHandle = globalBufferStorages.acquire_write(stagingBuffer.getBufferID());

        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, 0);
        tempExecutor << &copyCmd << tempExecutor.commit();
    }
}

HardwareImage::HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage, int arrayLayers, void* imageData) {
    const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(imageFormat);
    const VkImageUsageFlags vkUsage = convertImageUsage(imageUsage, isCompressed);

    imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());

    {
        const auto handle = globalImageStorages.acquire_write(*imageID);

        *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(
            ktm::uvec2(width, height),
            vkFormat,
            pixelSize,
            vkUsage,
            arrayLayers);
        handle->refCount = 1;
    }

    if (imageData != nullptr) {
        HardwareExecutorVulkan tempExecutor;

        auto imageHandle = globalImageStorages.acquire_write(*imageID);
        HardwareBuffer stagingBuffer(imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize,
                                     BufferUsage::StorageBuffer,
                                     imageData);

        auto bufferHandle = globalBufferStorages.acquire_write(stagingBuffer.bufferID.load(std::memory_order_acquire));

        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, 0);
        tempExecutor << &copyCmd << tempExecutor.commit();
    }
}

HardwareImage::HardwareImage(const HardwareImage& other)
    : imageID(other.imageID) {
    if (*imageID > 0) {
        auto const handle = globalImageStorages.acquire_write(*imageID);
        incrementImageRefCount(handle);
    }
}

HardwareImage::~HardwareImage() {
    if (imageID && *imageID > 0) {
        bool destroy = false;
        if (auto const handle = globalImageStorages.acquire_write(*imageID); decrementImageRefCount(handle)) {
            destroy = true;
        }
        if (destroy) {
            globalImageStorages.deallocate(*imageID);
        }
    }
}

//HardwareImage::HardwareImage(std::shared_ptr<uintptr_t> parentImageID, uint32_t layer, uint32_t mipLevel) {
//    imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());
//    auto subImageHandle = globalImageStorages.acquire_write(*imageID);
//    auto imageHandle = globalImageStorages.acquire_write(*parentImageID);
//
//    // 基础属性复制
//    subImageHandle->device = imageHandle->device;
//    subImageHandle->resourceManager = imageHandle->resourceManager;
//    subImageHandle->imageFormat = imageHandle->imageFormat;
//    subImageHandle->pixelSize = imageHandle->pixelSize;
//    subImageHandle->imageLayout = imageHandle->imageLayout;
//    subImageHandle->aspectMask = imageHandle->aspectMask;
//    subImageHandle->clearValue = imageHandle->clearValue;
//    subImageHandle->imageUsage = imageHandle->imageUsage;
//    subImageHandle->imageHandle = imageHandle->imageHandle;  // 共享同一个 VkImage
//    subImageHandle->imageAlloc = imageHandle->imageAlloc;
//    subImageHandle->imageAllocInfo = imageHandle->imageAllocInfo;
//    subImageHandle->bindlessIndex = -1;
//    subImageHandle->refCount = 1;
//
//    subImageHandle->imageView = globalHardwareContext.getMainDevice()->resourceManager.createImageView(*imageHandle, layer, mipLevel);
//    subImageHandle->imageSize = ktm::uvec2(std::max(1u, imageHandle->imageSize.x >> mipLevel),
//                                           std::max(1u, imageHandle->imageSize.y >> mipLevel));
//    subImageHandle->arrayLayers = 1;
//    subImageHandle->mipLevels = 1;
//}
//
//HardwareImage HardwareImage::operator[](const uint32_t index) {
//    if (imageID && *imageID != 0) {
//        auto imageHandle = globalImageStorages.acquire_read(*imageID);
//        if (imageHandle->arrayLayers > 1) {
//            // 这是一个多层图像，索引选择一个层
//            return HardwareImage(imageID, index, static_cast<uint32_t>(-1));
//        } else {
//            // 这是一个单层图像（或已经是层视图），索引选择一个 mipmap
//            uint32_t baseLayer = 0;  // 假设基础层为0
//            return HardwareImage(imageID, baseLayer, index);
//        }
//    }
//    return HardwareImage();
//}

HardwareImage HardwareImage::operator[](const uint32_t index) {
    if (imageID && *imageID != 0) {
        HardwareImage subImage;
        subImage.imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());

        {
            auto imageHandle = globalImageStorages.acquire_write(*imageID);
            auto subImageHandle = globalImageStorages.acquire_write(*subImage.imageID);

            // 基础属性复制
            subImageHandle->device = imageHandle->device;
            subImageHandle->resourceManager = imageHandle->resourceManager;
            subImageHandle->imageFormat = imageHandle->imageFormat;
            subImageHandle->pixelSize = imageHandle->pixelSize;
            subImageHandle->imageLayout = imageHandle->imageLayout;
            subImageHandle->aspectMask = imageHandle->aspectMask;
            subImageHandle->clearValue = imageHandle->clearValue;
            subImageHandle->imageUsage = imageHandle->imageUsage;
            subImageHandle->imageHandle = imageHandle->imageHandle;  // 共享同一个 VkImage
            subImageHandle->imageAlloc = imageHandle->imageAlloc;
            subImageHandle->imageAllocInfo = imageHandle->imageAllocInfo;
            subImageHandle->bindlessIndex = -1;
            subImageHandle->refCount = 1;

            // 直接获取mipmap（单层数组图像）
            if (imageHandle->arrayLayers == 1) {
                subImageHandle->imageView = globalHardwareContext.getMainDevice()->resourceManager.createImageView(*imageHandle, 0, index);
                // 设置子视图特定的属性
                subImageHandle->imageSize = ktm::uvec2(std::max(1u, imageHandle->imageSize.x >> index),
                                                       std::max(1u, imageHandle->imageSize.y >> index));
                subImageHandle->arrayLayers = 1;
                subImageHandle->mipLevels = 1;
            }
            // 返回指定 layer 的所有 mipmap（多层数组图像）
            else if (imageHandle->arrayLayers > 1) {
                subImageHandle->imageView = globalHardwareContext.getMainDevice()->resourceManager.createImageView(*imageHandle, index, static_cast<uint32_t>(-1));
                subImageHandle->imageSize = imageHandle->imageSize;
                subImageHandle->arrayLayers = 1;
                subImageHandle->mipLevels = imageHandle->mipLevels;
            }
        }

        return subImage;
    }
    return HardwareImage();
}

HardwareImage& HardwareImage::operator=(const HardwareImage& other) {
    if (this != &other) {
        {
            auto const handle = globalImageStorages.acquire_write(*other.imageID);
            incrementImageRefCount(handle);
        }
        {
            if (imageID && *imageID > 0) {
                bool destroy = false;
                if (auto const handle = globalImageStorages.acquire_write(*imageID); decrementImageRefCount(handle)) {
                    destroy = true;
                }
                if (destroy) {
                    globalImageStorages.deallocate(*imageID);
                }
            }
        }
        *(this->imageID) = *(other.imageID);
    }
    return *this;
}

HardwareImage::operator bool() const {
    if (imageID && *imageID != 0) {
        return globalImageStorages.acquire_read(*imageID)->imageHandle != VK_NULL_HANDLE;
    } else {
        return false;
    }
}

uint32_t HardwareImage::storeDescriptor() {
    auto imageHandle = globalImageStorages.acquire_write(*imageID);
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(imageHandle);
}

//uint32_t HardwareImage::getNumMipLevels() const {
//    if (imageID && *imageID != 0) {
//        auto imageHandle = globalImageStorages.acquire_read(*imageID);
//        return imageHandle->mipLevels;
//    }
//    return 0;
//}

HardwareImage& HardwareImage::copyFromBuffer(const HardwareBuffer& buffer, HardwareExecutor* executor, uint32_t mipLevel) {
    if (!executor || !executor->getExecutorID() || *executor->getExecutorID() == 0) {
        return *this;  // 必须提供有效的 executor
    }

    auto imageHandle = globalImageStorages.acquire_write(*imageID);
    if (mipLevel >= imageHandle->mipLevels) {
        return *this;
    }

    auto bufferHandle = globalBufferStorages.acquire_write(buffer.bufferID.load(std::memory_order_acquire));

    {
        auto const executor_handle = gExecutorStorage.acquire_read(*executor->getExecutorID());
        if (!executor_handle->impl) {
            return *this;
        }

        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, mipLevel);
        *executor_handle->impl << &copyCmd;
    }

    return *this;
}

HardwareImage& HardwareImage::copyFromData(const void* inputData, HardwareExecutor* executor, uint32_t mipLevel) {
    if (inputData != nullptr) {
        uint64_t bufferSize = 0;
        {
            auto imageHandle = globalImageStorages.acquire_read(*imageID);

            if (mipLevel >= imageHandle->mipLevels) {
                return *this;
            }

            uint32_t width = std::max(1u, imageHandle->imageSize.x >> mipLevel);
            uint32_t height = std::max(1u, imageHandle->imageSize.y >> mipLevel);
            bufferSize = width * height * imageHandle->pixelSize;
        }

        HardwareBuffer stagingBuffer(bufferSize, BufferUsage::StorageBuffer, inputData);
        copyFromBuffer(stagingBuffer, executor, mipLevel);
    }
    return *this;
}