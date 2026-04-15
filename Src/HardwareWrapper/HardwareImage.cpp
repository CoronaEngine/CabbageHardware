#include "CabbageHardware.h"
#include "HardwareCommands.h"
#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#include <algorithm>
#include <limits>

struct ImageFormatInfo
{
    VkFormat vkFormat;
    float pixelSize;
    bool isCompressed;
};

ImageFormatInfo convertImageFormat(ImageFormat format)
{
    switch (format)
    {
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
        return {VK_FORMAT_BC1_RGB_UNORM_BLOCK, 0.5f, true};
    case ImageFormat::BC1_RGB_SRGB:
        return {VK_FORMAT_BC1_RGB_SRGB_BLOCK, 0.5f, true};
    case ImageFormat::BC2_RGBA_UNORM:
        return {VK_FORMAT_BC2_UNORM_BLOCK, 1.0f, true};
    case ImageFormat::BC2_RGBA_SRGB:
        return {VK_FORMAT_BC2_SRGB_BLOCK, 1.0f, true};
    case ImageFormat::BC3_RGBA_UNORM:
        return {VK_FORMAT_BC3_UNORM_BLOCK, 1.0f, true};
    case ImageFormat::BC3_RGBA_SRGB:
        return {VK_FORMAT_BC3_SRGB_BLOCK, 1.0f, true};
    case ImageFormat::BC4_R_UNORM:
        return {VK_FORMAT_BC4_UNORM_BLOCK, 0.5f, true};
    case ImageFormat::BC4_R_SNORM:
        return {VK_FORMAT_BC4_SNORM_BLOCK, 0.5f, true};
    case ImageFormat::BC5_RG_UNORM:
        return {VK_FORMAT_BC5_UNORM_BLOCK, 1.0f, true};
    case ImageFormat::BC5_RG_SNORM:
        return {VK_FORMAT_BC5_SNORM_BLOCK, 1.0f, true};

    // ASTC compressed formats - Mobile
    case ImageFormat::ASTC_4x4_UNORM:
        return {VK_FORMAT_ASTC_4x4_UNORM_BLOCK, 1.0f, true};
    case ImageFormat::ASTC_4x4_SRGB:
        return {VK_FORMAT_ASTC_4x4_SRGB_BLOCK, 1.0f, true};

    default:
        return {VK_FORMAT_R8G8B8A8_UNORM, 4, false};
    }
}

VkImageUsageFlags convertImageUsage(ImageUsage usage, bool isCompressed)
{
    // 所有图像默认支持传输
    VkImageUsageFlags vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    switch (usage)
    {
    case ImageUsage::SampledImage:
        vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        // 压缩纹理不支持作为 Render Target
        if (!isCompressed)
        {
            vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        break;

    case ImageUsage::StorageImage:
        // 修正：压缩纹理不支持 STORAGE_BIT (imageLoad/Store)
        // 如果强行请求 Storage 且是压缩格式，为了安全降级为 Sampled
        if (!isCompressed)
        {
            vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
            vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        else
        {
            // 压缩格式无法作为 Storage，降级以避免 Crash
            vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
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

static void incrementImageRefCount(uint32_t id, const Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap>::WriteHandle &handle)
{
    ++handle->refCount;
    // CFW_LOG_TRACE("HardwareImage ref++: id={}, count={}", id, handle->refCount);
}

static bool decrementImageRefCount(uint32_t id, const Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap>::WriteHandle &handle)
{
    int count = --handle->refCount;
    // CFW_LOG_TRACE("HardwareImage ref--: id={}, count={}", id, count);
    if (count == 0)
    {
        globalHardwareContext.getMainDevice()->resourceManager.destroyImage(*handle);
        // CFW_LOG_TRACE("HardwareImage destroyed: id={}", id);
        return true;
    }
    return false;
}

static uint64_t computeImageMipByteSize(const ResourceManager::ImageHardwareWrap &image, uint32_t imageMip)
{
    if (imageMip >= image.mipLevels)
    {
        return 0;
    }

    const uint32_t width = std::max(1u, image.imageSize.x >> imageMip);
    const uint32_t height = std::max(1u, image.imageSize.y >> imageMip);
    const bool isCompressed = image.pixelSize < 2.0f;

    if (isCompressed)
    {
        constexpr uint32_t blockWidth = 4;
        constexpr uint32_t blockHeight = 4;
        const uint32_t widthInBlocks = (width + blockWidth - 1) / blockWidth;
        const uint32_t heightInBlocks = (height + blockHeight - 1) / blockHeight;
        const uint32_t bytesPerBlock = static_cast<uint32_t>(image.pixelSize * 16.0f);
        return static_cast<uint64_t>(widthInBlocks) * heightInBlocks * bytesPerBlock;
    }

    return static_cast<uint64_t>(width) * height * static_cast<uint32_t>(image.pixelSize);
}

HardwareImage::HardwareImage()
    : imageID(0)
{
}

// HardwareImage::HardwareImage(const HardwareImageCreateInfo& createInfo) {
//     const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(createInfo.format);
//     const VkImageUsageFlags vkUsage = convertImageUsage(createInfo.usage, isCompressed);
//
//     imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());
//
//     {
//         const auto handle = globalImageStorages.acquire_write(*imageID);
//
//         *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(
//             ktm::uvec2(createInfo.width, createInfo.height),
//             vkFormat,
//             pixelSize,
//             vkUsage,
//             createInfo.arrayLayers,
//             createInfo.mipLevels);
//         handle->refCount = 1;
//     }
//
//     if (createInfo.initialData != nullptr) {
//         HardwareExecutorVulkan tempExecutor;
//
//         auto imageHandle = globalImageStorages.acquire_write(*imageID);
//         HardwareBuffer stagingBuffer(imageHandle->imageSize.x * imageHandle->imageSize.y * imageHandle->pixelSize,
//                                      BufferUsage::StorageBuffer,
//                                      createInfo.initialData);
//
//         auto bufferHandle = globalBufferStorages.acquire_write(stagingBuffer.bufferID.load(std::memory_order_acquire));
//
//         CopyBufferToImageCommand copyCmd(*bufferHandle, *imageHandle, 0);
//         tempExecutor << &copyCmd << tempExecutor.commit();
//     }
// }

HardwareImage::HardwareImage(const HardwareImageCreateInfo &createInfo)
{
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
        handle->initialState = createInfo.initialState;
        handle->currentState = createInfo.initialState;
        handle->keepInitialState = createInfo.keepInitialState;
        handle->debugName = createInfo.debugName;
        handle->subresourceStates.assign(static_cast<size_t>(std::max(1, createInfo.arrayLayers)) *
                                             static_cast<size_t>(std::max(1, createInfo.mipLevels)),
                                         createInfo.initialState);
    }

    //CFW_LOG_TRACE("HardwareImage created: id={}", self_image_id);

    if (createInfo.initialData != nullptr)
    {
        const uint8_t *currentSrcDataPtr = static_cast<const uint8_t *>(createInfo.initialData);
        HardwareExecutor executor;

        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        std::vector<uint64_t> mipSizes;
        {
            auto imageHandle = globalImageStorages.acquire_read(self_image_id);
            mipLevels = std::max(1u, imageHandle->mipLevels);
            arrayLayers = std::max(1u, imageHandle->arrayLayers);
            mipSizes.reserve(mipLevels);
            for (uint32_t mip = 0; mip < mipLevels; ++mip)
            {
                mipSizes.push_back(computeImageMipByteSize(*imageHandle, mip));
            }
        }

        for (uint32_t mip = 0; mip < mipLevels; ++mip)
        {
            const uint64_t mipSize = mipSizes[mip];
            if (mipSize == 0 || mipSize > std::numeric_limits<uint32_t>::max())
            {
                CFW_LOG_WARNING("Skipping HardwareImage initialData upload for unsupported mip size.");
                break;
            }

            for (uint32_t layer = 0; layer < arrayLayers; ++layer)
            {
                HardwareBuffer stagingBuffer(static_cast<uint32_t>(mipSize),
                                             1,
                                             BufferUsage::StorageBuffer,
                                             currentSrcDataPtr,
                                             false);
                executor << stagingBuffer.copyTo(*this, 0, layer, mip);
                currentSrcDataPtr += mipSize;
            }
        }

        executor.commit();
        executor.waitForDeferredResources();
    }
}

// TODO : 后面会被弃用
HardwareImage::HardwareImage(uint32_t width, uint32_t height, ImageFormat imageFormat, ImageUsage imageUsage, int arrayLayers, void *imageData)
{
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

    //CFW_LOG_TRACE("HardwareImage created: id={}", self_image_id);

    if (imageData != nullptr)
    {
        uint64_t layerSize = 0;
        uint32_t layerCount = 1;
        {
            auto imageHandle = globalImageStorages.acquire_read(self_image_id);
            layerSize = computeImageMipByteSize(*imageHandle, 0);
            layerCount = std::max(1u, imageHandle->arrayLayers);
        }

        if (layerSize > 0 && layerSize <= std::numeric_limits<uint32_t>::max())
        {
            const uint8_t *currentSrcDataPtr = static_cast<const uint8_t *>(imageData);
            HardwareExecutor executor;
            for (uint32_t layer = 0; layer < layerCount; ++layer)
            {
                HardwareBuffer stagingBuffer(static_cast<uint32_t>(layerSize),
                                             1,
                                             BufferUsage::StorageBuffer,
                                             currentSrcDataPtr,
                                             false);
                executor << stagingBuffer.copyTo(*this, 0, layer, 0);
                currentSrcDataPtr += layerSize;
            }
            executor.commit();
            executor.waitForDeferredResources();
        }
    }
}

HardwareImage::HardwareImage(const HardwareImage &other)
{
    std::lock_guard<std::mutex> lock(other.imageMutex);
    auto const other_image_id = other.imageID.load(std::memory_order_acquire);
    imageID.store(other_image_id, std::memory_order_release);
    if (other_image_id > 0)
    {
        auto const handle = globalImageStorages.acquire_write(other_image_id);
        incrementImageRefCount(other_image_id, handle);
    }
}

HardwareImage::HardwareImage(HardwareImage &&other) noexcept
{
    std::lock_guard<std::mutex> lock(other.imageMutex);
    auto const other_image_id = other.imageID.load(std::memory_order_acquire);
    imageID.store(other_image_id, std::memory_order_release);
    other.imageID.store(0, std::memory_order_release);
}

HardwareImage::~HardwareImage()
{
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    if (self_image_id > 0)
    {
        bool destroy = false;
        if (auto const handle = globalImageStorages.acquire_write(self_image_id); decrementImageRefCount(self_image_id, handle))
        {
            destroy = true;
        }

        if (destroy)
        {
            globalImageStorages.deallocate(self_image_id);
        }
        imageID.store(0, std::memory_order_release);
    }
}

// HardwareImage::HardwareImage(std::shared_ptr<uintptr_t> parentImageID, uint32_t layer, uint32_t mipLevel) {
//     imageID = std::make_shared<uintptr_t>(globalImageStorages.allocate());
//     auto subImageHandle = globalImageStorages.acquire_write(*imageID);
//     auto imageHandle = globalImageStorages.acquire_write(*parentImageID);
//
//     // 基础属性复制
//     subImageHandle->device = imageHandle->device;
//     subImageHandle->resourceManager = imageHandle->resourceManager;
//     subImageHandle->imageFormat = imageHandle->imageFormat;
//     subImageHandle->pixelSize = imageHandle->pixelSize;
//     subImageHandle->imageLayout = imageHandle->imageLayout;
//     subImageHandle->aspectMask = imageHandle->aspectMask;
//     subImageHandle->clearValue = imageHandle->clearValue;
//     subImageHandle->imageUsage = imageHandle->imageUsage;
//     subImageHandle->imageHandle = imageHandle->imageHandle;  // 共享同一个 VkImage
//     subImageHandle->imageAlloc = imageHandle->imageAlloc;
//     subImageHandle->imageAllocInfo = imageHandle->imageAllocInfo;
//     subImageHandle->bindlessIndex = -1;
//     subImageHandle->refCount = 1;
//
//     subImageHandle->imageView = globalHardwareContext.getMainDevice()->resourceManager.createImageView(*imageHandle, layer, mipLevel);
//     subImageHandle->imageSize = ktm::uvec2(std::max(1u, imageHandle->imageSize.x >> mipLevel),
//                                            std::max(1u, imageHandle->imageSize.y >> mipLevel));
//     subImageHandle->arrayLayers = 1;
//     subImageHandle->mipLevels = 1;
// }
//
// HardwareImage HardwareImage::operator[](const uint32_t index) {
//     if (imageID && *imageID != 0) {
//         auto imageHandle = globalImageStorages.acquire_read(*imageID);
//         if (imageHandle->arrayLayers > 1) {
//             // 这是一个多层图像，索引选择一个层
//             return HardwareImage(imageID, index, static_cast<uint32_t>(-1));
//         } else {
//             // 这是一个单层图像（或已经是层视图），索引选择一个 mipmap
//             uint32_t baseLayer = 0;  // 假设基础层为0
//             return HardwareImage(imageID, baseLayer, index);
//         }
//     }
//     return HardwareImage();
// }

HardwareImage HardwareImage::operator[](const uint32_t index)
{
    auto const selfImageId = imageID.load(std::memory_order_acquire);
    if (selfImageId > 0)
    {
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
            subImageHandle->imageHandle = imageHandle->imageHandle; // 共享同一个 VkImage
            subImageHandle->imageAlloc = imageHandle->imageAlloc;
            subImageHandle->imageAllocInfo = imageHandle->imageAllocInfo;
            subImageHandle->sampler = imageHandle->sampler;
            subImageHandle->samplerRef = imageHandle->samplerRef;
            subImageHandle->bindlessIndex = -1;
            subImageHandle->ownsImage = false;
            subImageHandle->ownsImageView = false;
            subImageHandle->parentImageId = selfImageId;
            subImageHandle->currentState = imageHandle->currentState;
            subImageHandle->initialState = imageHandle->initialState;
            subImageHandle->keepInitialState = imageHandle->keepInitialState;
            subImageHandle->subresourceStates = imageHandle->subresourceStates;

            // 直接获取mipmap（单层数组图像）
            if (imageHandle->arrayLayers == 1)
            {
                subImageHandle->imageView = globalHardwareContext.getMainDevice()->resourceManager.createImageView(*imageHandle, 0, index);
                // 设置子视图特定的属性
                subImageHandle->imageSize = ktm::uvec2(std::max(1u, imageHandle->imageSize.x >> index),
                                                       std::max(1u, imageHandle->imageSize.y >> index));
                subImageHandle->arrayLayers = 1;
                subImageHandle->mipLevels = 1;
            }
            // 返回指定 layer 的所有 mipmap（多层数组图像）
            else if (imageHandle->arrayLayers > 1)
            {
                subImageHandle->imageView = globalHardwareContext.getMainDevice()->resourceManager.createImageView(*imageHandle, index, static_cast<uint32_t>(-1));
                subImageHandle->imageSize = imageHandle->imageSize;
                subImageHandle->arrayLayers = 1;
                subImageHandle->mipLevels = imageHandle->mipLevels;
            }
        }
        //CFW_LOG_TRACE("HardwareImage sub-image created: id={}, parent_id={}", subImageId, selfImageId);

        return subImage;
    }
    return HardwareImage();
}

HardwareImage &HardwareImage::operator=(const HardwareImage &other)
{
    if (this == &other)
    {
        return *this;
    }
    std::scoped_lock lock(imageMutex, other.imageMutex);
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    auto const other_image_id = other.imageID.load(std::memory_order_acquire);

    if (self_image_id == 0 && other_image_id == 0)
    {
        return *this;
    }

    if (self_image_id == other_image_id)
    {
        return *this;
    }

    if (other_image_id == 0)
    {
        bool should_destroy_self = false;
        if (auto const self_handle = globalImageStorages.acquire_write(self_image_id);
            decrementImageRefCount(self_image_id, self_handle) == true)
        {
            should_destroy_self = true;
        }
        if (should_destroy_self)
        {
            globalImageStorages.deallocate(self_image_id);
        }
        imageID.store(0, std::memory_order_release);
        return *this;
    }

    if (self_image_id == 0)
    {
        imageID.store(other_image_id, std::memory_order_release);
        auto const other_handle = globalImageStorages.acquire_write(other_image_id);
        incrementImageRefCount(other_image_id, other_handle);
        return *this;
    }

    bool should_destroy_self = false;
    if (self_image_id < other_image_id)
    {
        auto const self_handle = globalImageStorages.acquire_write(self_image_id);
        auto const other_handle = globalImageStorages.acquire_write(other_image_id);
        incrementImageRefCount(other_image_id, other_handle);
        if (decrementImageRefCount(self_image_id, self_handle) == true)
        {
            should_destroy_self = true;
        }
    }
    else
    {
        auto const other_handle = globalImageStorages.acquire_write(other_image_id);
        auto const self_handle = globalImageStorages.acquire_write(self_image_id);
        incrementImageRefCount(other_image_id, other_handle);
        if (decrementImageRefCount(self_image_id, self_handle) == true)
        {
            should_destroy_self = true;
        }
    }
    if (should_destroy_self)
    {
        globalImageStorages.deallocate(self_image_id);
    }
    imageID.store(other_image_id, std::memory_order_release);
    return *this;
}

HardwareImage &HardwareImage::operator=(HardwareImage &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    std::scoped_lock lock(imageMutex, other.imageMutex);
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    auto const other_image_id = other.imageID.load(std::memory_order_acquire);

    if (self_image_id > 0)
    {
        bool should_destroy_self = false;
        if (auto const self_handle = globalImageStorages.acquire_write(self_image_id);
            decrementImageRefCount(self_image_id, self_handle))
        {
            should_destroy_self = true;
        }

        if (should_destroy_self)
        {
            globalImageStorages.deallocate(self_image_id);
        }
    }

    imageID.store(other_image_id, std::memory_order_release);
    other.imageID.store(0, std::memory_order_release);
    return *this;
}

HardwareImage::operator bool() const
{
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    return self_image_id > 0 && globalImageStorages.acquire_read(self_image_id)->imageHandle != VK_NULL_HANDLE;
}

uint32_t HardwareImage::storeDescriptor()
{
    auto imageHandle = globalImageStorages.acquire_write(imageID.load(std::memory_order_acquire));
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(imageHandle);
}

void HardwareImage::setClearColor(float r, float g, float b, float a)
{
    auto const self_image_id = imageID.load(std::memory_order_acquire);
    if (self_image_id == 0) return;
    auto handle = globalImageStorages.acquire_write(self_image_id);
    handle->clearValue.color = {{r, g, b, a}};
}

void HardwareImage::setSampler(const HardwareSampler &sampler)
{
    auto const selfImageId = imageID.load(std::memory_order_acquire);
    if (selfImageId == 0)
    {
        return;
    }

    VkSampler vkSampler = VK_NULL_HANDLE;
    auto const samplerId = sampler.getSamplerID();
    if (samplerId > 0)
    {
        auto samplerHandle = globalSamplerStorages.acquire_read(samplerId);
        vkSampler = samplerHandle->sampler;
    }

    auto imageHandle = globalImageStorages.acquire_write(selfImageId);
    imageHandle->samplerRef = sampler;
    imageHandle->sampler = vkSampler;

    if (imageHandle->bindlessIndex >= 0)
    {
        (void)globalHardwareContext.getMainDevice()->resourceManager.storeDescriptorAt(imageHandle,
                                                                                       static_cast<uint32_t>(imageHandle->bindlessIndex));
    }
}

ImageCopyCommand HardwareImage::copyTo(const HardwareImage &dst,
                                       uint32_t srcLayer, uint32_t dstLayer,
                                       uint32_t srcMip, uint32_t dstMip) const
{
    return ImageCopyCommand(*this, dst, srcLayer, dstLayer, srcMip, dstMip);
}

ImageToBufferCommand HardwareImage::copyTo(const HardwareBuffer &dst,
                                           uint32_t imageLayer,
                                           uint32_t imageMip,
                                           uint64_t bufferOffset) const
{
    return ImageToBufferCommand(*this, dst, imageLayer, imageMip, bufferOffset);
}

// uint32_t HardwareImage::getNumMipLevels() const {
//     if (imageID && *imageID != 0) {
//         auto imageHandle = globalImageStorages.acquire_read(*imageID);
//         return imageHandle->mipLevels;
//     }
//     return 0;
// }

BufferToImageCommand HardwareImage::copyFrom(const void *inputData,
                                             uint32_t imageLayer,
                                             uint32_t imageMip) const
{
    if (inputData == nullptr)
    {
        return BufferToImageCommand();
    }

    uint64_t bufferSize = 0;

    {
        auto const imageHandle = globalImageStorages.acquire_read(imageID.load(std::memory_order_acquire));
        if (imageMip >= imageHandle->mipLevels || imageLayer >= imageHandle->arrayLayers)
        {
            return BufferToImageCommand();
        }
        bufferSize = computeImageMipByteSize(*imageHandle, imageMip);
    }

    if (bufferSize == 0 || bufferSize > std::numeric_limits<uint32_t>::max())
    {
        return BufferToImageCommand();
    }

    HardwareBuffer stagingBuffer(static_cast<uint32_t>(bufferSize), BufferUsage::StorageBuffer, inputData);
    auto cmd = BufferToImageCommand(std::move(stagingBuffer), *this, 0, imageLayer, imageMip);
    return cmd;
}

bool HardwareImage::readback(void *outputData,
                             uint64_t size,
                             uint32_t imageLayer,
                             uint32_t imageMip) const
{
    if (outputData == nullptr || size == 0)
    {
        return false;
    }

    HardwareBuffer stagingBuffer(static_cast<uint32_t>(size), BufferUsage::StorageBuffer, nullptr, false);
    HardwareExecutor executor;
    executor << copyTo(stagingBuffer, imageLayer, imageMip, 0) << executor.commit();
    executor.waitForDeferredResources();
    return stagingBuffer.copyToData(outputData, size);
}
