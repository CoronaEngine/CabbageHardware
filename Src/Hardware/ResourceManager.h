#pragma once

#include <unordered_map>
#include"DeviceManager.h"
#include<vk_mem_alloc.h>
#include <ktm/ktm.h>

//#include"HardwareExecutor.h"

class HardwareExecutor;

struct ResourceManager
{
    struct ExternalMemoryHandle
    {
#if _WIN32 || _WIN64
        HANDLE handle = nullptr;
#else
        int fd = -1;
#endif
    };

    struct BufferHardwareWrap
    {
        VkPipelineStageFlags stageMask;
        VkAccessFlags accessMask;

        VkBuffer bufferHandle = VK_NULL_HANDLE;
        VmaAllocation bufferAlloc = VK_NULL_HANDLE;
        VmaAllocationInfo bufferAllocInfo = {};
        VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;

        DeviceManager *device;
        ResourceManager *resourceManager;
    };

    struct ImageHardwareWrap
    {
        VkPipelineStageFlags stageMask;
        VkAccessFlags accessMask;

        uint32_t pixelSize = 0;
        ktm::uvec2 imageSize = ktm::uvec2(0, 0);
        VkFormat imageFormat = VK_FORMAT_MAX_ENUM;
        VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE;
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        int arrayLayers = 1;
        int mipLevels = 1;

        VkClearValue clearValue = {};

        VmaAllocation imageAlloc = VK_NULL_HANDLE;
        VmaAllocationInfo imageAllocInfo = {};

        VkImage imageHandle = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;

        DeviceManager *device;
        ResourceManager *resourceManager;
    };

    struct
    {
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    } bindlessDescriptors[4];

    ResourceManager();
    ~ResourceManager();

    void initResourceManager(DeviceManager &device);
    void cleanUpResourceManager();

    ImageHardwareWrap createImage(ktm::uvec2 imageSize, VkFormat imageFormat, uint32_t pixelSize,
                                  VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                  int arrayLayers = 1, int mipLevels = 1);
    VkImageView createImageView(ImageHardwareWrap &image);
    void destroyImage(ImageHardwareWrap &image);

    BufferHardwareWrap createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    void destroyBuffer(BufferHardwareWrap &buffer);

    uint32_t storeDescriptor(ImageHardwareWrap image);
    uint32_t storeDescriptor(BufferHardwareWrap buffer);
    //uint32_t storeDescriptor(VkAccelerationStructureKHR m_tlas);

    ResourceManager &copyBuffer(VkCommandBuffer &commandBuffer, BufferHardwareWrap &srcBuffer, BufferHardwareWrap &dstBuffer);
    ResourceManager &copyImage(VkCommandBuffer &commandBuffer, ImageHardwareWrap &source, ImageHardwareWrap &destination);

    ResourceManager &blitImage(VkCommandBuffer &commandBuffer, ImageHardwareWrap &srcImage, ImageHardwareWrap &dstImage);

    ResourceManager &copyBufferToImage(VkCommandBuffer &commandBuffer, BufferHardwareWrap& buffer, ImageHardwareWrap& image);
    ResourceManager &copyImageToBuffer(VkCommandBuffer &commandBuffer, ImageHardwareWrap& image, BufferHardwareWrap& buffer);

    void copyBufferToCpu(BufferHardwareWrap &buffer, void *cpuData);
    //void copyBufferToCpu(VkDevice &device, VkDeviceMemory &memory, VkDeviceSize size, void *cpuData);

    ExternalMemoryHandle exportBufferMemory(BufferHardwareWrap &sourceBuffer);
    BufferHardwareWrap importBufferMemory(const ExternalMemoryHandle &memHandle, const BufferHardwareWrap &sourceBuffer);
    void TestWin32HandlesImport(BufferHardwareWrap &srcStaging, BufferHardwareWrap &dstStaging, VkDeviceSize imageSizeBytes, ResourceManager &srcResourceManager, ResourceManager &dstResourceManager);

    uint32_t findExternalMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    //void transitionImageLayoutUnblocked(const VkCommandBuffer &commandBuffer, ImageHardwareWrap &image, VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

    ResourceManager &transitionImageLayout(VkCommandBuffer &commandBuffer, ImageHardwareWrap &image, VkImageLayout newLayout, VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);

    VkShaderModule createShaderModule(const std::vector<unsigned int> &code);

    uint64_t getHostSharedMemorySize()
    {
        return hostSharedMemorySize;
    }

    uint64_t getDeviceMemorySize()
    {
        return deviceMemorySize;
    }

    DeviceManager& getDeviceManager()
    {
        return *device;
    }

    VmaAllocator getVmaAllocator()
    {
        return g_hAllocator;
    }

    VmaPool getVmaPool()
    {
        return g_hPool;
    }

private:
    void createTextureSampler();
    void CreateVmaAllocator();
    void createBindlessDescriptorSet();
    void createExternalMemoryPool();

    VmaAllocator g_hAllocator;

    VmaPool g_hPool;

    VkSampler textureSampler;

    const uint32_t UniformBinding = 0;
    const uint32_t TextureBinding = 1;
    const uint32_t StorageBufferBinding = 2;
    const uint32_t StorageImageBinding = 3;

    std::unordered_map<VkBuffer, int> UniformBindingList;
    std::unordered_map<VkImageView, int> TextureBindingList;
    std::unordered_map<VkBuffer, int> StorageBufferBindingList;
    std::unordered_map<VkImageView, int> StorageImageBindingList;

    std::mutex bindlessDescriptorMutex;

    uint32_t UniformBindingIndex = 0;
    uint32_t TextureBindingIndex = 0;
    uint32_t StorageBufferBindingIndex = 0;
    uint32_t StorageImageBindingIndex = 0;

    uint64_t deviceMemorySize = 0;
    uint64_t hostSharedMemorySize = 0;
    uint64_t mutiInstanceMemorySize = 0;

    DeviceManager *device;
};