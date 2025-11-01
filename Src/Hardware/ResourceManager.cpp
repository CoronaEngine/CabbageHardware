#include "ResourceManager.h"

// #define VMA_STATS_STRING_ENABLED 0
#define VK_NO_PROTOTYPES
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <Hardware/GlobalContext.h>
#include"HardwareExecutor.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    cleanUpResourceManager();
}

void ResourceManager::initResourceManager(DeviceManager &device)
{
    this->device = &device;

    CreateVmaAllocator();
    createTextureSampler();
    createBindlessDescriptorSet();
}

void ResourceManager::cleanUpResourceManager()
{
    if (device && device->logicalDevice != VK_NULL_HANDLE)
    {
        for (auto &desc : bindlessDescriptors)
        {
            if (desc.descriptorPool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(device->logicalDevice, desc.descriptorPool, nullptr);
                desc.descriptorPool = VK_NULL_HANDLE;
                desc.descriptorSet = VK_NULL_HANDLE;
            }
            if (desc.descriptorSetLayout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(device->logicalDevice, desc.descriptorSetLayout, nullptr);
                desc.descriptorSetLayout = VK_NULL_HANDLE;
            }
        }

        if (textureSampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device->logicalDevice, textureSampler, nullptr);
            textureSampler = VK_NULL_HANDLE;
        }
    }

    {
        std::scoped_lock lock(bindlessDescriptorMutex);
        UniformBindingList.clear();
        TextureBindingList.clear();
        StorageBufferBindingList.clear();
        StorageImageBindingList.clear();

        UniformBindingIndex = 0;
        TextureBindingIndex = 0;
        StorageBufferBindingIndex = 0;
        StorageImageBindingIndex = 0;
    }

    // 销毁导出内存池（若已创建）
    if (g_hAllocator != VK_NULL_HANDLE)
    {
        if (g_hBufferPool != VK_NULL_HANDLE)
        {
            vmaDestroyPool(g_hAllocator, g_hBufferPool);
            g_hBufferPool = VK_NULL_HANDLE;
            g_exportBufferPoolMemTypeIndex = UINT32_MAX;
        }
    }

    if (g_hAllocator != VK_NULL_HANDLE)
    {
        //Todo：删除的时候有内存泄露，同时考虑多线程的问题
        vmaDestroyAllocator(g_hAllocator);
        g_hAllocator = VK_NULL_HANDLE;
    }

    device = nullptr;
}

void ResourceManager::CreateVmaAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};

    // std::vector<VkExternalMemoryHandleTypeFlagsKHR> externalMemoryHandleTypes;
    // #if _WIN32 || _WIN64
    //     externalMemoryHandleTypes.push_back(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT);
    // #endif
    // allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes.data();
//#if _WIN32 || _WIN64
//    const VkExternalMemoryHandleTypeFlags externalMemoryHandleTypes[] = {
//        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT};
//    allocatorInfo.pTypeExternalMemoryHandleTypes = externalMemoryHandleTypes;
//#endif
    //bool g_EnableValidationLayer = true;
    //bool VK_KHR_get_memory_requirements2_enabled = false;
    //bool VK_KHR_get_physical_device_properties2_enabled = false;
    //bool VK_KHR_dedicated_allocation_enabled = false;
    //bool VK_KHR_bind_memory2_enabled = false;
    //bool VK_EXT_memory_budget_enabled = false;
    //bool VK_AMD_device_coherent_memory_enabled = false;
    //bool VK_KHR_buffer_device_address_enabled = true;
    //bool VK_EXT_memory_priority_enabled = false;
    //bool VK_EXT_debug_utils_enabled = false;
    //bool VK_KHR_maintenance5_enabled = false;
    //bool g_SparseBindingEnabled = false;

    allocatorInfo.physicalDevice = this->device->physicalDevice;
    allocatorInfo.device = this->device->logicalDevice;
    allocatorInfo.instance = globalHardwareContext.getVulkanInstance();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

    //if (VK_KHR_dedicated_allocation_enabled)
    //{
    //    allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
    //}
    //if (VK_KHR_bind_memory2_enabled)
    //{
    //    allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
    //}
//#if !defined(VMA_MEMORY_BUDGET) || VMA_MEMORY_BUDGET == 1
//    if (VK_EXT_memory_budget_enabled)
//    {
//        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
//    }
//#endif
//    if (VK_AMD_device_coherent_memory_enabled)
//    {
//        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
//    }
//    if (VK_KHR_buffer_device_address_enabled)
//    {
//        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
//    }
//#if !defined(VMA_MEMORY_PRIORITY) || VMA_MEMORY_PRIORITY == 1
//    if (VK_EXT_memory_priority_enabled)
//    {
//        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
//    }
//#endif
//    if (VK_KHR_maintenance5_enabled)
//    {
//        allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
//    }
    allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
    // if (USE_CUSTOM_CPU_ALLOCATION_CALLBACKS)
    //{
    //	allocatorInfo.pAllocationCallbacks = &g_CpuAllocationCallbacks;
    // }

//#if VMA_DYNAMIC_VULKAN_FUNCTIONS
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
//#endif

    // Uncomment to enable recording to CSV file.
    /*
    static VmaRecordSettings recordSettings = {};
    recordSettings.pFilePath = "VulkanSample.csv";
    outInfo.pRecordSettings = &recordSettings;
    */

    // Uncomment to enable HeapSizeLimit.
    /*
    static std::array<VkDeviceSize, VK_MAX_MEMORY_HEAPS> heapSizeLimit;
    std::fill(heapSizeLimit.begin(), heapSizeLimit.end(), VK_WHOLE_SIZE);
    heapSizeLimit[0] = 512ull * 1024 * 1024;
    outInfo.pHeapSizeLimit = heapSizeLimit.data();
    */

    vmaCreateAllocator(&allocatorInfo, &g_hAllocator);

    const VkPhysicalDeviceMemoryProperties *memProps = nullptr;
    vmaGetMemoryProperties(g_hAllocator, &memProps);
    for (uint32_t heapIndex = 0; heapIndex < memProps->memoryHeapCount; ++heapIndex)
    {
        const VkMemoryHeap &heap = memProps->memoryHeaps[heapIndex];
        if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        {
            deviceMemorySize += heap.size;
        }
        else if (heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
        {
            mutiInstanceMemorySize += heap.size;
        }
        else
        {
            hostSharedMemorySize += heap.size;
        }
    }

#ifdef CABBAGE_ENGINE_DEBUG
    std::cout << "Device Memory Size: " << (deviceMemorySize / 1073741824.0f) << " GB " << std::endl;
    std::cout << "Multi Instance Memory Size: " << (mutiInstanceMemorySize / 1073741824.0f) << " GB " << std::endl;
    std::cout << "Host Shared Memory Size: " << (hostSharedMemorySize / 1073741824.0f) << " GB " << std::endl;
#endif
}

void ResourceManager::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = false;
    samplerInfo.maxAnisotropy = this->device->deviceFeaturesUtils.supportedProperties.properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(1);
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(this->device->logicalDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void ResourceManager::destroyBuffer(BufferHardwareWrap &buffer)
{
    if (buffer.bufferAlloc != VK_NULL_HANDLE && buffer.bufferHandle != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(g_hAllocator, buffer.bufferHandle, buffer.bufferAlloc);
    }
}

// Todo: 方法写的很丑，有待重构
ResourceManager::BufferHardwareWrap ResourceManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, bool hostVisibleMapped)
{
    BufferHardwareWrap resultBuffer;
    resultBuffer.device = this->device;
    resultBuffer.resourceManager = this;

    if (size == 0)
    {
        return resultBuffer;
    }

    resultBuffer.bufferUsage = usage;

    VkBufferCreateInfo bufCreateInfo = {};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufCreateInfo.size = size;
    bufCreateInfo.usage = usage;

    // Keep queue family indices alive until after vkCreateBuffer/vmaCreateBuffer
    // Todo：这里后面可以优化，只传递需要的队列族索引
    std::vector<uint32_t> queueFamilyIndices;
    if (device->getQueueFamilyNumber() > 1)
    {
        queueFamilyIndices.resize(device->getQueueFamilyNumber());
        for (size_t i = 0; i < queueFamilyIndices.size(); i++)
        {
            queueFamilyIndices[i] = static_cast<uint32_t>(i);
        }
        bufCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        bufCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // External compatibility query
    VkExternalMemoryBufferCreateInfoKHR externalMemBufCreateInfo = {};
    externalMemBufCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR;
#if _WIN32 || _WIN64
    externalMemBufCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif __APPLE__
    // Not used in this repo
#elif __linux__
    externalBufferInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    bufCreateInfo.pNext = &externalMemBufCreateInfo;

    VkPhysicalDeviceExternalBufferInfo externalBufferInfo = {};
    externalBufferInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
    externalBufferInfo.flags = bufCreateInfo.flags;
    externalBufferInfo.usage = bufCreateInfo.usage;
    externalBufferInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    VkExternalBufferProperties externalBufferProperties = {};
    externalBufferProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;

    vkGetPhysicalDeviceExternalBufferProperties(this->device->physicalDevice, &externalBufferInfo, &externalBufferProperties);

    const auto feats = externalBufferProperties.externalMemoryProperties.externalMemoryFeatures;
    const bool exportable = (feats & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) != 0;
    const bool importable = (feats & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
    const bool dedicatedOnly = (feats & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;

    if (!exportable || !importable)
    {
        // Todo: fallback to non-exportable buffer creation
        VmaAllocationCreateInfo vbAllocCreateInfo = {};
        vbAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vbAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        VkResult res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &vbAllocCreateInfo, &resultBuffer.bufferHandle, &resultBuffer.bufferAlloc, &resultBuffer.bufferAllocInfo);
        if (res != VK_SUCCESS)
        {
            switch (res)
            {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                std::cerr << "Host memory allocation failed!" << std::endl;
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                std::cerr << "Device memory allocation failed!" << std::endl;
                break;
            default:
                std::cerr << "VMA buffer creation failed with error code: " << res << std::endl;
                break;
            }
        }
    }
    else if (!dedicatedOnly)
    {
        // Non-dedicated exportable buffer creation
        if (g_hBufferPool == VK_NULL_HANDLE)
        {
            createExternalBufferMemoryPool(bufCreateInfo);
        }

        VmaAllocationCreateInfo vbAllocCreateInfo = {};
        vbAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vbAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        vbAllocCreateInfo.pool = g_hBufferPool;

        VkResult res = vmaCreateBuffer(g_hAllocator, &bufCreateInfo, &vbAllocCreateInfo, &resultBuffer.bufferHandle, &resultBuffer.bufferAlloc, &resultBuffer.bufferAllocInfo);
        if (res != VK_SUCCESS)
        {
            switch (res)
            {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                std::cerr << "Host memory allocation failed!" << std::endl;
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                std::cerr << "Device memory allocation failed!" << std::endl;
                break;
            default:
                std::cerr << "VMA buffer creation failed with error code: " << res << std::endl;
                break;
            }
        }
    }
    else
    {
        // Dedicated allocation
        VkExportMemoryAllocateInfo exportAlloc{};
        exportAlloc.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
#if _WIN32 || _WIN64
        exportAlloc.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif __APPLE__
        exportAlloc.handleTypes = (VkExternalMemoryHandleTypeFlags)0;
#elif __linux__
        exportAlloc.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        if (hostVisibleMapped)
        {
            allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        // 专用导出内存，不使用内存池
        if (vmaCreateDedicatedBuffer(g_hAllocator, &bufCreateInfo, &allocInfo, &exportAlloc, &resultBuffer.bufferHandle, &resultBuffer.bufferAlloc, &resultBuffer.bufferAllocInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create dedicated exportable buffer!");
        }
    }

    VmaAllocationInfo allocInfoBasic{};
    vmaGetAllocationInfo(g_hAllocator, resultBuffer.bufferAlloc, &allocInfoBasic);
    if (allocInfoBasic.offset != 0)
    {
        std::cerr << "Warning: exportable buffer allocation offset != 0 (" << allocInfoBasic.offset << ")" << std::endl;
    }

    return resultBuffer;
}

void ResourceManager::createExternalBufferMemoryPool(const VkBufferCreateInfo &bufferInfo)
{
#if _WIN32 || _WIN64
     const VkExternalMemoryHandleTypeFlagsKHR handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif __APPLE__
     const VkExternalMemoryHandleTypeFlagsKHR handleType = 0;
#elif __linux__
     const VkExternalMemoryHandleTypeFlagsKHR handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

     VmaAllocationCreateInfo allocInfo{};
     allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
     allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

     // 查找合适的内存类型
     uint32_t memTypeIndex = VK_MAX_MEMORY_TYPES;
     VkResult res = vmaFindMemoryTypeIndexForBufferInfo(g_hAllocator, &bufferInfo, &allocInfo, &memTypeIndex);
     if (res != VK_SUCCESS)
     {
         throw std::runtime_error("vmaFindMemoryTypeIndexForBufferInfo failed!");
     }
     if (memTypeIndex == VK_MAX_MEMORY_TYPES)
     {
         throw std::runtime_error("No suitable memory type found for external memory pool");
     }

     // 配置内存池
     VkExportMemoryAllocateInfoKHR exportInfo{};
     exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
     exportInfo.pNext = nullptr;
     exportInfo.handleTypes = handleType;

     VmaPoolCreateInfo poolInfo{};
     poolInfo.memoryTypeIndex = memTypeIndex;
     poolInfo.pMemoryAllocateNext = &exportInfo;
     poolInfo.blockSize = 0; // 使用自动块大小
     poolInfo.minBlockCount = 1;
     poolInfo.maxBlockCount = 0; // 无限制

     // 创建内存池（导出 Buffer 池）
     res = vmaCreatePool(g_hAllocator, &poolInfo, &g_hBufferPool);
     if (res != VK_SUCCESS)
     {
         throw std::runtime_error("vmaCreatePool failed!");
     }
     g_exportBufferPoolMemTypeIndex = memTypeIndex;
 }

void ResourceManager::destroyImage(ImageHardwareWrap &image)
{
    // VkImageView依赖于VkImage，必须按（先销毁视图，再销毁图像）的顺序释放资源

    if (image.imageAlloc != VK_NULL_HANDLE && image.imageHandle != VK_NULL_HANDLE)
    {
        vmaDestroyImage(g_hAllocator, image.imageHandle, image.imageAlloc);
        image.imageHandle = VK_NULL_HANDLE;
        image.imageAlloc = VK_NULL_HANDLE;
    }

    if (image.imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(this->device->logicalDevice, image.imageView, nullptr);
        image.imageView = VK_NULL_HANDLE;
    }
}

VkImageView ResourceManager::createImageView(ImageHardwareWrap &image)
{
    // Check if the format supports the required usage
    // VkFormatProperties2 formatProperties = {};
    // vkGetPhysicalDeviceFormatProperties2(deviceManager.mainDevice.physicalDevice, image.imageFormat, &formatProperties);

    // if ((image.imageUsage & VK_IMAGE_USAGE_STORAGE_BIT) && !(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
    //	throw std::runtime_error("Image format does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT.");
    // }

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

    if (vkCreateImageView(this->device->logicalDevice, &viewInfo, nullptr, &image.imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
        return VK_NULL_HANDLE;
    }
    else
    {
        return image.imageView;
    }
}

ResourceManager::ImageHardwareWrap ResourceManager::createImage(ktm::uvec2 imageSize, VkFormat imageFormat, uint32_t pixelSize, VkImageUsageFlags imageUsage, int arrayLayers, int mipLevels, VkImageTiling tiling)
{
    ImageHardwareWrap resultImage;
    resultImage.pixelSize = pixelSize;
    resultImage.device = this->device;
    resultImage.resourceManager = this;
    resultImage.imageSize = imageSize;
    resultImage.imageFormat = imageFormat;
    resultImage.arrayLayers = arrayLayers;
    resultImage.mipLevels = mipLevels;
    resultImage.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (imageUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        resultImage.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        resultImage.clearValue.depthStencil = {1.0f, 0};
    }
    else
    {
        resultImage.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        resultImage.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    }

    imageUsage = imageUsage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    resultImage.imageUsage = imageUsage;

    if (imageSize.x != 0 || imageSize.y != 0)
    {
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

        // Keep queue family indices alive until after vkCreateImage/vmaCreateImage
        std::vector<uint32_t> imageQueueFamilyIndices;
        if (device->getQueueFamilyNumber() > 1)
        {
            imageQueueFamilyIndices.resize(device->getQueueFamilyNumber());
            for (size_t i = 0; i < imageQueueFamilyIndices.size(); i++)
            {
                imageQueueFamilyIndices[i] = static_cast<uint32_t>(i);
            }
            imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(imageQueueFamilyIndices.size());
            imageInfo.pQueueFamilyIndices = imageQueueFamilyIndices.data();
        }
        else
        {
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VmaAllocationCreateInfo imageAllocCreateInfo = {};
        imageAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        if (vmaCreateImage(g_hAllocator, &imageInfo, &imageAllocCreateInfo, &resultImage.imageHandle, &resultImage.imageAlloc, &resultImage.imageAllocInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        resultImage.imageView = createImageView(resultImage);
    }

    return resultImage;
}

ResourceManager &ResourceManager::copyImage(VkCommandBuffer &commandBuffer, ImageHardwareWrap &source, ImageHardwareWrap &destination)
{
    //if (source.pixelSize == destination.pixelSize)
    //{
        if (source.imageSize == destination.imageSize && source.imageFormat == destination.imageFormat)
        {/*

            if (source.device != destination.device)
            {
                VkDeviceSize imageSizeBytes = source.imageSize.x * source.imageSize.y * source.pixelSize;

                ResourceManager::BufferHardwareWrap tempSrcStaging;
                ResourceManager::BufferHardwareWrap tempDstStaging;
                if (srcStaging == nullptr && dstStaging == nullptr)
                {
                    tempSrcStaging = source.resourceManager->createBuffer(imageSizeBytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
                    tempDstStaging = destination.resourceManager->createBuffer(imageSizeBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
                }
                else
                {
                    tempSrcStaging = *srcStaging;
                    tempDstStaging = *dstStaging;
                }

                auto srcCopyCmd = [&](const VkCommandBuffer& commandBuffer) {
                    VkBufferImageCopy region{};
                    region.bufferOffset = 0;
                    region.bufferRowLength = 0;
                    region.bufferImageHeight = 0;
                    region.imageSubresource.aspectMask = source.aspectMask;
                    region.imageSubresource.mipLevel = 0;
                    region.imageSubresource.baseArrayLayer = 0;
                    region.imageSubresource.layerCount = 1;
                    region.imageOffset = {0, 0, 0};
                    region.imageExtent = {source.imageSize.x, source.imageSize.y, 1};
                    vkCmdCopyImageToBuffer(commandBuffer, source.imageHandle, VK_IMAGE_LAYOUT_GENERAL, tempSrcStaging.bufferHandle, 1, &region);
                };
                source.device->startCommands(DeviceManager::TransferQueue) << srcCopyCmd << source.device->endCommands();

                void *mappedData = nullptr;
                void *dstMappedData = nullptr;
                vmaMapMemory(g_hAllocator, tempSrcStaging.bufferAlloc, &mappedData);
                vmaMapMemory(g_hAllocator, tempDstStaging.bufferAlloc, &dstMappedData);
                memcpy(dstMappedData, mappedData, imageSizeBytes);
                vmaUnmapMemory(g_hAllocator, tempSrcStaging.bufferAlloc);
                vmaUnmapMemory(g_hAllocator, tempDstStaging.bufferAlloc);

                auto dstCopyCmd = [&](const VkCommandBuffer& commandBuffer) {
                    VkBufferImageCopy region{};
                    region.bufferOffset = 0;
                    region.bufferRowLength = 0;
                    region.bufferImageHeight = 0;
                    region.imageSubresource.aspectMask = destination.aspectMask;
                    region.imageSubresource.mipLevel = 0;
                    region.imageSubresource.baseArrayLayer = 0;
                    region.imageSubresource.layerCount = 1;
                    region.imageOffset = {0, 0, 0};
                    region.imageExtent = {destination.imageSize.x, destination.imageSize.y, 1};
                    vkCmdCopyBufferToImage(commandBuffer, tempDstStaging.bufferHandle, destination.imageHandle, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
                };
                destination.device->startCommands(DeviceManager::TransferQueue) << dstCopyCmd << destination.device->endCommands();

                if (srcStaging == nullptr && dstStaging == nullptr)
                {
                    source.resourceManager->destroyBuffer(tempSrcStaging);
                    destination.resourceManager->destroyBuffer(tempDstStaging);
                }

                return true;
            }
            else*/
            {
                //auto runCommand = [&](const VkCommandBuffer &commandBuffer) {
                    VkImageCopy imageCopyRegion{};
                    imageCopyRegion.srcSubresource.aspectMask = source.aspectMask;
                    imageCopyRegion.srcSubresource.layerCount = 1;
                    imageCopyRegion.dstSubresource.aspectMask = destination.aspectMask;
                    imageCopyRegion.dstSubresource.layerCount = 1;
                    imageCopyRegion.extent.width = source.imageSize.x;
                    imageCopyRegion.extent.height = source.imageSize.y;
                    imageCopyRegion.extent.depth = 1;

                    vkCmdCopyImage(commandBuffer, source.imageHandle, VK_IMAGE_LAYOUT_GENERAL, destination.imageHandle, VK_IMAGE_LAYOUT_GENERAL, 1, &imageCopyRegion);
                //};

                 //*executor << runCommand;

                 return *this;
                //return true;
            }
        }

        //return false;

        //else
        //{
        //    if (source.device == destination.device)
        //    {
        //        // ͬ device��ʹ�� vkCmdBlitImage
        //        auto runCommand = [&](VkCommandBuffer &commandBuffer) {
        //            VkImageBlit imageBlit{};
        //            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //            imageBlit.srcSubresource.mipLevel = 0;
        //            imageBlit.srcSubresource.baseArrayLayer = 0;
        //            imageBlit.srcSubresource.layerCount = 1;
        //            imageBlit.srcOffsets[0] = {0, 0, 0};
        //            imageBlit.srcOffsets[1] = {int32_t(source.imageSize.x), int32_t(source.imageSize.y), 1};

        //            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //            imageBlit.dstSubresource.mipLevel = 0;
        //            imageBlit.dstSubresource.baseArrayLayer = 0;
        //            imageBlit.dstSubresource.layerCount = 1;
        //            imageBlit.dstOffsets[0] = {0, 0, 0};
        //            imageBlit.dstOffsets[1] = {int32_t(destination.imageSize.x), int32_t(destination.imageSize.y), 1};

        //            vkCmdBlitImage(commandBuffer,
        //                           source.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        //                           destination.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //                           1, &imageBlit, VK_FILTER_LINEAR);
        //        };
        //        source.device->executeSingleTimeCommands(runCommand);
        //        return true;
        //    }
        //}
        //    else
        //    {
        //        // device ��ͬ��ʹ�� staging buffer
        //        VkDeviceSize srcSize = source.imageSize.x * source.imageSize.y * source.pixelSize;
        //        VkDeviceSize dstSize = destination.imageSize.x * destination.imageSize.y * destination.pixelSize;

        //        // 1. Դ device��image -> buffer
        //        BufferHardwareWrap srcStaging = source.resourceManager->createBuffer(srcSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        //        auto srcCopyCmd = [&](VkCommandBuffer commandBuffer) {
        //            VkBufferImageCopy region{};
        //            region.bufferOffset = 0;
        //            region.bufferRowLength = 0;
        //            region.bufferImageHeight = 0;
        //            region.imageSubresource.aspectMask = source.aspectMask;
        //            region.imageSubresource.mipLevel = 0;
        //            region.imageSubresource.baseArrayLayer = 0;
        //            region.imageSubresource.layerCount = 1;
        //            region.imageOffset = {0, 0, 0};
        //            region.imageExtent = {source.imageSize.x, source.imageSize.y, 1};
        //            vkCmdCopyImageToBuffer(commandBuffer, source.imageHandle, VK_IMAGE_LAYOUT_GENERAL, srcStaging.bufferHandle, 1, &region);
        //        };
        //        source.device->executeSingleTimeCommands(srcCopyCmd);

        //        // 2. host ����
        //        void *mappedData = nullptr;
        //        vmaMapMemory(source.resourceManager->g_hAllocator, srcStaging.bufferAlloc, &mappedData);

        //        // 3. Ŀ�� device��buffer -> image
        //        BufferHardwareWrap dstStaging = destination.resourceManager->createBuffer(dstSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        //        void *dstMappedData = nullptr;
        //        vmaMapMemory(destination.resourceManager->g_hAllocator, dstStaging.bufferAlloc, &dstMappedData);
        //        memcpy(dstMappedData, mappedData, std::min(srcSize, dstSize));
        //        vmaUnmapMemory(source.resourceManager->g_hAllocator, srcStaging.bufferAlloc);
        //        vmaUnmapMemory(destination.resourceManager->g_hAllocator, dstStaging.bufferAlloc);

        //        //auto dstCopyCmd = [&](VkCommandBuffer commandBuffer) {
        //        //    VkBufferImageCopy region{};
        //        //    region.bufferOffset = 0;
        //        //    region.bufferRowLength = 0;
        //        //    region.bufferImageHeight = 0;
        //        //    region.imageSubresource.aspectMask = destination.aspectMask;
        //        //    region.imageSubresource.mipLevel = 0;
        //        //    region.imageSubresource.baseArrayLayer = 0;
        //        //    region.imageSubresource.layerCount = 1;
        //        //    region.imageOffset = {0, 0, 0};
        //        //    region.imageExtent = {destination.imageSize.x, destination.imageSize.y, 1};
        //        //    vkCmdCopyBufferToImage(commandBuffer, dstStaging.bufferHandle, destination.imageHandle, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
        //        //};
        //        //destination.device->executeSingleTimeCommands(dstCopyCmd);

        //        //auto runCommand = [&](VkCommandBuffer &commandBuffer) {
        //        //    VkImageBlit imageBlit{};
        //        //    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //        //    imageBlit.srcSubresource.mipLevel = 0;
        //        //    imageBlit.srcSubresource.baseArrayLayer = 0;
        //        //    imageBlit.srcSubresource.layerCount = 1;
        //        //    imageBlit.srcOffsets[0] = {0, 0, 0};
        //        //    imageBlit.srcOffsets[1] = {int32_t(source.imageSize.x), int32_t(source.imageSize.y), 1};

        //        //    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //        //    imageBlit.dstSubresource.mipLevel = 0;
        //        //    imageBlit.dstSubresource.baseArrayLayer = 0;
        //        //    imageBlit.dstSubresource.layerCount = 1;
        //        //    imageBlit.dstOffsets[0] = {0, 0, 0};
        //        //    imageBlit.dstOffsets[1] = {int32_t(destination.imageSize.x), int32_t(destination.imageSize.y), 1};

        //        //    vkCmdBlitImage(commandBuffer,
        //        //                   source.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        //        //                   destination.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        //        //                   1, &imageBlit, VK_FILTER_LINEAR);
        //        //};


        //        auto runCommand = [&](VkCommandBuffer &commandBuffer) {

        //            VkImageBlit imageBlit;
        //            imageBlit.dstOffsets[0] = VkOffset3D{0, 0, 0};
        //            imageBlit.dstOffsets[1] = VkOffset3D{int32_t(destination.imageSize.x), int32_t(destination.imageSize.y), 1};

        //            imageBlit.srcOffsets[0] = VkOffset3D{0, 0, 0};
        //            imageBlit.srcOffsets[1] = VkOffset3D{int32_t(dstStaging.imageSize.x), int32_t(source.imageSize.y), 1};

        //            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        //            imageBlit.dstSubresource.baseArrayLayer = 0;
        //            imageBlit.srcSubresource.baseArrayLayer = 0;

        //            imageBlit.dstSubresource.layerCount = 1;
        //            imageBlit.srcSubresource.layerCount = 1;

        //            imageBlit.dstSubresource.mipLevel = 0;
        //            imageBlit.srcSubresource.mipLevel = 0;

        //            vkCmdBlitImage(commandBuffer, source.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        //                           destination.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
        //                           &imageBlit, VK_FILTER_LINEAR);
        //        };
        //        destination.device->executeSingleTimeCommands(runCommand);

        //        source.resourceManager->destroyBuffer(srcStaging);
        //        destination.resourceManager->destroyBuffer(dstStaging);

        //        return true;
        //    }
        //}
    //}
    //else
    //{
    //    std::cerr << "Pixel size mismatch between source and destination images." << std::endl;
    //    return false;
    //}
}

//void ResourceManager::transitionImageLayoutUnblocked(const VkCommandBuffer &commandBuffer, ImageHardwareWrap &image,
//                                                     VkImageLayout newLayout, VkPipelineStageFlags sourceStage,
//                                                     VkPipelineStageFlags destinationStage)
//{
//
//}

ResourceManager &ResourceManager::transitionImageLayout(VkCommandBuffer &commandBuffer, ImageHardwareWrap &image, VkImageLayout newLayout, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage)
{
    if (image.imageLayout != newLayout)
    {
        if (image.imageLayout != newLayout)
            {
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = image.imageLayout;
                barrier.newLayout = newLayout;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = image.imageHandle;
                barrier.subresourceRange.aspectMask = image.aspectMask;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = image.mipLevels;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = image.arrayLayers;

                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = 0;

                if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                         newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                         newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask =
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
                         newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                         newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_GENERAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                         newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                         newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                         newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
                         newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                {
                    barrier.srcAccessMask = 0;
                    barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
                         newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                else if (image.imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                         newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                }
                else
                {
                    throw std::invalid_argument("unsupported layout transition!");
                }

                vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

                image.imageLayout = newLayout;
            }

        return *this;
    }
}

ResourceManager &ResourceManager::copyImageToBuffer(VkCommandBuffer &commandBuffer, ImageHardwareWrap &image, BufferHardwareWrap &buffer)
{

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            image.imageSize.x,
            image.imageSize.y,
            1};

        vkCmdCopyImageToBuffer(commandBuffer, image.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.bufferHandle, 1, &region);

    return *this;
}

ResourceManager &ResourceManager::copyBufferToImage(VkCommandBuffer &commandBuffer, BufferHardwareWrap &buffer, ImageHardwareWrap &image)
{

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            image.imageSize.x,
            image.imageSize.y,
            1};

        vkCmdCopyBufferToImage(commandBuffer, buffer.bufferHandle, image.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    return *this;
}

ResourceManager &ResourceManager::copyBuffer(VkCommandBuffer &commandBuffer, BufferHardwareWrap &srcBuffer, BufferHardwareWrap &dstBuffer)
{
    if (srcBuffer.bufferAllocInfo.size == dstBuffer.bufferAllocInfo.size)
    {
            VkBufferCopy copyRegion{};
            copyRegion.size = srcBuffer.bufferAllocInfo.size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer.bufferHandle, dstBuffer.bufferHandle, 1, &copyRegion);
    }

    return *this;
}

uint32_t ResourceManager::findExternalMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(this->device->physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {

            // 添加详细的外部内存类型验证
            VkPhysicalDeviceExternalImageFormatInfo extFormatInfo = {};
            extFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
            extFormatInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

            VkPhysicalDeviceImageFormatInfo2 formatInfo = {};
            formatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
            formatInfo.pNext = &extFormatInfo;
            formatInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            formatInfo.type = VK_IMAGE_TYPE_2D;
            formatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            formatInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

            VkImageFormatProperties2 formatProps = {};
            formatProps.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

            VkResult result = vkGetPhysicalDeviceImageFormatProperties2(
                this->device->physicalDevice,
                &formatInfo,
                &formatProps);

            if (result == VK_SUCCESS)
            {
                std::cout << "Found compatible memory type index: " << i << std::endl;
                std::cout << "Memory type flags: " << memProperties.memoryTypes[i].propertyFlags << std::endl;
                return i;
            }

            std::cout << "Memory type " << i << " not compatible with external memory" << std::endl;
        }
    }

    throw std::runtime_error("failed to find suitable external memory type!");
}

//ResourceManager::BufferHardwareWrap ResourceManager::importBufferMemory(const ExternalMemoryHandle &memHandle, const BufferHardwareWrap &sourceBuffer)
//{
//    // 验证外部内存句柄的有效性
//#if _WIN32 || _WIN64
//    if (memHandle.handle == nullptr || memHandle.handle == INVALID_HANDLE_VALUE)
//    {
//        throw std::runtime_error("Cannot import buffer with invalid memory handle!");
//    }
//
//    VkPhysicalDeviceExternalBufferInfo externalBufferInfo{};
//    externalBufferInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
//    externalBufferInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
//    externalBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//
//    VkExternalBufferProperties externalBufferProperties{};
//    externalBufferProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;
//
//    vkGetPhysicalDeviceExternalBufferProperties(this->device->physicalDevice, &externalBufferInfo, &externalBufferProperties);
//#endif
//
//    BufferHardwareWrap importedBuffer = {};
//    importedBuffer.device = this->device;
//    importedBuffer.resourceManager = this;
//    importedBuffer.bufferUsage = sourceBuffer.bufferUsage;
//
//    // 创建新的缓冲区
//    VkBufferCreateInfo bufferInfo = {};
//    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//    bufferInfo.size = sourceBuffer.bufferAllocInfo.size;
//    bufferInfo.usage = importedBuffer.bufferUsage;
//    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//    VkExternalMemoryBufferCreateInfo externalInfo = {};
//    externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
//#if _WIN32 || _WIN64
//    externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
//#endif
//    bufferInfo.pNext = &externalInfo;
//
//    if (vkCreateBuffer(this->device->logicalDevice, &bufferInfo, nullptr, &importedBuffer.bufferHandle) != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to create buffer for import!");
//    }
//
//    // 获取内存需求
//    VkMemoryRequirements memRequirements;
//    vkGetBufferMemoryRequirements(this->device->logicalDevice, importedBuffer.bufferHandle, &memRequirements);
//
//    VkMemoryAllocateInfo allocInfo = {};
//    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//    allocInfo.allocationSize = memRequirements.size;
//    allocInfo.memoryTypeIndex = findExternalMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
//
//#if _WIN32 || _WIN64
//    VkImportMemoryWin32HandleInfoKHR importInfo = {};
//    importInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
//    importInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
//    importInfo.handle = memHandle.handle;
//    allocInfo.pNext = &importInfo;
//#endif
//
//    VkDeviceMemory importedMemory = VK_NULL_HANDLE;
//    VkResult result = vkAllocateMemory(this->device->logicalDevice, &allocInfo, nullptr, &importedMemory);
//    if (result != VK_SUCCESS)
//    {
//        std::string errorMsg = "Failed to allocate imported memory. Error code: " + std::to_string(result);
//        switch (result)
//        {
//        case VK_ERROR_OUT_OF_HOST_MEMORY:
//            errorMsg += " (VK_ERROR_OUT_OF_HOST_MEMORY)";
//            break;
//        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//            errorMsg += " (VK_ERROR_OUT_OF_DEVICE_MEMORY)";
//            break;
//        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
//            errorMsg += " (VK_ERROR_INVALID_EXTERNAL_HANDLE)";
//            break;
//        }
//        throw std::runtime_error(errorMsg);
//    }
//
//    if (importedMemory != nullptr)
//    {
//        if (vkBindBufferMemory(this->device->logicalDevice, importedBuffer.bufferHandle, importedMemory, 0) != VK_SUCCESS)
//        {
//            throw std::runtime_error("failed to bind imported memory to buffer!");
//        }
//
//        // 保存分配信息
//        importedBuffer.bufferAllocInfo.deviceMemory = importedMemory;
//        importedBuffer.bufferAllocInfo.size = sourceBuffer.bufferAllocInfo.size;
//
//        return importedBuffer;
//    }
//    else
//    {
//        throw std::runtime_error("failed to bind imported memory to buffer!");
//    }
//}

ResourceManager::BufferHardwareWrap ResourceManager::importBufferMemory(const ExternalMemoryHandle &memHandle, const BufferHardwareWrap &sourceBuffer)
{
    // 验证外部内存句柄的有效性
#if _WIN32 || _WIN64
    if (memHandle.handle == nullptr || memHandle.handle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Cannot import buffer with invalid memory handle!");
    }
#endif

    BufferHardwareWrap importedBuffer = {};
    importedBuffer.device = this->device;
    importedBuffer.resourceManager = this;
    importedBuffer.bufferUsage = sourceBuffer.bufferUsage;

    VkBufferCreateInfo bufCreateInfo = {};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufCreateInfo.size = sourceBuffer.bufferAllocInfo.size;
    bufCreateInfo.usage = importedBuffer.bufferUsage;

    // Keep queue family indices alive until after vkCreateBuffer/vmaCreateBuffer
    // Todo：这里后面可以优化，只传递需要的队列族索引
    std::vector<uint32_t> queueFamilyIndices;
    if (device->getQueueFamilyNumber() > 1)
    {
        queueFamilyIndices.resize(device->getQueueFamilyNumber());
        for (size_t i = 0; i < queueFamilyIndices.size(); i++)
        {
            queueFamilyIndices[i] = static_cast<uint32_t>(i);
        }
        bufCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        bufCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        bufCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    else
    {
        bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // External compatibility query
    VkExternalMemoryBufferCreateInfoKHR externalMemBufCreateInfo = {};
    externalMemBufCreateInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR;
#if _WIN32 || _WIN64
    externalMemBufCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#elif __APPLE__
    // Not used in this repo
#elif __linux__
    externalBufferInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif
    bufCreateInfo.pNext = &externalMemBufCreateInfo;

    VkPhysicalDeviceExternalBufferInfo externalBufferInfo = {};
    externalBufferInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
    externalBufferInfo.flags = bufCreateInfo.flags;
    externalBufferInfo.usage = bufCreateInfo.usage;
    externalBufferInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    VkExternalBufferProperties externalBufferProperties = {};
    externalBufferProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;

    vkGetPhysicalDeviceExternalBufferProperties(this->device->physicalDevice, &externalBufferInfo, &externalBufferProperties);

    const auto feats = externalBufferProperties.externalMemoryProperties.externalMemoryFeatures;
    const bool exportable = (feats & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) != 0;
    const bool importable = (feats & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
    const bool dedicatedOnly = (feats & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;

    VkImportMemoryWin32HandleInfoKHR importMemHandleInfo = {};
    importMemHandleInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
    importMemHandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    importMemHandleInfo.handle = memHandle.handle;
    importMemHandleInfo.name = nullptr;

    VmaAllocationCreateInfo importAllocCreateInfo = {};
    importAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    importAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    importedBuffer.bufferHandle = VK_NULL_HANDLE;
    importedBuffer.bufferAlloc = VK_NULL_HANDLE;
    vmaCreateDedicatedBuffer(g_hAllocator, &bufCreateInfo, &importAllocCreateInfo, &importMemHandleInfo, &importedBuffer.bufferHandle, &importedBuffer.bufferAlloc, &importedBuffer.bufferAllocInfo);
    return importedBuffer;
}


ResourceManager::BufferHardwareWrap ResourceManager::importHostBuffer(void *hostPtr, uint64_t size)
{
    BufferHardwareWrap bufferWrap;
    bufferWrap.device = this->device;
    bufferWrap.resourceManager = this;
    bufferWrap.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    // 1. 创建 VkBuffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = bufferWrap.bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // 2. 设置导入主机指针信息
    VkImportMemoryHostPointerInfoEXT importInfo{};
    importInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
    importInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
    importInfo.pHostPointer = hostPtr;

    // 3. VMA 分配参数
    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocCreateInfo.pUserData = nullptr;

    // 4. 创建 Buffer
    VkResult res = vmaCreateBuffer(
        g_hAllocator,
        &bufferInfo,
        &allocCreateInfo,
        &bufferWrap.bufferHandle,
        &bufferWrap.bufferAlloc,
        &bufferWrap.bufferAllocInfo);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("vmaCreateBuffer with host pointer import failed!");
    }

    return bufferWrap;
}

//void ResourceManager::TestWin32HandlesImport(BufferHardwareWrap &srcStaging, BufferHardwareWrap &dstStaging, VkDeviceSize imageSizeBytes, ResourceManager &srcResourceManager, ResourceManager &dstResourceManager)
//{
//    constexpr VkExternalMemoryHandleTypeFlagBits handleType =
//        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
//
//    constexpr static VkExportMemoryAllocateInfoKHR exportMemAllocInfo{
//        VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR,
//        nullptr,
//        handleType};
//
//    constexpr static VkExternalMemoryBufferCreateInfoKHR externalMemBufCreateInfo{
//        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO_KHR,
//        nullptr,
//        handleType};
//
//    //BufferHardwareWrap srcBuffer;
//    srcStaging.device = &srcResourceManager.getDeviceManager();
//    srcStaging.resourceManager = &srcResourceManager;
//
//    if (imageSizeBytes > 0)
//    {
//        srcStaging.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//
//        VkBufferCreateInfo bufCreateInfo = {};
//        bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//        bufCreateInfo.size = imageSizeBytes;
//        bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//
//        if (device->getQueueFamilyNumber() > 1)
//        {
//            std::vector<uint32_t> queueFamilys(device->getQueueFamilyNumber());
//            for (size_t i = 0; i < queueFamilys.size(); i++)
//            {
//                queueFamilys[i] = i;
//            }
//            bufCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
//            bufCreateInfo.queueFamilyIndexCount = queueFamilys.size();
//            bufCreateInfo.pQueueFamilyIndices = queueFamilys.data();
//        }
//        else
//        {
//            bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//        }
//
//        bufCreateInfo.pNext = &externalMemBufCreateInfo;
//
//        bool requiresDedicated = true;
//        {
//            VkPhysicalDeviceExternalBufferInfo externalBufferInfo = {
//                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO};
//            externalBufferInfo.flags = bufCreateInfo.flags;
//            externalBufferInfo.usage = bufCreateInfo.usage;
//            externalBufferInfo.handleType = handleType;
//
//            VkExternalBufferProperties externalBufferProperties = {
//                VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES};
//
//            vkGetPhysicalDeviceExternalBufferProperties(srcStaging.device->physicalDevice, &externalBufferInfo, &externalBufferProperties);
//            constexpr VkExternalMemoryFeatureFlags expectedFlags =VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
//            if ((externalBufferProperties.externalMemoryProperties.externalMemoryFeatures & expectedFlags) != expectedFlags)
//            {
//                return;
//            }
//            requiresDedicated = (externalBufferProperties.externalMemoryProperties.externalMemoryFeatures &
//                                 VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT) != 0;
//        }
//
//        VmaAllocationCreateInfo vbAllocCreateInfo = {};
//        vbAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
//        vbAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
//        vbAllocCreateInfo.pool = srcResourceManager.getVmaPool();
//
//
//        VkResult res = vmaCreateBuffer(srcResourceManager.getVmaAllocator(), &bufCreateInfo, &vbAllocCreateInfo, &srcStaging.bufferHandle, &srcStaging.bufferAlloc, &srcStaging.bufferAllocInfo);
//        if (res != VK_SUCCESS)
//        {
//            switch (res)
//            {
//            case VK_ERROR_OUT_OF_HOST_MEMORY:
//                std::cerr << "Host memory allocation failed!" << std::endl;
//                break;
//            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
//                std::cerr << "Device memory allocation failed!" << std::endl;
//                break;
//            default:
//                std::cerr << "VMA buffer creation failed with error code: " << res << std::endl;
//                break;
//            }
//        }
//
//        HANDLE handle = NULL;
//        VkResult result = vmaGetMemoryWin32Handle2(srcResourceManager.getVmaAllocator(), srcStaging.bufferAlloc, handleType, nullptr, &handle);
//
//        VkImportMemoryWin32HandleInfoKHR importMemHandleInfo = {
//            VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR};
//        importMemHandleInfo.handleType = handleType;
//        importMemHandleInfo.handle = handle;
//        importMemHandleInfo.name = nullptr;
//        VmaAllocationCreateInfo importAllocCreateInfo = {};
//        importAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
//        importAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
//
//        dstStaging.bufferHandle = VK_NULL_HANDLE;
//        dstStaging.bufferAlloc = VK_NULL_HANDLE;
//        dstStaging.device = &dstResourceManager.getDeviceManager();
//        dstStaging.resourceManager = &dstResourceManager;
//        vmaCreateDedicatedBuffer(dstResourceManager.getVmaAllocator(), &bufCreateInfo, &importAllocCreateInfo,
//                                 &importMemHandleInfo, &dstStaging.bufferHandle, &dstStaging.bufferAlloc, &dstStaging.bufferAllocInfo);
//
//    }
//
//}

//void ResourceManager::copyBufferToCpu(BufferHardwareWrap &buffer, void *cpuData)
//{
//    void *mappedData = nullptr;
//    VkResult result = vmaMapMemory(buffer.resourceManager->getVmaAllocator(), buffer.bufferAlloc, &mappedData);
//    if (result != VK_SUCCESS)
//    {
//        throw std::runtime_error("Failed to map memory");
//    }
//
//    vmaInvalidateAllocation(buffer.resourceManager->getVmaAllocator(), buffer.bufferAlloc, 0, VK_WHOLE_SIZE);
//    memcpy(cpuData, mappedData, buffer.bufferAllocInfo.size);
//    vmaUnmapMemory(buffer.resourceManager->getVmaAllocator(), buffer.bufferAlloc);
//}

//void ResourceManager::copyBufferToCpu(VkDevice &device, VkDeviceMemory &memory, VkDeviceSize size, void *cpuData)
//{
//    void *mappedData = nullptr;
//    VkResult result = vkMapMemory(device, memory, 0, size, 0, &mappedData);
//    if (result != VK_SUCCESS)
//    {
//        throw std::runtime_error("Failed to map memory");
//    }
//
//    memcpy(cpuData, mappedData, size);
//    vkUnmapMemory(device, memory);
//}

ResourceManager::ExternalMemoryHandle ResourceManager::exportBufferMemory(BufferHardwareWrap &sourceBuffer)
{
    ExternalMemoryHandle memHandle{};

    if (sourceBuffer.bufferHandle == VK_NULL_HANDLE || sourceBuffer.bufferAlloc == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Cannot export memory from invalid buffer!");
    }

    // 确保是专用分配（offset==0），否则导出的句柄指向整个内存块而非子分配，会导致导入侧偏移/对齐错误
    /*{
        VmaAllocationInfo info{};
        vmaGetAllocationInfo(g_hAllocator, sourceBuffer.bufferAlloc, &info);
        if (info.offset != 0)
        {
            throw std::runtime_error("exportBufferMemory: allocation is not dedicated (offset != 0). Use createExportableBuffer to allocate a dedicated, exportable buffer.");
        }
    }*/

#if _WIN32 || _WIN64
    VkResult result = vmaGetMemoryWin32Handle2(g_hAllocator, sourceBuffer.bufferAlloc, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT, nullptr, &memHandle.handle);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to export memory handle! VkResult: " + std::to_string(result));
    }

    if (memHandle.handle == nullptr || memHandle.handle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("Exported memory handle is invalid!");
    }
#else
    throw std::runtime_error("Exporting memory is not implemented on this platform!");
#endif

    return memHandle;
}

//ResourceManager::ExternalMemoryHandle ResourceManager::exportImageMemory(ImageHardwareWrap &sourceImage)
//{
//    ExternalMemoryHandle memHandle{};
//
//    if (sourceImage.imageHandle == VK_NULL_HANDLE || sourceImage.imageAlloc == VK_NULL_HANDLE)
//    {
//        throw std::runtime_error("Cannot export memory from invalid image!");
//    }
//
//    // 为保证导入侧简单安全，要求为 dedicated 分配（offset==0）
//    {
//        VmaAllocationInfo info{};
//        vmaGetAllocationInfo(g_hAllocator, sourceImage.imageAlloc, &info);
//        if (info.offset != 0)
//        {
//            throw std::runtime_error("exportImageMemory: allocation is not dedicated (offset != 0). Use createExportableImage with dedicated path to allocate an exportable image.");
//        }
//    }
//
//#if _WIN32 || _WIN64
//    VkResult result = vmaGetMemoryWin32Handle2(g_hAllocator, sourceImage.imageAlloc, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT, nullptr, &memHandle.handle);
//    if (result != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to export image memory handle! VkResult: " + std::to_string(result));
//    }
//    if (memHandle.handle == nullptr || memHandle.handle == INVALID_HANDLE_VALUE)
//    {
//        throw std::runtime_error("Exported image memory handle is invalid!");
//    }
//#else
//    throw std::runtime_error("Exporting image memory is not implemented on this platform!");
//#endif
//
//    return memHandle;
//}
//
//ResourceManager::ImageHardwareWrap ResourceManager::importImageMemory(const ExternalMemoryHandle &memHandle, const ImageHardwareWrap &sourceImage)
//{
//#if _WIN32 || _WIN64
//    if (memHandle.handle == nullptr || memHandle.handle == INVALID_HANDLE_VALUE)
//    {
//        throw std::runtime_error("Cannot import image with invalid memory handle!");
//    }
//
//    constexpr VkExternalMemoryHandleTypeFlagBits handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
//
//    // 创建与 sourceImage 相同配置的 VkImage（带 external 标记）
//    VkExternalMemoryImageCreateInfo externalImageInfo{};
//    externalImageInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
//    externalImageInfo.handleTypes = handleType;
//
//    VkImageCreateInfo imageInfo{};
//    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//    imageInfo.pNext = &externalImageInfo;
//    imageInfo.imageType = VK_IMAGE_TYPE_2D;
//    imageInfo.extent = {sourceImage.imageSize.x, sourceImage.imageSize.y, 1};
//    imageInfo.mipLevels = sourceImage.mipLevels;
//    imageInfo.arrayLayers = sourceImage.arrayLayers;
//    imageInfo.format = sourceImage.imageFormat;
//    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // 与导出侧一致
//    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    imageInfo.usage = sourceImage.imageUsage;
//    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//    if (device->getQueueFamilyNumber() > 1)
//    {
//        std::vector<uint32_t> qf(device->getQueueFamilyNumber());
//        for (size_t i = 0; i < qf.size(); ++i) qf[i] = static_cast<uint32_t>(i);
//        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
//        imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(qf.size());
//        imageInfo.pQueueFamilyIndices = qf.data();
//    }
//    else
//    {
//        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//    }
//
//    // 通过 VMA 的 dedicated import 创建
//    VkImportMemoryWin32HandleInfoKHR importMemInfo{VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR};
//    importMemInfo.handleType = handleType;
//    importMemInfo.handle = memHandle.handle;
//    importMemInfo.name = nullptr;
//
//    VmaAllocationCreateInfo importAllocCI{};
//    importAllocCI.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
//
//    ImageHardwareWrap imported{};
//    imported.device = this->device;
//    imported.resourceManager = this;
//    imported.pixelSize = sourceImage.pixelSize;
//    imported.imageSize = sourceImage.imageSize;
//    imported.imageFormat = sourceImage.imageFormat;
//    imported.arrayLayers = sourceImage.arrayLayers;
//    imported.mipLevels = sourceImage.mipLevels;
//    imported.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    imported.aspectMask = sourceImage.aspectMask;
//    imported.imageUsage = sourceImage.imageUsage;
//
//    if (vmaCreateDedicatedImage(g_hAllocator, &imageInfo, &importAllocCI, &importMemInfo, &imported.imageHandle, &imported.imageAlloc, &imported.imageAllocInfo) != VK_SUCCESS)
//    {
//        throw std::runtime_error("Failed to import image memory into dedicated image");
//    }
//
//    imported.imageView = createImageView(imported);
//    return imported;
//#else
//    (void)memHandle; (void)sourceImage;
//    throw std::runtime_error("Importing image memory is not implemented on this platform!");
//#endif
//}


void ResourceManager::createBindlessDescriptorSet()
{
    VkPhysicalDeviceDescriptorIndexingProperties indexingProperties = {};
    indexingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT;

    VkPhysicalDeviceProperties2 deviceProperties = {};
    deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties.pNext = &indexingProperties;

    vkGetPhysicalDeviceProperties2(device->physicalDevice, &deviceProperties);

    uint32_t k_max_bindless_resources[4] = {
        std::min({10000u,
                  indexingProperties.maxUpdateAfterBindDescriptorsInAllPools / 4,
                  indexingProperties.maxPerStageUpdateAfterBindResources / 4,
                  indexingProperties.maxPerStageDescriptorUpdateAfterBindUniformBuffers,
                  indexingProperties.maxDescriptorSetUpdateAfterBindUniformBuffers}),
        std::min({10000u,
                  indexingProperties.maxUpdateAfterBindDescriptorsInAllPools / 4,
                  indexingProperties.maxPerStageUpdateAfterBindResources / 4,
                  indexingProperties.maxPerStageDescriptorUpdateAfterBindSampledImages,
                  indexingProperties.maxDescriptorSetUpdateAfterBindSampledImages}),
        std::min({10000u,
                  indexingProperties.maxUpdateAfterBindDescriptorsInAllPools / 4,
                  indexingProperties.maxPerStageUpdateAfterBindResources / 4,
                  indexingProperties.maxPerStageDescriptorUpdateAfterBindStorageBuffers,
                  indexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffers}),
        std::min({10000u,
                  indexingProperties.maxUpdateAfterBindDescriptorsInAllPools / 4,
                  indexingProperties.maxPerStageUpdateAfterBindResources / 4,
                  indexingProperties.maxPerStageDescriptorUpdateAfterBindStorageImages,
                  indexingProperties.maxDescriptorSetUpdateAfterBindStorageImages})};


    std::vector<VkDescriptorSetLayoutBindingFlagsCreateInfo> descriptorSetLayoutBindingFlagsCreateInfo(4);
    std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfos(4);

    std::array<VkDescriptorSetLayoutBinding, 4> bindings{};
    std::array<VkDescriptorBindingFlags, 4> flags{};
    std::array<VkDescriptorType, 4> types{
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};

    for (uint32_t i = 0; i < 4; ++i)
    {
        bindings[i].descriptorCount = k_max_bindless_resources[i];
        bindings.at(i).binding = 0;
        bindings.at(i).descriptorType = types.at(i);
        bindings.at(i).stageFlags = VK_SHADER_STAGE_ALL;
        flags.at(i) = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

        descriptorSetLayoutBindingFlagsCreateInfo[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        descriptorSetLayoutBindingFlagsCreateInfo[i].pNext = nullptr;
        descriptorSetLayoutBindingFlagsCreateInfo[i].pBindingFlags = &flags[i];
        descriptorSetLayoutBindingFlagsCreateInfo[i].bindingCount = 1;

        descriptorSetLayoutCreateInfos[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfos[i].bindingCount = 1;
        descriptorSetLayoutCreateInfos[i].pBindings = &bindings[i];
        descriptorSetLayoutCreateInfos[i].flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        descriptorSetLayoutCreateInfos[i].pNext = &descriptorSetLayoutBindingFlagsCreateInfo[i];

        VkResult result = vkCreateDescriptorSetLayout(this->device->logicalDevice, &descriptorSetLayoutCreateInfos[i], nullptr, &bindlessDescriptors[i].descriptorSetLayout);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        {
            std::array<VkDescriptorPoolSize, 1> poolSizes{};
            poolSizes[0].type = types[i];
            poolSizes[0].descriptorCount = k_max_bindless_resources[i];

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = k_max_bindless_resources[i];
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

            if (vkCreateDescriptorPool(this->device->logicalDevice, &poolInfo, nullptr, &bindlessDescriptors[i].descriptorPool) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }

        {
            VkDescriptorSetVariableDescriptorCountAllocateInfo count_info{};
            count_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
            count_info.descriptorSetCount = 1;
            count_info.pDescriptorCounts = &k_max_bindless_resources[i];
            count_info.pNext = nullptr;

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = bindlessDescriptors[i].descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &bindlessDescriptors[i].descriptorSetLayout;
            allocInfo.pNext = &count_info;

            VkResult result = vkAllocateDescriptorSets(this->device->logicalDevice, &allocInfo, &bindlessDescriptors[i].descriptorSet);
            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }
        }
    }
}

uint32_t ResourceManager::storeDescriptor(ImageHardwareWrap image)
{
    std::unique_lock<std::mutex> lock(bindlessDescriptorMutex);

    uint32_t textureIndex = -1;

    VkDescriptorType descriptorType = (image.imageUsage & VK_IMAGE_USAGE_STORAGE_BIT) ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    bool updateDescriptorSets = false;

    if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
    {
        auto it = StorageImageBindingList.find(image.imageView);
        if (it != StorageImageBindingList.end())
        {
            textureIndex = it->second;
        }
        else
        {
            updateDescriptorSets = true;

            textureIndex = StorageImageBindingIndex++;
            StorageImageBindingList.insert(std::pair<VkImageView, int>(image.imageView, textureIndex));
        }
    }
    else
    {
        auto it = TextureBindingList.find(image.imageView);
        if (it != TextureBindingList.end())
        {
            textureIndex = it->second;
        }
        else
        {
            updateDescriptorSets = true;

            textureIndex = TextureBindingIndex++;
            TextureBindingList.insert(std::pair<VkImageView, int>(image.imageView, textureIndex));
        }
    }

    if (updateDescriptorSets)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageInfo.imageView = image.imageView;
        imageInfo.sampler = textureSampler;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = descriptorType;
        write.descriptorCount = 1;
        write.dstArrayElement = textureIndex;
        write.pImageInfo = &imageInfo;

        if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            write.dstSet = bindlessDescriptors[TextureBinding].descriptorSet;
            write.dstBinding = 0;
        }
        if (write.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
        {
            write.dstSet = bindlessDescriptors[StorageImageBinding].descriptorSet;
            write.dstBinding = 0;
        }

        vkUpdateDescriptorSets(this->device->logicalDevice, 1, &write, 0, nullptr);
    }

    return textureIndex;
}

uint32_t ResourceManager::storeDescriptor(BufferHardwareWrap buffer)
{
    std::unique_lock<std::mutex> lock(bindlessDescriptorMutex);

    uint32_t bufferIndex = -1;

    VkDescriptorType descriptorType = (buffer.bufferUsage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    bool updateDescriptorSets = false;

    if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
    {
        auto it = StorageBufferBindingList.find(buffer.bufferHandle);
        if (it != StorageBufferBindingList.end())
        {
            bufferIndex = it->second;
        }
        else
        {
            updateDescriptorSets = true;

            bufferIndex = StorageBufferBindingIndex++;
            StorageBufferBindingList.insert(std::pair<VkBuffer, int>(buffer.bufferHandle, bufferIndex));
        }
    }
    else
    {
        auto it = UniformBindingList.find(buffer.bufferHandle);
        if (it != UniformBindingList.end())
        {
            bufferIndex = it->second;
        }
        else
        {
            updateDescriptorSets = true;

            bufferIndex = UniformBindingIndex++;
            UniformBindingList.insert(std::pair<VkBuffer, int>(buffer.bufferHandle, bufferIndex));
        }
    }

    if (updateDescriptorSets)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer.bufferHandle;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet writes{};
        writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes.descriptorCount = 1;
        writes.pBufferInfo = &bufferInfo;
        writes.descriptorType = descriptorType;
        writes.dstArrayElement = bufferIndex;

        if (writes.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            writes.dstSet = bindlessDescriptors[UniformBinding].descriptorSet;
            writes.dstBinding = 0;
        }
        if (writes.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
        {
            writes.dstSet = bindlessDescriptors[StorageBufferBinding].descriptorSet;
            writes.dstBinding = 0;
        }

        vkUpdateDescriptorSets(this->device->logicalDevice, 1, &writes, 0, nullptr);
    }

    return bufferIndex;
}

// uint32_t ResourceManager::storeDescriptor(VkAccelerationStructureKHR m_tlas)
//{
//	uint32_t asIndex = -1;
//
//	{
//		auto it = accelerationStructureBindingList.find(m_tlas);
//		if (it != accelerationStructureBindingList.end())
//		{
//			asIndex = it->second;
//		}
//		else
//		{
//			asIndex = accelerationStructureBindingIndex++;
//			accelerationStructureBindingList.insert(std::pair<VkAccelerationStructureKHR, int>(m_tlas, asIndex));
//		}
//	}
//
//	VkWriteDescriptorSetAccelerationStructureKHR descASInfo = {};
//	descASInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
//	descASInfo.accelerationStructureCount = 1;
//	descASInfo.pAccelerationStructures = &m_tlas;
//
//	VkWriteDescriptorSet writes{};
//	writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//	writes.dstSet = bindlessDescriptor.descriptorSet;
//	writes.dstBinding = accelerationStructureBinding;
//	writes.dstArrayElement = asIndex;
//	writes.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
//	writes.descriptorCount = 1;
//	writes.pImageInfo = VK_NULL_HANDLE;
//	writes.pBufferInfo = VK_NULL_HANDLE;
//	writes.pNext = &descASInfo;
//
//	vkUpdateDescriptorSets(deviceManager.mainDevice.logicalDevice, 1, &writes, 0, nullptr);
//
//	return asIndex;
//}

VkShaderModule ResourceManager::createShaderModule(const std::vector<unsigned int> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(unsigned int);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(this->device->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

ResourceManager &ResourceManager::blitImage(VkCommandBuffer &commandBuffer, ImageHardwareWrap &srcImage, ImageHardwareWrap &dstImage)
{

        VkImageBlit imageBlit;
        imageBlit.dstOffsets[0] = VkOffset3D{0, 0, 0};
        imageBlit.dstOffsets[1] = VkOffset3D{int32_t(dstImage.imageSize.x), int32_t(dstImage.imageSize.y), 1};

        imageBlit.srcOffsets[0] = VkOffset3D{0, 0, 0};
        imageBlit.srcOffsets[1] = VkOffset3D{int32_t(srcImage.imageSize.x), int32_t(srcImage.imageSize.y), 1};

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        imageBlit.dstSubresource.baseArrayLayer = 0;
        imageBlit.srcSubresource.baseArrayLayer = 0;

        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.srcSubresource.layerCount = 1;

        imageBlit.dstSubresource.mipLevel = 0;
        imageBlit.srcSubresource.mipLevel = 0;

        vkCmdBlitImage(
            commandBuffer,
            srcImage.imageHandle, srcImage.imageLayout,
            dstImage.imageHandle, dstImage.imageLayout,
            1, &imageBlit,
            VK_FILTER_LINEAR);

    return *this;

}