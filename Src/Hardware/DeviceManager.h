#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__) || defined(__unix__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
#else
#error "Platform not supported by this example."
#endif

#define VK_NO_PROTOTYPES
#include <volk.h>

#include "FeaturesChain.h"

class DeviceManager
{
  public:
    struct ExternalSemaphoreHandle
    {
#if _WIN32 || _WIN64
        HANDLE handle = nullptr;
#else
        int fd = -1;
#endif
    };

    struct QueueUtils
    {
        std::shared_ptr<std::mutex> queueMutex;
        std::shared_ptr<std::atomic_uint64_t> timelineValue;
        VkSemaphore timelineSemaphore = VK_NULL_HANDLE;
        uint32_t queueFamilyIndex = -1;
        VkQueue vkQueue = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        DeviceManager *deviceManager = nullptr;
    };

    struct FeaturesUtils
    {
        std::set<const char *> instanceExtensions;
        std::set<const char *> deviceExtensions;
        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties{};
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
        VkPhysicalDeviceProperties2 supportedProperties{};
        DeviceFeaturesChain featuresChain;
    };

    DeviceManager();
    ~DeviceManager();

    DeviceManager(const DeviceManager &) = delete;
    DeviceManager &operator=(const DeviceManager &) = delete;
    DeviceManager(DeviceManager &&) = delete;
    DeviceManager &operator=(DeviceManager &&) = delete;

    void initDeviceManager(const CreateCallback &createCallback, const VkInstance &vkInstance, const VkPhysicalDevice &physicalDevice);
    void cleanUpDeviceManager();

    ExternalSemaphoreHandle exportSemaphore(VkSemaphore &semaphore);
    VkSemaphore importSemaphore(const ExternalSemaphoreHandle &memHandle, const VkSemaphore &semaphore);

    std::vector<QueueUtils> pickAvailableQueues(std::function<bool(const QueueUtils &)> predicate) const;

    VkPhysicalDevice getPhysicalDevice() const
    {
        return physicalDevice;
    }
    VkDevice getLogicalDevice() const
    {
        return logicalDevice;
    }
    const FeaturesUtils &getFeaturesUtils() const
    {
        return deviceFeaturesUtils;
    }
    FeaturesUtils &getFeaturesUtils()
    {
        return deviceFeaturesUtils;
    }
    uint16_t getQueueFamilyNumber() const
    {
        return static_cast<uint16_t>(queueFamilies.size());
    }

    bool operator==(const DeviceManager &other) const
    {
        return physicalDevice == other.physicalDevice && logicalDevice == other.logicalDevice;
    }

  private:
    friend class HardwareExecutor;

    void createDevices(const CreateCallback &createInfo, const VkInstance &vkInstance);
    void chooseMainDevice();
    void choosePresentQueueFamily();
    bool createCommandBuffers();
    void createTimelineSemaphore();

    void destroyQueueResources(std::vector<QueueUtils> &queues);

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    FeaturesUtils deviceFeaturesUtils;

    std::atomic_uint16_t currentGraphicsQueueIndex{0};
    std::atomic_uint16_t currentComputeQueueIndex{0};
    std::atomic_uint16_t currentTransferQueueIndex{0};

    std::vector<QueueUtils> graphicsQueues;
    std::vector<QueueUtils> computeQueues;
    std::vector<QueueUtils> transferQueues;
    std::vector<VkQueueFamilyProperties> queueFamilies;
};