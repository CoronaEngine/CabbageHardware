#include "CabbageHardware.h"
#include "HardwareCommands.h"
#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/HardwareVulkan/ResourceCommand.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace
{
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
    VkImageUsageFlags vkUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    switch (usage)
    {
    case ImageUsage::SampledImage:
        vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (!isCompressed)
        {
            vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        break;
    case ImageUsage::StorageImage:
        if (!isCompressed)
        {
            vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        else
        {
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

uint64_t computeImageMipByteSize(const ResourceManager::ImageHardwareWrap &image, uint32_t imageMip)
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

void uploadInitialImageData(HardwareImage &image, const void *initialData)
{
    if (initialData == nullptr || image.getImageID() == 0)
    {
        return;
    }

    const uint8_t *currentSrcDataPtr = static_cast<const uint8_t *>(initialData);
    HardwareExecutor executor;

    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    std::vector<uint64_t> mipSizes;
    {
        auto imageHandle = globalImageStorages.acquire_read(image.getImageID());
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
            executor << stagingBuffer.copyTo(image, 0, layer, mip);
            currentSrcDataPtr += mipSize;
        }
    }

    executor.commit();
    executor.waitForDeferredResources();
}
} // namespace

HardwareImage::HardwareImage() = default;

HardwareImage::HardwareImage(const HardwareImageCreateInfo &createInfo)
{
    const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(createInfo.format);
    const VkImageUsageFlags vkUsage = convertImageUsage(createInfo.usage, isCompressed);

    imageHandle_ = globalImageStorages.allocate_handle(createInfo.debugName);
    auto const selfImageId = getImageID();

    {
        const auto handle = globalImageStorages.acquire_write(selfImageId);
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

    uploadInitialImageData(*this, createInfo.initialData);
}

HardwareImage::HardwareImage(uint32_t width,
                             uint32_t height,
                             ImageFormat imageFormat,
                             ImageUsage imageUsage,
                             int arrayLayers,
                             void *imageData)
{
    const auto [vkFormat, pixelSize, isCompressed] = convertImageFormat(imageFormat);
    const VkImageUsageFlags vkUsage = convertImageUsage(imageUsage, isCompressed);

    imageHandle_ = globalImageStorages.allocate_handle();
    auto const selfImageId = getImageID();

    {
        const auto handle = globalImageStorages.acquire_write(selfImageId);
        *handle = globalHardwareContext.getMainDevice()->resourceManager.createImage(
            ktm::uvec2(width, height),
            vkFormat,
            pixelSize,
            vkUsage,
            arrayLayers);
    }

    uploadInitialImageData(*this, imageData);
}

HardwareImage::HardwareImage(const HardwareImage &other)
    : imageHandle_(other.imageHandle_)
{
}

HardwareImage::HardwareImage(HardwareImage &&other) noexcept
    : imageHandle_(std::move(other.imageHandle_))
{
}

HardwareImage::~HardwareImage() = default;

HardwareImage HardwareImage::operator[](const uint32_t index)
{
    auto const selfImageId = getImageID();
    if (selfImageId == 0)
    {
        return HardwareImage();
    }

    HardwareImage subImage;
    subImage.imageHandle_ = globalImageStorages.allocate_handle();
    auto const subImageId = subImage.getImageID();

    {
        auto const imageHandle = globalImageStorages.acquire_write(selfImageId);
        auto const subImageHandle = globalImageStorages.acquire_write(subImageId);

        subImageHandle->device = imageHandle->device;
        subImageHandle->resourceManager = imageHandle->resourceManager;
        subImageHandle->imageFormat = imageHandle->imageFormat;
        subImageHandle->pixelSize = imageHandle->pixelSize;
        subImageHandle->imageLayout = imageHandle->imageLayout;
        subImageHandle->aspectMask = imageHandle->aspectMask;
        subImageHandle->clearValue = imageHandle->clearValue;
        subImageHandle->imageUsage = imageHandle->imageUsage;
        subImageHandle->imageHandle = imageHandle->imageHandle;
        subImageHandle->imageAlloc = imageHandle->imageAlloc;
        subImageHandle->imageAllocInfo = imageHandle->imageAllocInfo;
        subImageHandle->sampler = imageHandle->sampler;
        subImageHandle->samplerRef = imageHandle->samplerRef;
        subImageHandle->bindlessIndex = -1;
        subImageHandle->ownsImage = false;
        subImageHandle->ownsImageView = false;
        subImageHandle->parentImageId = selfImageId;
        subImageHandle->parentImageRef = imageHandle_;
        subImageHandle->currentState = imageHandle->currentState;
        subImageHandle->initialState = imageHandle->initialState;
        subImageHandle->keepInitialState = imageHandle->keepInitialState;
        subImageHandle->subresourceStates = imageHandle->subresourceStates;

        if (imageHandle->arrayLayers == 1)
        {
            subImageHandle->imageView = globalHardwareContext.getMainDevice()->resourceManager.createImageView(*imageHandle, 0, index);
            subImageHandle->imageSize = ktm::uvec2(std::max(1u, imageHandle->imageSize.x >> index),
                                                   std::max(1u, imageHandle->imageSize.y >> index));
            subImageHandle->arrayLayers = 1;
            subImageHandle->mipLevels = 1;
        }
        else
        {
            subImageHandle->imageView = globalHardwareContext.getMainDevice()->resourceManager.createImageView(*imageHandle, index, static_cast<uint32_t>(-1));
            subImageHandle->imageSize = imageHandle->imageSize;
            subImageHandle->arrayLayers = 1;
            subImageHandle->mipLevels = imageHandle->mipLevels;
        }
    }

    return subImage;
}

HardwareImage &HardwareImage::operator=(const HardwareImage &other)
{
    if (this != &other)
    {
        imageHandle_ = other.imageHandle_;
    }
    return *this;
}

HardwareImage &HardwareImage::operator=(HardwareImage &&other) noexcept
{
    if (this != &other)
    {
        imageHandle_ = std::move(other.imageHandle_);
    }
    return *this;
}

HardwareImage::operator bool() const
{
    auto const selfImageId = getImageID();
    if (selfImageId == 0 || !imageHandle_.valid())
    {
        return false;
    }

    auto const handle = globalImageStorages.try_acquire_read(selfImageId);
    return handle.valid() && handle->imageHandle != VK_NULL_HANDLE;
}

uint32_t HardwareImage::storeDescriptor()
{
    auto const selfImageId = getImageID();
    if (selfImageId == 0)
    {
        return 0;
    }

    auto imageHandle = globalImageStorages.acquire_write(selfImageId);
    return globalHardwareContext.getMainDevice()->resourceManager.storeDescriptor(imageHandle);
}

void HardwareImage::setClearColor(float r, float g, float b, float a)
{
    auto const selfImageId = getImageID();
    if (selfImageId == 0)
    {
        return;
    }

    auto handle = globalImageStorages.acquire_write(selfImageId);
    handle->clearValue.color = {{r, g, b, a}};
}

void HardwareImage::setSampler(const HardwareSampler &sampler)
{
    auto const selfImageId = getImageID();
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
        (void)globalHardwareContext.getMainDevice()->resourceManager.storeDescriptorAt(
            imageHandle,
            static_cast<uint32_t>(imageHandle->bindlessIndex));
    }
}

ImageCopyCommand HardwareImage::copyTo(const HardwareImage &dst,
                                       uint32_t srcLayer,
                                       uint32_t dstLayer,
                                       uint32_t srcMip,
                                       uint32_t dstMip) const
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

BufferToImageCommand HardwareImage::copyFrom(const void *inputData,
                                             uint32_t imageLayer,
                                             uint32_t imageMip) const
{
    if (inputData == nullptr || getImageID() == 0)
    {
        return BufferToImageCommand();
    }

    uint64_t bufferSize = 0;

    {
        auto const imageHandle = globalImageStorages.acquire_read(getImageID());
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
    return BufferToImageCommand(std::move(stagingBuffer), *this, 0, imageLayer, imageMip);
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
