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
            q.timelineValue = 0;
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

        for (uint32_t queueIndex = 0; queueIndex < queueFamilies[i].queueCount; queueIndex++)
        {
            tempQueueUtils.queueMutex = std::make_shared<std::mutex>();

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

//DeviceManager &DeviceManager::startCommands(QueueType queueType)
//{
//    uint16_t queueIndex = 0;
//
//    while (true)
//    {
//        switch (queueType)
//        {
//        case QueueType::GraphicsQueue:
//            queueIndex = currentGraphicsQueueIndex.fetch_add(1) % graphicsQueues.size();
//            currentRecordQueue = &graphicsQueues[queueIndex];
//            break;
//        case QueueType::ComputeQueue:
//            queueIndex = currentComputeQueueIndex.fetch_add(1) % computeQueues.size();
//            currentRecordQueue = &computeQueues[queueIndex];
//            break;
//        case QueueType::TransferQueue:
//            queueIndex = currentTransferQueueIndex.fetch_add(1) % transferQueues.size();
//            currentRecordQueue = &transferQueues[queueIndex];
//            break;
//        }
//
//        if (currentRecordQueue->queueMutex->try_lock())
//        {
//            uint64_t timelineCounterValue = 0;
//            vkGetSemaphoreCounterValue(logicalDevice, currentRecordQueue->timelineSemaphore, &timelineCounterValue);
//            if (timelineCounterValue >= currentRecordQueue->timelineValue)
//            {
//                break;
//            }
//            else
//            {
//                currentRecordQueue->queueMutex->unlock();
//            }
//        }
//
//        std::this_thread::yield();
//    }
//
//    vkResetCommandBuffer(currentRecordQueue->commandBuffer, 0);
//
//    VkCommandBufferBeginInfo beginInfo{};
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//    vkBeginCommandBuffer(currentRecordQueue->commandBuffer, &beginInfo);
//
//    return *this;
//}
//
//DeviceManager &DeviceManager::endCommands(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos,
//                                          std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos,
//                                          VkFence fence)
//{
//    vkEndCommandBuffer(currentRecordQueue->commandBuffer);
//
//    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
//    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
//    commandBufferSubmitInfo.commandBuffer = currentRecordQueue->commandBuffer;
//
//    VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
//    timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
//    timelineWaitSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
//    timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue++;
//    timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
//    waitSemaphoreInfos.push_back(timelineWaitSemaphoreSubmitInfo);
//
//    VkSemaphoreSubmitInfo timelineSignalSemaphoreSubmitInfo{};
//    timelineSignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
//    timelineSignalSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
//    timelineSignalSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue;
//    timelineSignalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
//    signalSemaphoreInfos.push_back(timelineSignalSemaphoreSubmitInfo);
//
//    VkSubmitInfo2 submitInfo{};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
//    submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphoreInfos.size());
//    submitInfo.pWaitSemaphoreInfos = waitSemaphoreInfos.data();
//    submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphoreInfos.size());
//    submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfos.data();
//    submitInfo.commandBufferInfoCount = 1;
//    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;
//
//    VkResult result = vkQueueSubmit2(currentRecordQueue->vkQueue, 1, &submitInfo, fence);
//    if (result != VK_SUCCESS)
//    {
//        throw std::runtime_error("Failed to submit command buffer!");
//    }
//
//    currentRecordQueue->queueMutex->unlock();
//
//    return *this;
//}
//
//DeviceManager &DeviceManager::operator<<(std::function<void(const VkCommandBuffer &commandBuffer)> commandsFunction)
//{
//    commandsFunction(currentRecordQueue->commandBuffer);
//    return *this;
//}