#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

struct ImageFormatInfo {
    VkFormat vkFormat;
    uint32_t pixelSize;
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
        case ImageFormat::BC1_RGBA_UNORM:
            return {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, 8, true};
        case ImageFormat::BC3_UNORM:
            return {VK_FORMAT_BC3_UNORM_BLOCK, 16, true};
        case ImageFormat::BC5_UNORM:
            return {VK_FORMAT_BC5_UNORM_BLOCK, 16, true};
        case ImageFormat::BC7_UNORM:
            return {VK_FORMAT_BC7_UNORM_BLOCK, 16, true};
        case ImageFormat::BC7_SRGB:
            return {VK_FORMAT_BC7_SRGB_BLOCK, 16, true};

        // ASTC compressed formats - Mobile
        case ImageFormat::ASTC_4x4_UNORM:
            return {VK_FORMAT_ASTC_4x4_UNORM_BLOCK, 16, true};
        case ImageFormat::ASTC_4x4_SRGB:
            return {VK_FORMAT_ASTC_4x4_SRGB_BLOCK, 16, true};
        case ImageFormat::ASTC_6x6_UNORM:
            return {VK_FORMAT_ASTC_6x6_UNORM_BLOCK, 16, true};
        case ImageFormat::ASTC_8x8_UNORM:
            return {VK_FORMAT_ASTC_8x8_UNORM_BLOCK, 16, true};

        default:
            return {VK_FORMAT_R8G8B8A8_UNORM, 4, false};
    }
}

VkImageUsageFlags convertImageUsage(ImageUsage usage) {
    VkImageUsageFlags vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    switch (usage) {
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

// 计算最大 mipmap 层级数
static uint32_t calculateMaxMipLevels(uint32_t width, uint32_t height) {
    uint32_t maxDimension = std::max(width, height);
    return static_cast<uint32_t>(std::floor(std::log2(maxDimension))) + 1;
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

HardwareImage::HardwareImage(const HardwareImageCreateInfo& createInfo) {
    const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(createInfo.format);
    const VkImageUsageFlags vkUsage = convertImageUsage(createInfo.usage);

    // 如果 mipLevels == 0, 自动计算最大层级
    int mipLevels = createInfo.mipLevels;
    if (mipLevels == 0) {
        mipLevels = calculateMaxMipLevels(createInfo.width, createInfo.height);
    }

    imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());

    {
        const auto handle = globalImageStorages.acquire_write(*imageID);

        *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(
            ktm::uvec2(createInfo.width, createInfo.height),
            vkFormat,
            pixelSize,
            vkUsage,
            createInfo.arrayLayers,
            mipLevels);
        handle->refCount = 1;
    }

    if (createInfo.initialData != nullptr) {
        /*HardwareExecutorVulkan tempExecutor;

        auto imageHandle = globalImageStorages.acquire_write(*imageID);

        HardwareBuffer stagingBuffer(imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize, BufferUsage::StorageBuffer, imageData);
        auto bufferHandle = globalBufferStorages.acquire_write(*stagingBuffer.bufferID);

        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle);
        tempExecutor << &copyCmd << tempExecutor.commit();*/

        copyFromData(createInfo.initialData, nullptr);
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

uint32_t HardwareImage::storeDescriptor(uint32_t mipLevel) {
    auto imageHandle = globalImageStorages.acquire_write(*imageID);

    if (mipLevel >= imageHandle->mipLevels) {
        return 0;  // Invalid mip level
    }

    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(imageHandle, mipLevel);
}

uint32_t HardwareImage::getMipLevels() const {
    if (imageID && *imageID != 0) {
        auto imageHandle = globalImageStorages.acquire_read(*imageID);
        return imageHandle->mipLevels;
    }
    return 0;
}

std::pair<uint32_t, uint32_t> HardwareImage::getMipLevelSize(uint32_t mipLevel) const {
    if (imageID && *imageID != 0) {
        auto imageHandle = globalImageStorages.acquire_read(*imageID);

        if (mipLevel >= imageHandle->mipLevels) {
            return {0, 0};
        }

        uint32_t width = std::max(1u, imageHandle->imageSize.x >> mipLevel);
        uint32_t height = std::max(1u, imageHandle->imageSize.y >> mipLevel);

        return {width, height};
    }
    return {0, 0};
}

HardwareImage& HardwareImage::copyFromBuffer(const HardwareBuffer& buffer, HardwareExecutor* executor) {
    if (!executor || !executor->getExecutorID() || *executor->getExecutorID() == 0) {
        return *this;  // 必须提供有效的 executor
    }

    {
        auto const executor_handle = gExecutorStorage.acquire_write(*executor->getExecutorID());
        if (!executor_handle->impl) {
            return *this;
        }
        if (*imageID < *buffer.bufferID) {
            auto imageHandle = globalImageStorages.acquire_write(*imageID);
            auto bufferHandle = globalBufferStorages.acquire_write(*buffer.bufferID);
            CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle);
            *executor_handle->impl << &copyCmd;
        } else {
            auto bufferHandle = globalBufferStorages.acquire_write(*buffer.bufferID);
            auto imageHandle = globalImageStorages.acquire_write(*imageID);
            CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle);
            *executor_handle->impl << &copyCmd;
        }
    }

    return *this;
}

HardwareImage& HardwareImage::copyFromData(const void* inputData, HardwareExecutor* executor) {
    if (inputData != nullptr) {
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

HardwareImage& HardwareImage::copyFromBuffer(const HardwareBuffer& buffer, HardwareExecutor* executor, uint32_t mipLevel) {
    if (!executor || !executor->getExecutorID() || *executor->getExecutorID() == 0) {
        // 如果没有提供 executor, 创建临时的
        HardwareExecutorVulkan tempExecutor;

        auto imageHandle = globalImageStorages.acquire_write(*imageID);
        if (mipLevel >= imageHandle->mipLevels) {
            return *this;
        }

        auto bufferHandle = globalBufferStorages.acquire_write(*buffer.bufferID);

        // 传递 mipLevel 参数
        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, mipLevel);
        tempExecutor << &copyCmd << tempExecutor.commit();

        return *this;
    }

    {
        auto imageHandle = globalImageStorages.acquire_write(*imageID);
        if (mipLevel >= imageHandle->mipLevels) {
            return *this;
        }
        auto bufferHandle = globalBufferStorages.acquire_write(*buffer.bufferID);
        auto const executor_handle = gExecutorStorage.acquire_write(*executor->getExecutorID());
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