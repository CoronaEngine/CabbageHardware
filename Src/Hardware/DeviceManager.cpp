#include "DeviceManager.h"
#include "../VulkanUtils.h"

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{
    cleanUpDeviceManager();
}

void DeviceManager::initDeviceManager(const CreateCallback &createCallback, const VkInstance &vkInstance, const VkPhysicalDevice &physicalDevice)
{
    this->physicalDevice = physicalDevice;

    createDevices(createCallback, vkInstance);
    choosePresentQueueFamily();
    createCommandBuffers();
    createTimelineSemaphore();
}

void DeviceManager::cleanUpDeviceManager()
{
    if (logicalDevice == VK_NULL_HANDLE)
    {
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

    auto destroyQueueResources = [&](std::vector<QueueUtils> &queues) {
        for (auto &q : queues)
        {
            if (q.commandBuffer != VK_NULL_HANDLE && q.commandPool != VK_NULL_HANDLE)
            {
                vkFreeCommandBuffers(logicalDevice, q.commandPool, 1, &q.commandBuffer);
                q.commandBuffer = VK_NULL_HANDLE;
            }
            if (q.commandPool != VK_NULL_HANDLE)
            {
                vkDestroyCommandPool(logicalDevice, q.commandPool, nullptr);
                q.commandPool = VK_NULL_HANDLE;
            }

            if (q.timelineSemaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(logicalDevice, q.timelineSemaphore, nullptr);
                q.timelineSemaphore = VK_NULL_HANDLE;
            }

            q.vkQueue = VK_NULL_HANDLE;
            //q.timelineValue.store(0);
            q.queueFamilyIndex = static_cast<uint32_t>(-1);
            q.queueMutex.reset();
            q.deviceManager = nullptr;
        }
        queues.clear();
    };

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

void DeviceManager::createTimelineSemaphore()
{
    auto createTimelineSemaphore = [&](QueueUtils &queues) {
        VkExportSemaphoreCreateInfo exportInfo{};
        exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
#if _WIN32 || _WIN64
        exportInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#endif
        exportInfo.pNext = nullptr;

        VkSemaphoreTypeCreateInfo type_create_info{};
        type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
        type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
        type_create_info.initialValue = 0;
        type_create_info.pNext = &exportInfo;

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &type_create_info;

        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &queues.timelineSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    };

    for (size_t i = 0; i < graphicsQueues.size(); i++)
    {
        createTimelineSemaphore(graphicsQueues[i]);
    }

    for (size_t i = 0; i < computeQueues.size(); i++)
    {
        createTimelineSemaphore(computeQueues[i]);
    }

    for (size_t i = 0; i < transferQueues.size(); i++)
    {
        createTimelineSemaphore(transferQueues[i]);
    }
}

void DeviceManager::createDevices(const CreateCallback &initInfo, const VkInstance &vkInstance)
{

    // userDevices.resize(deviceCount);
    // for (uint32_t index = 0; index < deviceCount; index++)
    {
        // physicalDevice = devices[index];

        std::set<const char *> inputExtensions = initInfo.requiredDeviceExtensions(vkInstance, physicalDevice);
        std::vector<const char *> requiredExtensions = std::vector<const char *>(inputExtensions.begin(), inputExtensions.end());

        deviceFeaturesUtils.supportedProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceFeaturesUtils.rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        deviceFeaturesUtils.accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
        deviceFeaturesUtils.rayTracingPipelineProperties.pNext = &deviceFeaturesUtils.accelerationStructureProperties;
        deviceFeaturesUtils.supportedProperties.pNext = &deviceFeaturesUtils.rayTracingPipelineProperties;
        deviceFeaturesUtils.accelerationStructureProperties.pNext = nullptr;

        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceFeaturesUtils.supportedProperties);
        std::cout << "---------- GPU " << " : " << deviceFeaturesUtils.supportedProperties.properties.deviceName << "----------" << std::endl;

        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

            for (size_t i = 0; i < requiredExtensions.size(); i++)
            {
                bool extensionSupported = false;
                for (size_t j = 0; j < availableExtensions.size(); j++)
                {
                    if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
                    {
                        extensionSupported = true;
                        break;
                    }
                }
                if (!extensionSupported)
                {
                    std::cerr << "      Extensions Warning : Device not support : " << requiredExtensions[i] << std::endl;

                    requiredExtensions.erase(requiredExtensions.begin() + i);
                    i--;
                }
            }
        }

        vkGetPhysicalDeviceFeatures2(physicalDevice, deviceFeaturesUtils.featuresChain.getChainHead());

        deviceFeaturesUtils.featuresChain = deviceFeaturesUtils.featuresChain & initInfo.requiredDeviceFeatures(vkInstance, physicalDevice);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        queueFamilies.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::vector<std::vector<float>> queuePriorities(queueFamilies.size());
        for (int i = 0; i < queueFamilies.size(); i++)
        {
            queuePriorities[i].resize(queueFamilies[i].queueCount);
            for (int j = 0; j < queuePriorities[i].size(); j++)
            {
                queuePriorities[i][j] = 1.0f;
            }

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = i;
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
}

void DeviceManager::choosePresentQueueFamily()
{
    for (int i = 0; i < queueFamilies.size(); i++)
    {
        QueueUtils tempQueueUtils;
        tempQueueUtils.queueFamilyIndex = i;
        tempQueueUtils.deviceManager = this;
        //tempQueueUtils.timelineValue.store(0);

        for (uint32_t queueIndex = 0; queueIndex < queueFamilies[i].queueCount; queueIndex++)
        {
            tempQueueUtils.queueMutex = std::make_shared<std::mutex>();
            tempQueueUtils.timelineValue = std::make_shared<std::atomic_uint64_t>(0);

            vkGetDeviceQueue(logicalDevice, i, queueIndex, &tempQueueUtils.vkQueue);

            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                graphicsQueues.push_back(tempQueueUtils);
            }
            else if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                computeQueues.push_back(tempQueueUtils);
            }
            else if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                transferQueues.push_back(tempQueueUtils);
            }
        }
    }

    if (!graphicsQueues.empty())
    {
        if (computeQueues.empty())
        {
            computeQueues.push_back(graphicsQueues[0]);
        }
        if (transferQueues.empty())
        {
            transferQueues.push_back(graphicsQueues[0]);
        }
    }
    else
    {
        throw std::runtime_error("No graphics queues found!");
    }
}

bool DeviceManager::createCommandBuffers()
{
    auto createCommand = [&](DeviceManager::QueueUtils &queues) {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queues.queueFamilyIndex;
        VkResult result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &queues.commandPool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = queues.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        coronaHardwareCheck(vkAllocateCommandBuffers(logicalDevice, &allocInfo, &queues.commandBuffer));
    };

    for (size_t i = 0; i < graphicsQueues.size(); i++)
    {
        createCommand(graphicsQueues[i]);
    }

    for (size_t i = 0; i < computeQueues.size(); i++)
    {
        createCommand(computeQueues[i]);
    }

    for (size_t i = 0; i < transferQueues.size(); i++)
    {
        createCommand(transferQueues[i]);
    }

    return true;
}

DeviceManager::ExternalSemaphoreHandle DeviceManager::exportSemaphore(VkSemaphore &semaphore)
{
    ExternalSemaphoreHandle handleInfo{};

#if _WIN32 || _WIN64
    VkSemaphoreGetWin32HandleInfoKHR getHandleInfo{};
    getHandleInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
    getHandleInfo.pNext = nullptr;
    getHandleInfo.semaphore = semaphore;
    getHandleInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;

    HANDLE handle = nullptr;
    VkResult result = vkGetSemaphoreWin32HandleKHR(logicalDevice, &getHandleInfo, &handle);
    if (result != VK_SUCCESS || handle == nullptr)
    {
        throw std::runtime_error("Failed to export semaphore handle.");
    }
    handleInfo.handle = handle;
#else
#endif

    return handleInfo;
}

VkSemaphore DeviceManager::importSemaphore(const DeviceManager::ExternalSemaphoreHandle &memHandle, const VkSemaphore &semaphore)
{
#if _WIN32 || _WIN64
    VkImportSemaphoreWin32HandleInfoKHR importInfo{};
    importInfo.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR;
    importInfo.pNext = nullptr;
    importInfo.semaphore = semaphore;
    importInfo.flags = 0; // VK_SEMAPHORE_IMPORT_TEMPORARY_BIT 或 0
    importInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    importInfo.handle = memHandle.handle;
    importInfo.name = nullptr;

    VkResult result = vkImportSemaphoreWin32HandleKHR(logicalDevice, &importInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to import semaphore handle.");
    }
    return semaphore;
#else
    return VK_NULL_HANDLE;
#endif
}