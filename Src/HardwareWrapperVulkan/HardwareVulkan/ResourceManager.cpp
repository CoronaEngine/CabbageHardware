#include "ResourceManager.h"

#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#define VK_NO_PROTOTYPES
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager() {
    cleanUpResourceManager();
}

void ResourceManager::initResourceManager(DeviceManager& device) {
    this->device = &device;

    createVmaAllocator();
    createTextureSampler();
    createBindlessDescriptorSet();
    createExternalBufferMemoryPool();
}

void ResourceManager::cleanUpResourceManager() {
    if (!device || device->getLogicalDevice() == VK_NULL_HANDLE) {
        return;
    }

    VkDevice logicalDevice = device->getLogicalDevice();

    // 等待设备空闲
    vkDeviceWaitIdle(logicalDevice);

    // 清理描述符相关资源
    for (auto& bindlessDesc : bindlessDescriptors) {
        if (bindlessDesc.descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(logicalDevice, bindlessDesc.descriptorPool, nullptr);
            bindlessDesc.descriptorPool = VK_NULL_HANDLE;
        }

        if (bindlessDesc.descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(logicalDevice, bindlessDesc.descriptorSetLayout, nullptr);
            bindlessDesc.descriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    // 清理纹理采样器
    if (textureSampler != VK_NULL_HANDLE) {
        vkDestroySampler(logicalDevice, textureSampler, nullptr);
        textureSampler = VK_NULL_HANDLE;
    }

    // 清理 VMA 分配器
    if (vmaAllocator != VK_NULL_HANDLE) {
        // vmaDestroyAllocator(vmaAllocator);
        vmaAllocator = VK_NULL_HANDLE;
    }

    // 重置状态
    device = nullptr;
    deviceMemorySize = 0;
    hostSharedMemorySize = 0;
    multiInstanceMemorySize = 0;
}

void ResourceManager::createVmaAllocator() {
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorInfo.physicalDevice = device->getPhysicalDevice();
    allocatorInfo.device = device->getLogicalDevice();
    allocatorInfo.instance = globalHardwareContext.getVulkanInstance();

    VmaAllocatorCreateFlags flags = 0;

    // 启用 Buffer Device Address（Vulkan 1.2+）
    flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

#if _WIN32 || _WIN64
    // Windows 平台启用外部内存支持
    flags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
#endif

    allocatorInfo.flags = flags;

    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;

    coronaHardwareCheck(vmaCreateAllocator(&allocatorInfo, &vmaAllocator));

    const VkPhysicalDeviceMemoryProperties* memoryProperties = nullptr;
    vmaGetMemoryProperties(vmaAllocator, &memoryProperties);

    // 重置内存统计
    deviceMemorySize = 0;
    hostSharedMemorySize = 0;
    multiInstanceMemorySize = 0;

    for (uint32_t heapIndex = 0; heapIndex < memoryProperties->memoryHeapCount; ++heapIndex) {
        const VkMemoryHeap& heap = memoryProperties->memoryHeaps[heapIndex];

        if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            deviceMemorySize += heap.size;
        } else if (heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
            multiInstanceMemorySize += heap.size;
        } else {
            hostSharedMemorySize += heap.size;
        }
    }

#ifdef CABBAGE_ENGINE_DEBUG
    const auto toGB = [](uint64_t bytes) -> double {
        return bytes / 1073741824.0;  // 1024^3
    };

    CFW_LOG_DEBUG(
        "[ResourceManager] VMA Allocator created successfully:\n"
        "  Device Memory:        {:.2f} GB\n"
        "  Host Shared Memory:   {:.2f} GB\n"
        "  Multi-Instance Memory: {:.2f} GB",
        toGB(deviceMemorySize),
        toGB(hostSharedMemorySize),
        toGB(multiInstanceMemorySize));
#endif
}

void ResourceManager::createTextureSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device->getPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(1);

    coronaHardwareCheck(vkCreateSampler(device->getLogicalDevice(), &samplerInfo, nullptr, &textureSampler));
}

void ResourceManager::createBindlessDescriptorSet() {
    VkPhysicalDeviceDescriptorIndexingProperties indexingProperties{};
    indexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT;

    VkPhysicalDeviceProperties2 deviceProperties{};
    deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties.pNext = &indexingProperties;

    vkGetPhysicalDeviceProperties2(device->getPhysicalDevice(), &deviceProperties);

    struct DescriptorTypeConfig {
        VkDescriptorType type;
        std::function<uint32_t(const VkPhysicalDeviceDescriptorIndexingProperties&)> computeMaxCount;
    };

    constexpr uint32_t PREFERRED_MAX_RESOURCES = 10000u;
    const DescriptorTypeConfig configs[4] =
        {
            // Uniform Buffer
            {
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                [](const auto& props) {
                    return std::min({PREFERRED_MAX_RESOURCES,
                                     props.maxUpdateAfterBindDescriptorsInAllPools / 4,
                                     props.maxPerStageUpdateAfterBindResources / 4,
                                     props.maxPerStageDescriptorUpdateAfterBindUniformBuffers,
                                     props.maxDescriptorSetUpdateAfterBindUniformBuffers});
                }},
            // Combined Image Sampler
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                [](const auto& props) {
                    return std::min({PREFERRED_MAX_RESOURCES,
                                     props.maxUpdateAfterBindDescriptorsInAllPools / 4,
                                     props.maxPerStageUpdateAfterBindResources / 4,
                                     props.maxPerStageDescriptorUpdateAfterBindSampledImages,
                                     props.maxDescriptorSetUpdateAfterBindSampledImages});
                }},
            // Storage Buffer
            {
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                [](const auto& props) {
                    return std::min({PREFERRED_MAX_RESOURCES,
                                     props.maxUpdateAfterBindDescriptorsInAllPools / 4,
                                     props.maxPerStageUpdateAfterBindResources / 4,
                                     props.maxPerStageDescriptorUpdateAfterBindStorageBuffers,
                                     props.maxDescriptorSetUpdateAfterBindStorageBuffers});
                }},
            // Storage Image
            {
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                [](const auto& props) {
                    return std::min({PREFERRED_MAX_RESOURCES,
                                     props.maxUpdateAfterBindDescriptorsInAllPools / 4,
                                     props.maxPerStageUpdateAfterBindResources / 4,
                                     props.maxPerStageDescriptorUpdateAfterBindStorageImages,
                                     props.maxDescriptorSetUpdateAfterBindStorageImages});
                }}};

    std::array<uint32_t, 4> maxResourceCounts;
    for (size_t i = 0; i < 4; ++i) {
        maxResourceCounts[i] = configs[i].computeMaxCount(indexingProperties);
    }

    constexpr VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDevice logicalDevice = device->getLogicalDevice();

    for (size_t i = 0; i < 4; ++i) {
        const uint32_t maxCount = maxResourceCounts[i];
        const VkDescriptorType descriptorType = configs[i].type;

        // 创建描述符集布局
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = descriptorType;
        binding.descriptorCount = maxCount;
        binding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.bindingCount = 1;
        bindingFlagsInfo.pBindingFlags = &bindingFlags;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;
        layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        layoutInfo.pNext = &bindingFlagsInfo;

        coronaHardwareCheck(vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &bindlessDescriptors[i].descriptorSetLayout));

        // 创建描述符池
        VkDescriptorPoolSize poolSize{};
        poolSize.type = descriptorType;
        poolSize.descriptorCount = maxCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;  // 每个池只需要一个描述符集
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

        coronaHardwareCheck(vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &bindlessDescriptors[i].descriptorPool));

        // 分配描述符集（支持变长）
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
        variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
        variableCountInfo.descriptorSetCount = 1;
        variableCountInfo.pDescriptorCounts = &maxCount;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = bindlessDescriptors[i].descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &bindlessDescriptors[i].descriptorSetLayout;
        allocInfo.pNext = &variableCountInfo;

        coronaHardwareCheck(vkAllocateDescriptorSets(logicalDevice, &allocInfo, &bindlessDescriptors[i].descriptorSet));
    }
}

void ResourceManager::createExternalBufferMemoryPool() {
#if _WIN32 || _WIN64
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif __linux__
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#elif __APPLE__
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = 0;
    // macOS 不支持外部内存，但保留代码结构
#endif

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = 0x10000;  // 64KB 测试大小
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    // 配置队列族共享模式
    std::vector<uint32_t> queueFamilyIndices;
    const uint32_t queueFamilyCount = device->getQueueFamilyNumber();

    if (queueFamilyCount > 1) {
        // 多队列族：使用并发模式，允许跨队列共享
        queueFamilyIndices.resize(queueFamilyCount);
        std::iota(queueFamilyIndices.begin(), queueFamilyIndices.end(), 0u);

        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        // 单队列族：使用独占模式以获得更好的性能
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // 配置外部内存支持
    VkExternalMemoryBufferCreateInfo externalMemoryInfo{};
    externalMemoryInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR;
    externalMemoryInfo.handleTypes = EXTERNAL_MEMORY_HANDLE_TYPE;
    bufferInfo.pNext = &externalMemoryInfo;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
    coronaHardwareCheck(vmaFindMemoryTypeIndexForBufferInfo(vmaAllocator, &bufferInfo, &allocInfo, &memoryTypeIndex));

    VkExportMemoryAllocateInfo exportMemoryInfo{};
    exportMemoryInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
    exportMemoryInfo.handleTypes = EXTERNAL_MEMORY_HANDLE_TYPE;

    VmaPoolCreateInfo poolInfo{};
    poolInfo.memoryTypeIndex = memoryTypeIndex;
    poolInfo.pMemoryAllocateNext = &exportMemoryInfo;
    poolInfo.blockSize = 0;      // 使用 VMA 默认块大小（通常 256MB）
    poolInfo.minBlockCount = 1;  // 至少预分配一个块
    poolInfo.maxBlockCount = 0;  // 无限制，按需动态增长
    // poolInfo.flags = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT;

    coronaHardwareCheck(vmaCreatePool(vmaAllocator, &poolInfo, &exportBufferPool));

    CFW_LOG_DEBUG(
        "[ResourceManager] External memory pool created:\n"
        "  Memory Type Index: {}\n"
        "  Handle Type: 0x{:X}\n"
        "  Queue Sharing Mode: {}\n"
        "  Queue Family Count: {}",
        memoryTypeIndex,
        EXTERNAL_MEMORY_HANDLE_TYPE,
        (bufferInfo.sharingMode == VK_SHARING_MODE_CONCURRENT ? "CONCURRENT" : "EXCLUSIVE"),
        queueFamilyCount);
}

ResourceManager::ImageHardwareWrap ResourceManager::createImage(ktm::uvec2 imageSize,
                                                                VkFormat imageFormat,
                                                                float pixelSize,
                                                                VkImageUsageFlags imageUsage,
                                                                int arrayLayers,
                                                                int mipLevels,
                                                                VkImageTiling tiling) {
    ImageHardwareWrap resultImage{};
    resultImage.device = device;
    resultImage.resourceManager = this;
    resultImage.imageSize = imageSize;
    resultImage.imageFormat = imageFormat;
    resultImage.pixelSize = pixelSize;
    resultImage.arrayLayers = arrayLayers;
    resultImage.mipLevels = mipLevels;
    resultImage.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (mipLevels > 1) {
        resultImage.mipLevelViews.resize(mipLevels, VK_NULL_HANDLE);
        resultImage.mipLevelBindlessIndices.resize(mipLevels, -1);
    }

    if (imageUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        // 深度模板图像
        resultImage.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        resultImage.clearValue.depthStencil = {1.0f, 0};
    } else {
        // 颜色图像
        resultImage.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        resultImage.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    }

    // 确保包含基本传输和采样用途
    imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                  VK_IMAGE_USAGE_SAMPLED_BIT;
    resultImage.imageUsage = imageUsage;

    if (imageSize.x == 0 || imageSize.y == 0) {
        // 无效尺寸，返回空图像
        return resultImage;
    }

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = {imageSize.x, imageSize.y, 1};
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = arrayLayers;
    imageInfo.format = imageFormat;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = imageUsage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    // 配置队列族共享模式
    std::vector<uint32_t> queueFamilyIndices;
    const uint32_t queueFamilyCount = device->getQueueFamilyNumber();

    if (queueFamilyCount > 1) {
        // 多队列族：使用并发模式
        queueFamilyIndices.resize(queueFamilyCount);
        std::iota(queueFamilyIndices.begin(), queueFamilyIndices.end(), 0u);

        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        imageInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        // 单队列族：使用独占模式以获得更好性能
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;  // 优先使用设备本地内存

    coronaHardwareCheck(vmaCreateImage(vmaAllocator, &imageInfo, &allocInfo, &resultImage.imageHandle, &resultImage.imageAlloc, &resultImage.imageAllocInfo));

    // 创建图像视图
    resultImage.imageView = createImageView(resultImage);

    const uint64_t imageMemorySize = static_cast<uint64_t>(imageSize.x) *
                                     imageSize.y *
                                     pixelSize *
                                     arrayLayers *
                                     mipLevels;
    deviceMemorySize += imageMemorySize;

    CFW_LOG_DEBUG("Image created: {}x{} Format: 0x{:X} Layers: {} Mips: {} Size: {:.2f} MB",
                  imageSize.x, imageSize.y, static_cast<uint32_t>(imageFormat), arrayLayers, mipLevels, (imageMemorySize / 1024.0 / 1024.0));

    return resultImage;
}

VkImageView ResourceManager::createImageView(ImageHardwareWrap& image) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.imageHandle;
    viewInfo.viewType = image.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = image.imageFormat;
    viewInfo.subresourceRange.aspectMask = image.aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = image.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = image.arrayLayers;
    viewInfo.flags = 0;

    VkImageView imageView;
    coronaHardwareCheck(vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &imageView));

    return imageView;
}

VkImageView ResourceManager::createImageViewForMipLevel(ImageHardwareWrap& image, uint32_t mipLevel) {
    // 验证 mipLevel 有效性
    if (mipLevel >= image.mipLevels) {
        throw std::runtime_error("Invalid mip level: " + std::to_string(mipLevel) +
                                 " (max: " + std::to_string(image.mipLevels - 1) + ")");
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.imageHandle;
    viewInfo.viewType = image.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = image.imageFormat;
    viewInfo.subresourceRange.aspectMask = image.aspectMask;
    viewInfo.subresourceRange.baseMipLevel = mipLevel;  // 指定特定的 mip level
    viewInfo.subresourceRange.levelCount = 1;           // 只包含一个层级
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = image.arrayLayers;
    viewInfo.flags = 0;

    VkImageView imageView;
    coronaHardwareCheck(vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &imageView));

    CFW_LOG_DEBUG("[ResourceManager] Created ImageView for mip level {} (size: {}x{})",
                  mipLevel, (image.imageSize.x >> mipLevel), (image.imageSize.y >> mipLevel));

    return imageView;
}

void ResourceManager::destroyImage(ImageHardwareWrap& image) {
    if (vmaAllocator == VK_NULL_HANDLE) {
        return;
    }

    VkDevice logicalDevice = device->getLogicalDevice();

    // 清理 mip level views
    for (auto& mipView : image.mipLevelViews) {
        if (mipView != VK_NULL_HANDLE) {
            vkDestroyImageView(logicalDevice, mipView, nullptr);
        }
    }
    image.mipLevelViews.clear();
    image.mipLevelBindlessIndices.clear();

    // 清理主 ImageView
    if (image.imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(logicalDevice, image.imageView, nullptr);
        image.imageView = VK_NULL_HANDLE;
    }

    // 销毁图像
    if (image.imageHandle != VK_NULL_HANDLE) {
        vmaDestroyImage(vmaAllocator, image.imageHandle, image.imageAlloc);
        image.imageHandle = VK_NULL_HANDLE;
        image.imageAlloc = VK_NULL_HANDLE;
    }
}

ResourceManager::BufferHardwareWrap ResourceManager::createBuffer(uint32_t elementCount,
                                                                  uint32_t elementSize,
                                                                  VkBufferUsageFlags usage,
                                                                  bool hostVisibleMapped,
                                                                  bool useDedicated) {
    BufferHardwareWrap resultBuffer{};
    resultBuffer.device = device;
    resultBuffer.resourceManager = this;
    resultBuffer.elementCount = elementCount;
    resultBuffer.elementSize = elementSize;
    resultBuffer.bufferUsage = usage;

    const uint64_t totalSize = static_cast<uint64_t>(elementCount) * elementSize;
    if (totalSize == 0) {
        return resultBuffer;
    }

#if _WIN32 || _WIN64
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif __linux__
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#elif __APPLE__
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = 0;
#endif

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = totalSize;
    bufferInfo.usage = usage;

    // 配置队列族共享模式
    std::vector<uint32_t> queueFamilyIndices;
    const uint32_t queueFamilyCount = device->getQueueFamilyNumber();

    if (queueFamilyCount > 1) {
        queueFamilyIndices.resize(queueFamilyCount);
        std::iota(queueFamilyIndices.begin(), queueFamilyIndices.end(), 0u);

        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // 配置外部内存支持
    VkExternalMemoryBufferCreateInfo externalMemoryInfo{};
    externalMemoryInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR;
    externalMemoryInfo.handleTypes = EXTERNAL_MEMORY_HANDLE_TYPE;
    bufferInfo.pNext = &externalMemoryInfo;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (hostVisibleMapped) {
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    if (useDedicated) {
        // 强制使用专用内存
        createDedicatedBuffer(bufferInfo, allocInfo, resultBuffer);
    } else {
        // 查询外部缓冲区属性
        VkPhysicalDeviceExternalBufferInfo externalBufferInfo{};
        externalBufferInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
        externalBufferInfo.flags = bufferInfo.flags;
        externalBufferInfo.usage = bufferInfo.usage;
        externalBufferInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

        VkExternalBufferProperties externalBufferProperties{};
        externalBufferProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;

        vkGetPhysicalDeviceExternalBufferProperties(device->getPhysicalDevice(), &externalBufferInfo, &externalBufferProperties);

        const auto features = externalBufferProperties.externalMemoryProperties.externalMemoryFeatures;
        const bool exportable = (features & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) != 0;
        const bool importable = (features & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
        const bool dedicatedOnly = (features & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;

        if (!exportable || !importable) {
            // 回退到非导出缓冲区
            createNonExportableBuffer(bufferInfo, allocInfo, resultBuffer);
        } else if (dedicatedOnly) {
            // 需要专用内存
            createDedicatedBuffer(bufferInfo, allocInfo, resultBuffer);
        } else {
            // 使用内存池
            createPooledBuffer(bufferInfo, allocInfo, resultBuffer);
        }
    }

    return resultBuffer;
}

void ResourceManager::createDedicatedBuffer(const VkBufferCreateInfo& bufferInfo,
                                            const VmaAllocationCreateInfo& allocInfo,
                                            BufferHardwareWrap& resultBuffer) {
#if _WIN32 || _WIN64
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif __linux__
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#elif __APPLE__
    constexpr VkExternalMemoryHandleTypeFlagsKHR EXTERNAL_MEMORY_HANDLE_TYPE = 0;
#endif

    VkExportMemoryAllocateInfo exportInfo{};
    exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    exportInfo.handleTypes = EXTERNAL_MEMORY_HANDLE_TYPE;

    VmaAllocationCreateInfo dedicatedAllocInfo = allocInfo;
    dedicatedAllocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    coronaHardwareCheck(vmaCreateDedicatedBuffer(vmaAllocator,
                                                 &bufferInfo,
                                                 &dedicatedAllocInfo,
                                                 &exportInfo,
                                                 &resultBuffer.bufferHandle,
                                                 &resultBuffer.bufferAlloc,
                                                 &resultBuffer.bufferAllocInfo));
}

void ResourceManager::createPooledBuffer(const VkBufferCreateInfo& bufferInfo,
                                         const VmaAllocationCreateInfo& allocInfo,
                                         BufferHardwareWrap& resultBuffer) {
    VmaAllocationCreateInfo pooledAllocInfo = allocInfo;
    pooledAllocInfo.pool = exportBufferPool;

    coronaHardwareCheck(vmaCreateBuffer(vmaAllocator, &bufferInfo, &pooledAllocInfo, &resultBuffer.bufferHandle, &resultBuffer.bufferAlloc, &resultBuffer.bufferAllocInfo));
}

void ResourceManager::createNonExportableBuffer(const VkBufferCreateInfo& bufferInfo,
                                                const VmaAllocationCreateInfo& allocInfo,
                                                BufferHardwareWrap& resultBuffer) {
    coronaHardwareCheck(vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocInfo, &resultBuffer.bufferHandle, &resultBuffer.bufferAlloc, &resultBuffer.bufferAllocInfo));
}

void ResourceManager::destroyBuffer(BufferHardwareWrap& buffer) {
    if (buffer.bufferHandle != VK_NULL_HANDLE && vmaAllocator != VK_NULL_HANDLE) {
        // TODO: 考虑异步销毁以避免阻塞
        vkDeviceWaitIdle(device->getLogicalDevice());
        vmaDestroyBuffer(vmaAllocator, buffer.bufferHandle, buffer.bufferAlloc);

        buffer.bufferHandle = VK_NULL_HANDLE;
        buffer.bufferAlloc = VK_NULL_HANDLE;
        buffer.elementCount = 0;
    }
}

ResourceManager::ExternalMemoryHandle ResourceManager::exportBufferMemory(BufferHardwareWrap& sourceBuffer) {
    // 验证源缓冲区有效性
    if (sourceBuffer.bufferHandle == VK_NULL_HANDLE || sourceBuffer.bufferAlloc == VK_NULL_HANDLE) {
        throw std::runtime_error(
            "Cannot export memory from invalid buffer: "
            "buffer handle or allocation is null");
    }

    ExternalMemoryHandle memHandle{};

#if _WIN32 || _WIN64
    // Windows 平台：导出为 Win32 句柄
    coronaHardwareCheck(vmaGetMemoryWin32Handle2(vmaAllocator,
                                                 sourceBuffer.bufferAlloc,
                                                 VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT,
                                                 nullptr,
                                                 &memHandle.handle));

    if (memHandle.handle == nullptr || memHandle.handle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error(
            "Exported memory handle is invalid: "
            "received null or invalid handle from VMA");
    }
#endif

    return memHandle;
}

ResourceManager::BufferHardwareWrap ResourceManager::importBufferMemory(const ExternalMemoryHandle& memHandle,
                                                                        uint32_t elementCount,
                                                                        uint32_t elementSize,
                                                                        uint32_t allocSize,
                                                                        VkBufferUsageFlags bufferUsage) {
#if _WIN32 || _WIN64
    if (memHandle.handle == nullptr || memHandle.handle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Cannot import buffer with invalid memory handle!");
    }
#endif

#if _WIN32 || _WIN64
    constexpr VkExternalMemoryHandleTypeFlagBits EXTERNAL_MEMORY_HANDLE_TYPE =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    // 其他平台略
    constexpr VkExternalMemoryHandleTypeFlagBits EXTERNAL_MEMORY_HANDLE_TYPE =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

    BufferHardwareWrap importedBuffer{};
    importedBuffer.device = device;
    importedBuffer.resourceManager = this;
    importedBuffer.bufferUsage = bufferUsage;
    importedBuffer.elementCount = elementCount;
    importedBuffer.elementSize = elementSize;

    // allocSize 必须等于 CUDA 导出时的 aligned_size
    if (allocSize == 0) {
        throw std::runtime_error("Import failed: allocSize == 0.");
    }
    const uint64_t logicalSize = static_cast<uint64_t>(elementCount) * elementSize;
    if (logicalSize > allocSize) {
        // 逻辑数据大小不应超过物理分配大小
        throw std::runtime_error("Import failed: logical data size exceeds external allocation size.");
    }

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = allocSize;
    bufferInfo.usage = bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // 简化：独占更安全
    bufferInfo.pNext = nullptr;

    VkExternalMemoryBufferCreateInfo externalMemoryInfo{};
    externalMemoryInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    externalMemoryInfo.handleTypes = EXTERNAL_MEMORY_HANDLE_TYPE;
    externalMemoryInfo.pNext = nullptr;
    bufferInfo.pNext = &externalMemoryInfo;

    // 查询外部缓冲区特性
    VkPhysicalDeviceExternalBufferInfo extBufInfo{};
    extBufInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
    extBufInfo.usage = bufferInfo.usage;
    extBufInfo.handleType = EXTERNAL_MEMORY_HANDLE_TYPE;

    VkExternalBufferProperties extProps{};
    extProps.sType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;
    vkGetPhysicalDeviceExternalBufferProperties(device->getPhysicalDevice(), &extBufInfo, &extProps);

    auto feats = extProps.externalMemoryProperties.externalMemoryFeatures;
    const bool importable = (feats & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
    const bool dedicatedOnly = (feats & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;

    if (!importable) {
        throw std::runtime_error("Import failed: buffer not importable (features=" + std::to_string(feats) + ").");
    }

#if _WIN32 || _WIN64
    VkImportMemoryWin32HandleInfoKHR importInfo{};
    importInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
    importInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    importInfo.handle = memHandle.handle;
    importInfo.name = nullptr;
#endif

    // 使用专用 + 设备本地，不再请求映射
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;  // dedicated
    // 不添加 MAPPED / HOST flags，避免要求 Host 可见
    // 如果 extProps 表示需要 dedicatedOnly，则本路径满足

    coronaHardwareCheck(
        vmaCreateDedicatedBuffer(vmaAllocator,
                                 &bufferInfo,
                                 &allocInfo,
                                 &importInfo,
                                 &importedBuffer.bufferHandle,
                                 &importedBuffer.bufferAlloc,
                                 &importedBuffer.bufferAllocInfo));

    // 覆盖 size 以便后续 copy 时使用逻辑大小
    importedBuffer.bufferAllocInfo.size = allocSize;

    CFW_LOG_DEBUG(
        "[Import] External CUDA buffer imported. "
        "logicalSize={} allocSize={} dedicatedOnly={} feats=0x{:X}",
        logicalSize,
        allocSize,
        dedicatedOnly,
        feats);

    return importedBuffer;
}

ResourceManager::BufferHardwareWrap ResourceManager::importHostBuffer(void* hostPtr, uint64_t size) {
    if (hostPtr == nullptr) {
        throw std::invalid_argument("Cannot import host buffer: host pointer is null");
    }

    if (size == 0) {
        throw std::invalid_argument("Cannot import host buffer: size is zero");
    }

    BufferHardwareWrap bufferWrap{};
    bufferWrap.device = device;
    bufferWrap.resourceManager = this;
    bufferWrap.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkMemoryHostPointerPropertiesEXT hostPointerProps{};
    hostPointerProps.sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT;

    VkResult result = vkGetMemoryHostPointerPropertiesEXT(device->getLogicalDevice(),
                                                          VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT,
                                                          hostPtr,
                                                          &hostPointerProps);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to query host pointer properties: " + std::to_string(result) + ". The host pointer may not be compatible with this device.");
    }

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = bufferWrap.bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImportMemoryHostPointerInfoEXT importInfo{};
    importInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
    importInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    importInfo.pHostPointer = hostPtr;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT |
                      VMA_ALLOCATION_CREATE_MAPPED_BIT |
                      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    // 指定允许的内存类型（基于查询到的主机指针属性）
    allocInfo.requiredFlags = 0;
    allocInfo.preferredFlags = 0;
    allocInfo.memoryTypeBits = hostPointerProps.memoryTypeBits;

    coronaHardwareCheck(vmaCreateDedicatedBuffer(vmaAllocator,
                                                 &bufferInfo,
                                                 &allocInfo,
                                                 &importInfo,
                                                 &bufferWrap.bufferHandle,
                                                 &bufferWrap.bufferAlloc,
                                                 &bufferWrap.bufferAllocInfo));

    bufferWrap.bufferAllocInfo.size = size;
    return bufferWrap;
}

int32_t ResourceManager::storeDescriptor(Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap>::WriteHandle& image) {
    if (image->bindlessIndex < 0) {
        image->bindlessIndex = globalImageStorages.seq_id(image);

        VkDescriptorType descriptorType = (image->imageUsage & VK_IMAGE_USAGE_STORAGE_BIT) ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageInfo.imageView = image->imageView;
        imageInfo.sampler = textureSampler;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = descriptorType;
        write.descriptorCount = 1;
        write.dstArrayElement = image->bindlessIndex;
        write.pImageInfo = &imageInfo;

        if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            write.dstSet = bindlessDescriptors[textureBinding].descriptorSet;
            write.dstBinding = 0;
        }
        if (write.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
            write.dstSet = bindlessDescriptors[storageImageBinding].descriptorSet;
            write.dstBinding = 0;
        }

        vkUpdateDescriptorSets(device->getLogicalDevice(), 1, &write, 0, nullptr);
    }

    return image->bindlessIndex;
}

int32_t ResourceManager::storeDescriptor(Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap>::WriteHandle& image, uint32_t mipLevel) {
    if (mipLevel >= image->mipLevels) {
        return -1;
    }

    // 确保 vectors 的大小
    if (image->mipLevelViews.size() < image->mipLevels) {
        image->mipLevelViews.resize(image->mipLevels, VK_NULL_HANDLE);
        image->mipLevelBindlessIndices.resize(image->mipLevels, -1);
    }

    // 如果已经创建过，直接返回
    if (image->mipLevelBindlessIndices[mipLevel] >= 0) {
        return image->mipLevelBindlessIndices[mipLevel];
    }

    // 创建 mip level 专用的 ImageView
    if (image->mipLevelViews[mipLevel] == VK_NULL_HANDLE) {
        image->mipLevelViews[mipLevel] = createImageViewForMipLevel(*image, mipLevel);
    }

    // 生成 bindless index
    int32_t bindlessIndex = globalImageStorages.seq_id(image) * image->mipLevels + mipLevel;

    VkDescriptorType descriptorType = (image->imageUsage & VK_IMAGE_USAGE_STORAGE_BIT)
                                          ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                                          : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = image->mipLevelViews[mipLevel];
    imageInfo.sampler = textureSampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = descriptorType;
    write.descriptorCount = 1;
    write.dstArrayElement = bindlessIndex;
    write.pImageInfo = &imageInfo;

    if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        write.dstSet = bindlessDescriptors[textureBinding].descriptorSet;
        write.dstBinding = 0;
    }
    if (write.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
        write.dstSet = bindlessDescriptors[storageImageBinding].descriptorSet;
        write.dstBinding = 0;
    }

    vkUpdateDescriptorSets(device->getLogicalDevice(), 1, &write, 0, nullptr);

    // 保存 bindless index
    image->mipLevelBindlessIndices[mipLevel] = bindlessIndex;

    return bindlessIndex;
}

int32_t ResourceManager::storeDescriptor(Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap>::WriteHandle& buffer) {
    if (buffer->bindlessIndex < 0) {
        buffer->bindlessIndex = globalBufferStorages.seq_id(buffer);

        VkDescriptorType descriptorType = (buffer->bufferUsage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer->bufferHandle;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet writes{};
        writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes.descriptorCount = 1;
        writes.pBufferInfo = &bufferInfo;
        writes.descriptorType = descriptorType;
        writes.dstArrayElement = buffer->bindlessIndex;

        if (writes.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            writes.dstSet = bindlessDescriptors[uniformBinding].descriptorSet;
            writes.dstBinding = 0;
        }
        if (writes.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            writes.dstSet = bindlessDescriptors[storageBufferBinding].descriptorSet;
            writes.dstBinding = 0;
        }

        vkUpdateDescriptorSets(device->getLogicalDevice(), 1, &writes, 0, nullptr);
    }

    return buffer->bindlessIndex;
}

ResourceManager& ResourceManager::copyBuffer(VkCommandBuffer& commandBuffer,
                                             BufferHardwareWrap& srcBuffer,
                                             BufferHardwareWrap& dstBuffer) {
    if (srcBuffer.bufferAllocInfo.size == dstBuffer.bufferAllocInfo.size) {
        VkBufferCopy copyRegion{};
        copyRegion.size = srcBuffer.bufferAllocInfo.size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer.bufferHandle, dstBuffer.bufferHandle, 1, &copyRegion);
    }

    return *this;
}

ResourceManager& ResourceManager::copyImage(VkCommandBuffer& commandBuffer,
                                            ImageHardwareWrap& source,
                                            ImageHardwareWrap& destination) {
    if (source.imageSize == destination.imageSize && source.imageFormat == destination.imageFormat) {
        VkImageCopy copyRegion{};
        copyRegion.srcSubresource.aspectMask = source.aspectMask;
        copyRegion.srcSubresource.layerCount = source.arrayLayers;
        copyRegion.dstSubresource.aspectMask = destination.aspectMask;
        copyRegion.dstSubresource.layerCount = destination.arrayLayers;
        copyRegion.extent.width = std::min(source.imageSize.x, destination.imageSize.x);
        copyRegion.extent.height = std::min(source.imageSize.y, destination.imageSize.y);
        copyRegion.extent.depth = 1;

        vkCmdCopyImage(commandBuffer,
                       source.imageHandle,
                       source.imageLayout,
                       destination.imageHandle,
                       destination.imageLayout,
                       1,
                       &copyRegion);
    }
    return *this;
}

ResourceManager& ResourceManager::copyBufferToImage(VkCommandBuffer& commandBuffer,
                                                    BufferHardwareWrap& buffer,
                                                    ImageHardwareWrap& image) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = image.aspectMask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = image.arrayLayers;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {image.imageSize.x, image.imageSize.y, 1};

    vkCmdCopyBufferToImage(commandBuffer,
                           buffer.bufferHandle,
                           image.imageHandle,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    return *this;
}

ResourceManager& ResourceManager::copyBufferToImage(VkCommandBuffer& commandBuffer,
                                                    BufferHardwareWrap& buffer,
                                                    ImageHardwareWrap& image,
                                                    uint32_t mipLevel) {
    if (mipLevel >= image.mipLevels) {
        return *this;  // Invalid mip level
    }

    // 计算指定 mip level 的尺寸
    uint32_t mipWidth = std::max(1u, image.imageSize.x >> mipLevel);
    uint32_t mipHeight = std::max(1u, image.imageSize.y >> mipLevel);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = image.aspectMask;
    region.imageSubresource.mipLevel = mipLevel;  // 指定 mip level
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = image.arrayLayers;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {mipWidth, mipHeight, 1};

    vkCmdCopyBufferToImage(commandBuffer,
                           buffer.bufferHandle,
                           image.imageHandle,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    return *this;
}

ResourceManager& ResourceManager::copyImageToBuffer(VkCommandBuffer& commandBuffer,
                                                    ImageHardwareWrap& image,
                                                    BufferHardwareWrap& buffer) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = image.aspectMask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = image.arrayLayers;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {image.imageSize.x, image.imageSize.y, 1};

    vkCmdCopyImageToBuffer(commandBuffer,
                           image.imageHandle,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           buffer.bufferHandle,
                           1,
                           &region);

    return *this;
}

ResourceManager& ResourceManager::blitImage(VkCommandBuffer& commandBuffer,
                                            ImageHardwareWrap& srcImage,
                                            ImageHardwareWrap& dstImage) {
    VkImageBlit blitRegion{};
    blitRegion.srcSubresource.aspectMask = srcImage.aspectMask;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcOffsets[1] = {static_cast<int32_t>(srcImage.imageSize.x),
                                static_cast<int32_t>(srcImage.imageSize.y), 1};
    blitRegion.dstSubresource.aspectMask = dstImage.aspectMask;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstOffsets[1] = {static_cast<int32_t>(dstImage.imageSize.x),
                                static_cast<int32_t>(dstImage.imageSize.y), 1};

    vkCmdBlitImage(commandBuffer,
                   srcImage.imageHandle,
                   srcImage.imageLayout,
                   dstImage.imageHandle,
                   dstImage.imageLayout,
                   1,
                   &blitRegion,
                   VK_FILTER_LINEAR);

    return *this;
}

void ResourceManager::copyBufferToHost(BufferHardwareWrap& buffer, void* cpuData, uint64_t size) {
    if (buffer.bufferAllocInfo.pMappedData != nullptr) {
        std::memcpy(cpuData, buffer.bufferAllocInfo.pMappedData, size);
    }
}

// Todo: 有待优化
void ResourceManager::transitionImageLayout(VkCommandBuffer& commandBuffer,
                                            ImageHardwareWrap& image,
                                            VkImageLayout newLayout,
                                            VkPipelineStageFlags2 dstStageMask,
                                            VkAccessFlags2 dstAccessMask) {
    std::vector<VkMemoryBarrier2> memoryBarriers;
    std::vector<VkBufferMemoryBarrier2> bufferBarriers;
    std::vector<VkImageMemoryBarrier2> imageBarriers;

    VkImageMemoryBarrier2 imageBarrier;
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = image.imageHandle;
    imageBarrier.dstStageMask = dstStageMask;
    imageBarrier.dstAccessMask = dstAccessMask;
    imageBarrier.oldLayout = image.imageLayout;
    imageBarrier.newLayout = newLayout;
    imageBarrier.subresourceRange.aspectMask = image.aspectMask;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.pNext = nullptr;

    image.imageLayout = imageBarrier.newLayout;

    imageBarriers.push_back(imageBarrier);

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size());
    dependencyInfo.pMemoryBarriers = memoryBarriers.data();
    dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriers.size());
    dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();
    dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
    dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
    dependencyInfo.pNext = nullptr;

    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}

VkShaderModule ResourceManager::createShaderModule(const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    coronaHardwareCheck(vkCreateShaderModule(device->getLogicalDevice(), &createInfo, nullptr, &shaderModule));

    return shaderModule;
}