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

// TODO: 图片的字节或者比特大小要校对
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
    : imageID(0) {
    CFW_LOG_TRACE("HardwareImage@{} default constructed, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  imageID.load(std::memory_order_acquire));
}

HardwareImage::HardwareImage(const HardwareImageCreateInfo& createInfo) {
    const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(createInfo.format);
    const VkImageUsageFlags vkUsage = convertImageUsage(createInfo.usage, isCompressed);

    auto const self_image_id = globalImageStorages.allocate();
    imageID.store(self_image_id, std::memory_order_release);

    {
        const auto handle = globalImageStorages.acquire_write(self_image_id);
        *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(
            ktm::uvec2(createInfo.width, createInfo.height),
            vkFormat,
            pixelSize,
            vkUsage,
            createInfo.arrayLayers,
            createInfo.mipLevels);
    }

    if (createInfo.initialData != nullptr) {
        HardwareExecutorVulkan tempExecutor;
        auto const imageHandle = globalImageStorages.acquire_write(self_image_id);
        const HardwareBuffer stagingBuffer(imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize * imageHandle->arrayLayers,
                                           1,
                                           BufferUsage::StorageBuffer,
                                           createInfo.initialData);
        auto const bufferHandle = globalBufferStorages.acquire_write(stagingBuffer.bufferID.load(std::memory_order_acquire));
        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, 0);
        tempExecutor << &copyCmd << tempExecutor.submit();
    }
    CFW_LOG_TRACE("HardwareImage@{} constructed with createInfo, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  self_image_id);
}

HardwareImage::HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage, int arrayLayers, void* imageData) {
    const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(imageFormat);
    const VkImageUsageFlags vkUsage = convertImageUsage(imageUsage, isCompressed);

    auto const self_image_id = globalImageStorages.allocate();
    imageID.store(self_image_id, std::memory_order_release);

    {
        const auto handle = globalImageStorages.acquire_write(self_image_id);
        *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(
            ktm::uvec2(width, height),
            vkFormat,
            pixelSize,
            vkUsage,
            arrayLayers);
    }

    if (imageData != nullptr) {
        HardwareExecutorVulkan tempExecutor;
        auto const imageHandle = globalImageStorages.acquire_write(self_image_id);
        const HardwareBuffer stagingBuffer(imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize * imageHandle->arrayLayers,
                                           1,
                                           BufferUsage::StorageBuffer,
                                           imageData);
        auto const bufferHandle = globalBufferStorages.acquire_write(stagingBuffer.bufferID.load(std::memory_order_acquire));
        CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, 0);
        tempExecutor << &copyCmd << tempExecutor.submit();
    }
    CFW_LOG_TRACE("HardwareImage@{} constructed with params, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  self_image_id);
}

HardwareImage::HardwareImage(const HardwareImage& other) {
    auto const other_image_id = other.imageID.load(std::memory_order_acquire);
    imageID.store(other_image_id, std::memory_order_release);
    if (other_image_id > 0) {
        auto const handle = globalImageStorages.acquire_write(other_image_id);
        incrementImageRefCount(handle);
    }
    CFW_LOG_TRACE("HardwareImage@{} copy constructed from @{}, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  other_image_id);
}

HardwareImage::HardwareImage(HardwareImage&& other) noexcept {
    auto const other_image_id = other.imageID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwareImage@{} move constructed from @{}, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  other_image_id);
    other.imageID.store(0, std::memory_order_release);
    imageID.store(other_image_id, std::memory_order_release);
}

HardwareImage::~HardwareImage() {
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwareImage@{} destructor called, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  self_image_id);
    if (self_image_id > 0) {
        bool destroy = false;
        if (auto const handle = globalImageStorages.acquire_write(self_image_id); decrementImageRefCount(handle)) {
            destroy = true;
        }
        if (destroy) {
            CFW_LOG_TRACE("HardwareImage@{} destroying, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_image_id);
            globalImageStorages.deallocate(self_image_id);
        }
        imageID.store(0, std::memory_order_release);
    }
}

HardwareImage HardwareImage::operator[](const uint32_t index) {
    auto const selfImageId = imageID.load(std::memory_order_acquire);
    if (selfImageId > 0) {
        HardwareImage subImage;
        auto const subImageId = globalImageStorages.allocate();
        subImage.imageID.store(subImageId, std::memory_order_release);

        {
            auto const imageHandle = globalImageStorages.acquire_write(selfImageId);
            auto const subImageHandle = globalImageStorages.acquire_write(subImageId);

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
    if (this == &other) {
        return *this;
    }
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    auto const other_image_id = other.imageID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwareImage@{} copy assigned from @{}, ID: {} -> {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  self_image_id, other_image_id);

    if (self_image_id == 0 && other_image_id == 0) {
        // 都未初始化，直接返回
        CFW_LOG_WARNING("Copying from an uninitialized HardwareImage to an uninitialized HardwareImage.");
        return *this;
    }

    if (self_image_id == other_image_id) {
        // 已经指向同一个资源，无需操作
        return *this;
    }

    if (other_image_id == 0) {
        // 释放自身资源
        bool should_destroy_self = false;
        if (auto const self_handle = globalImageStorages.acquire_write(self_image_id);
            decrementImageRefCount(self_handle) == true) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            CFW_LOG_TRACE("HardwareImage@{} destroying in copy assignment, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_image_id);
            globalImageStorages.deallocate(self_image_id);
        }
        imageID.store(0, std::memory_order_release);
        CFW_LOG_WARNING("Copying from an uninitialized HardwareBuffer.");
        return *this;
    }
    if (self_image_id == 0) {
        // 直接拷贝
        CFW_LOG_DEBUG("HardwareImage@{}: {} assigned from HardwareImage@{}: {}",
                      reinterpret_cast<std::uintptr_t>(this),
                      self_image_id,
                      reinterpret_cast<std::uintptr_t>(&other),
                      other_image_id);
        imageID.store(other_image_id, std::memory_order_release);
        auto const other_handle = globalImageStorages.acquire_write(other_image_id);
        incrementImageRefCount(other_handle);
        return *this;
    }

    bool should_destroy_self = false;
    if (self_image_id < other_image_id) {
        auto const self_handle = globalImageStorages.acquire_write(self_image_id);
        auto const other_handle = globalImageStorages.acquire_write(other_image_id);
        incrementImageRefCount(other_handle);
        if (decrementImageRefCount(self_handle) == true) {
            should_destroy_self = true;
        }
    } else {
        auto const other_handle = globalImageStorages.acquire_write(other_image_id);
        auto const self_handle = globalImageStorages.acquire_write(self_image_id);
        incrementImageRefCount(other_handle);
        if (decrementImageRefCount(self_handle) == true) {
            should_destroy_self = true;
        }
    }
    if (should_destroy_self) {
        CFW_LOG_TRACE("HardwareImage@{} destroying in copy assignment, ID: {}",
                      reinterpret_cast<std::uintptr_t>(this),
                      self_image_id);
        globalImageStorages.deallocate(self_image_id);
    }
    imageID.store(other_image_id, std::memory_order_release);
    return *this;
}

HardwareImage& HardwareImage::operator=(HardwareImage&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    CFW_LOG_TRACE("HardwareImage@{} move assigned from @{}, ID: {} -> {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  imageID.load(std::memory_order_acquire),
                  other.imageID.load(std::memory_order_acquire));
    if (auto const self_image_id = imageID.load(std::memory_order_acquire);
        self_image_id > 0) {
        auto const self_handle = globalImageStorages.acquire_write(self_image_id);
        bool should_destroy_self = false;
        if (decrementImageRefCount(self_handle) == true) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            CFW_LOG_TRACE("HardwareImage@{} destroying in move assignment, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_image_id);
            globalImageStorages.deallocate(self_image_id);
        }
    }
    imageID.store(other.imageID.load(std::memory_order_acquire), std::memory_order_release);
    other.imageID.store(0, std::memory_order_release);
    return *this;
}

HardwareImage::operator bool() const {
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    return self_image_id > 0 &&
           globalImageStorages.acquire_read(self_image_id)->imageHandle != VK_NULL_HANDLE;
}

uint32_t HardwareImage::storeDescriptor() {
    auto imageHandle = globalImageStorages.acquire_write(imageID.load(std::memory_order_acquire));
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(imageHandle);
}

//uint32_t HardwareImage::getMipLevels() const {
//    auto const self_image_id = imageID.load(std::memory_order_acquire);
//    return self_image_id > 0 ? globalImageStorages.acquire_read(self_image_id)->mipLevels : 0;
//}
//
//std::pair<uint32_t, uint32_t> HardwareImage::getMipLevelSize(uint32_t mipLevel) const {
//    if (auto const self_image_id = imageID.load(std::memory_order_acquire);
//        self_image_id > 0) {
//        auto const imageHandle = globalImageStorages.acquire_read(self_image_id);
//        if (mipLevel >= imageHandle->mipLevels) {
//            return {0, 0};
//        }
//        uint32_t width = std::max(1u, imageHandle->imageSize.x >> mipLevel);
//        uint32_t height = std::max(1u, imageHandle->imageSize.y >> mipLevel);
//        return {width, height};
//    }
//    return {0, 0};
//}

HardwareImage& HardwareImage::copyFromBuffer(const HardwareBuffer& buffer, HardwareExecutor* executor, uint32_t mipLevel) {
    if (!executor) {
        return *this;  // 必须提供有效的 executor
    }
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    auto const buffer_id = buffer.bufferID.load(std::memory_order_acquire);
    auto const executor_id = executor->getExecutorID();
    if (self_image_id == 0 || buffer_id == 0 || executor_id == 0) {
        CFW_LOG_WARNING("Copy operation failed due to uninitialized HardwareImage.");
        return *this;
    }
    auto const executor_handle = gExecutorStorage.acquire_write(executor_id);
    auto const bufferHandle = globalBufferStorages.acquire_write(buffer_id);
    auto const imageHandle = globalImageStorages.acquire_write(self_image_id);
    if (mipLevel >= imageHandle->mipLevels) {
        return *this;
    }
    if (!executor_handle->impl) {
        return *this;
    }
    CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, mipLevel);
    *executor_handle->impl << &copyCmd;
    return *this;
}

HardwareImage& HardwareImage::copyFromData(const void* inputData, HardwareExecutor* executor, uint32_t mipLevel) {
    if (inputData == nullptr) {
        return *this;
    }
    uint64_t bufferSize = 0;
    {
        auto const imageHandle = globalImageStorages.acquire_read(imageID.load(std::memory_order_acquire));
        if (mipLevel >= imageHandle->mipLevels) {
            return *this;
        }
        const uint32_t width = std::max(1u, imageHandle->imageSize.x >> mipLevel);
        const uint32_t height = std::max(1u, imageHandle->imageSize.y >> mipLevel);
        bufferSize = width * height * imageHandle->pixelSize;
    }

    return copyFromBuffer(HardwareBuffer(bufferSize, BufferUsage::StorageBuffer, inputData),
                          executor,
                          mipLevel);
}