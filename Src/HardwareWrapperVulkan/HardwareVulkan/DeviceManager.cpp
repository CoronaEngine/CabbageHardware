#include "DeviceManager.h"

#include <algorithm>

DeviceManager::DeviceManager() = default;

DeviceManager::~DeviceManager() {
    cleanUpDeviceManager();
}

void DeviceManager::initDeviceManager(const CreateCallback& createCallback, const VkInstance& vkInstance, const VkPhysicalDevice& physicalDevice) {
    this->physicalDevice = physicalDevice;

    createDevices(createCallback, vkInstance);
    choosePresentQueueFamily();
    createCommandBuffers();
    createTimelineSemaphore();
}

void DeviceManager::cleanUpDeviceManager() {
    if (logicalDevice == VK_NULL_HANDLE) {
        graphicsQueues.clear();
        computeQueues.clear();
        transferQueues.clear();
        queueFamilies.clear();
        physicalDevice = VK_NULL_HANDLE;
        currentGraphicsQueueIndex = 0;
        currentComputeQueueIndex = 0;
        currentTransferQueueIndex = 0;
        return;
    }

    vkDeviceWaitIdle(logicalDevice);

    destroyQueueResources(graphicsQueues);
    destroyQueueResources(computeQueues);
    destroyQueueResources(transferQueues);

    queueFamilies.clear();

    vkDestroyDevice(logicalDevice, nullptr);
    logicalDevice = VK_NULL_HANDLE;
    physicalDevice = VK_NULL_HANDLE;

    currentGraphicsQueueIndex = 0;
    currentComputeQueueIndex = 0;
    currentTransferQueueIndex = 0;
}

void DeviceManager::destroyQueueResources(std::vector<QueueUtils>& queues) {
    for (auto& queue : queues) {
        if (queue.commandBuffer != VK_NULL_HANDLE && queue.commandPool != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(logicalDevice, queue.commandPool, 1, &queue.commandBuffer);
            queue.commandBuffer = VK_NULL_HANDLE;
        }

        if (queue.commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(logicalDevice, queue.commandPool, nullptr);
            queue.commandPool = VK_NULL_HANDLE;
        }

        if (queue.timelineSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(logicalDevice, queue.timelineSemaphore, nullptr);
            queue.timelineSemaphore = VK_NULL_HANDLE;
        }

        queue.vkQueue = VK_NULL_HANDLE;
        // q.timelineValue.store(0);
        queue.queueFamilyIndex = static_cast<uint32_t>(-1);
        queue.queueMutex.reset();
        queue.deviceManager = nullptr;
    }

    queues.clear();
}

void DeviceManager::createTimelineSemaphore() {
    auto createTimelineSemaphoreForQueue = [&](QueueUtils& queue) {
        VkExportSemaphoreCreateInfo exportInfo{};
        exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
        exportInfo.pNext = nullptr;
#if _WIN32 || _WIN64
        exportInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#endif
        VkSemaphoreTypeCreateInfo typeCreateInfo{};
        typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
        typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
        typeCreateInfo.initialValue = 0;
        typeCreateInfo.pNext = &exportInfo;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &typeCreateInfo;

        coronaHardwareCheck(vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &queue.timelineSemaphore));
    };

    for (auto& queue : graphicsQueues)
        createTimelineSemaphoreForQueue(queue);
    for (auto& queue : computeQueues)
        createTimelineSemaphoreForQueue(queue);
    for (auto& queue : transferQueues)
        createTimelineSemaphoreForQueue(queue);
}

void DeviceManager::createDevices(const CreateCallback& initInfo, const VkInstance& vkInstance) {
    std::set<const char*> inputExtensions = initInfo.requiredDeviceExtensions(vkInstance, physicalDevice);
    std::vector<const char*> requiredExtensions = std::vector<const char*>(inputExtensions.begin(), inputExtensions.end());

    deviceFeaturesUtils.supportedProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceFeaturesUtils.rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
    deviceFeaturesUtils.accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

    deviceFeaturesUtils.rayTracingPipelineProperties.pNext = &deviceFeaturesUtils.accelerationStructureProperties;
    deviceFeaturesUtils.supportedProperties.pNext = &deviceFeaturesUtils.rayTracingPipelineProperties;
    deviceFeaturesUtils.accelerationStructureProperties.pNext = nullptr;

    vkGetPhysicalDeviceProperties2(physicalDevice, &deviceFeaturesUtils.supportedProperties);
    printDeviceInfo(deviceFeaturesUtils.supportedProperties.properties);

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    requiredExtensions.erase(std::remove_if(requiredExtensions.begin(), requiredExtensions.end(),
                                            [&availableExtensions](const char* required) {
                                                bool supported = std::any_of(availableExtensions.begin(), availableExtensions.end(),
                                                                             [required](const VkExtensionProperties& available) {
                                                                                 return strcmp(required, available.extensionName) == 0;
                                                                             });

                                                if (!supported) {
                                                    printExtensionWarning(required);
                                                }
                                                return !supported;
                                            }),
                             requiredExtensions.end());

    vkGetPhysicalDeviceFeatures2(physicalDevice, deviceFeaturesUtils.featuresChain.getChainHead());
    deviceFeaturesUtils.featuresChain = deviceFeaturesUtils.featuresChain & initInfo.requiredDeviceFeatures(vkInstance, physicalDevice);

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    queueFamilies.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<std::vector<float>> queuePriorities(queueFamilies.size());

    for (int i = 0; i < queueFamilies.size(); i++) {
        queuePriorities[i].resize(queueFamilies[i].queueCount, 1.0f);

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(i);
        queueCreateInfo.queueCount = queueFamilies[i].queueCount;
        queueCreateInfo.pQueuePriorities = queuePriorities[i].data();

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.pEnabledFeatures = nullptr;
    createInfo.pNext = deviceFeaturesUtils.featuresChain.getChainHead();

    coronaHardwareCheck(vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice));
}

void DeviceManager::choosePresentQueueFamily() {
    for (int i = 0; i < queueFamilies.size(); i++) {
        QueueUtils baseQueueUtils;
        baseQueueUtils.queueFamilyIndex = static_cast<uint32_t>(i);
        baseQueueUtils.deviceManager = this;
        // tempQueueUtils.timelineValue.store(0);

        for (uint32_t queueIndex = 0; queueIndex < queueFamilies[i].queueCount; queueIndex++) {
            QueueUtils queueUtils = baseQueueUtils;
            queueUtils.queueMutex = std::make_shared<std::mutex>();
            queueUtils.timelineValue = std::make_shared<std::atomic_uint64_t>(0);

            vkGetDeviceQueue(logicalDevice, static_cast<uint32_t>(i), queueIndex, &queueUtils.vkQueue);

            const VkQueueFlags flags = queueFamilies[i].queueFlags;
            if (flags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueues.push_back(queueUtils);
            } else if (flags & VK_QUEUE_COMPUTE_BIT) {
                computeQueues.push_back(queueUtils);
            } else if (flags & VK_QUEUE_TRANSFER_BIT) {
                transferQueues.push_back(queueUtils);
            }
        }
    }

    if (!graphicsQueues.empty()) {
        if (computeQueues.empty()) {
            computeQueues.push_back(graphicsQueues[0]);
        }
        if (transferQueues.empty()) {
            transferQueues.push_back(graphicsQueues[0]);
        }
    } else {
        throw std::runtime_error("No graphics queues found!");
    }
}

bool DeviceManager::createCommandBuffers() {
    auto createCommandBuffer = [this](QueueUtils& queue) {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queue.queueFamilyIndex;

        coronaHardwareCheck(vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &queue.commandPool));

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = queue.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        coronaHardwareCheck(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &queue.commandBuffer));
    };

    for (auto& queue : graphicsQueues) createCommandBuffer(queue);
    for (auto& queue : computeQueues) createCommandBuffer(queue);
    for (auto& queue : transferQueues) createCommandBuffer(queue);

    return true;
}

std::vector<DeviceManager::QueueUtils> DeviceManager::pickAvailableQueues(std::function<bool(const QueueUtils&)> predicate) const {
    std::vector<QueueUtils> result;

    auto addMatchingQueues = [&](const std::vector<QueueUtils>& queues) {
        for (const auto& queue : queues) {
            if (predicate(queue)) {
                result.push_back(queue);
            }
        }
    };

    addMatchingQueues(graphicsQueues);
    addMatchingQueues(computeQueues);
    addMatchingQueues(transferQueues);

    return result;
}

DeviceManager::ExternalSemaphoreHandle DeviceManager::exportSemaphore(VkSemaphore& semaphore) {
    ExternalSemaphoreHandle handleInfo{};

#if _WIN32 || _WIN64
    VkSemaphoreGetWin32HandleInfoKHR getHandleInfo{};
    getHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
    getHandleInfo.pNext = nullptr;
    getHandleInfo.semaphore = semaphore;
    getHandleInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    HANDLE handle = nullptr;
    coronaHardwareCheck(vkGetSemaphoreWin32HandleKHR(logicalDevice, &getHandleInfo, &handle));

    if (handle == nullptr) {
        throw std::runtime_error("Failed to export semaphore: handle is null");
    }

    handleInfo.handle = handle;
#else
    throw std::runtime_error("Semaphore export not implemented for this platform");
#endif

    return handleInfo;
}

VkSemaphore DeviceManager::importSemaphore(const DeviceManager::ExternalSemaphoreHandle& memHandle, const VkSemaphore& semaphore) {
#if _WIN32 || _WIN64
    VkImportSemaphoreWin32HandleInfoKHR importInfo{};
    importInfo.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR;
    importInfo.pNext = nullptr;
    importInfo.semaphore = semaphore;
    importInfo.flags = 0;
    importInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    importInfo.handle = memHandle.handle;
    importInfo.name = nullptr;

    coronaHardwareCheck(vkImportSemaphoreWin32HandleKHR(logicalDevice, &importInfo));

    return semaphore;
#else
    throw std::runtime_error("Semaphore import not implemented for this platform");
#endif
}