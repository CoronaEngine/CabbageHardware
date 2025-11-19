#include "DisplayManager.h"
#include "Hardware/ResourceCommand.h"
#include "corona/kernel/memory/cache_aligned_allocator.h"

#define USE_SAME_DEVICE

DisplayManager::DisplayManager() = default;

DisplayManager::~DisplayManager()
{
    cleanUpDisplayManager();
}

void DisplayManager::cleanUpDisplayManager()
{
    VkDevice device = (displayDevice ? displayDevice->deviceManager.getLogicalDevice() : VK_NULL_HANDLE);

    if (device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(device);
    }

    // 按照正确的顺序清理资源
    cleanupSyncObjects();
    cleanupDisplayImage();
    cleanupStagingBuffers();
    cleanupSwapChainImages();

    // 清理交换链
    if (swapChain != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }

    // 清理 Surface（使用实例销毁）
    if (vkSurface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(globalHardwareContext.getVulkanInstance(), vkSurface, nullptr);
        vkSurface = VK_NULL_HANDLE;
    }

    // 清理宿主内存
    if (hostBufferPtr != nullptr)
    {
        Corona::Kernal::Memory::aligned_free(hostBufferPtr);
        hostBufferPtr = nullptr;
    }

    // 清理状态
    presentQueues.clear();
    mainDeviceExecutor.reset();
    displayDeviceExecutor.reset();
    waitedExecutor.reset();
    displaySurface = nullptr;
    displaySize = {0, 0};
    currentFrame = 0;
}

void DisplayManager::cleanupSyncObjects()
{
    VkDevice device = displayDevice ? displayDevice->deviceManager.getLogicalDevice() : VK_NULL_HANDLE;
    if (device == VK_NULL_HANDLE)
        return;

    for (auto &fence : inFlightFences)
    {
        if (fence != VK_NULL_HANDLE)
        {
            vkDestroyFence(device, fence, nullptr);
            fence = VK_NULL_HANDLE;
        }
    }
    inFlightFences.clear();

    for (auto &sem : imageAvailableSemaphores)
    {
        if (sem != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, sem, nullptr);
            sem = VK_NULL_HANDLE;
        }
    }
    imageAvailableSemaphores.clear();

    for (auto &sem : renderFinishedSemaphores)
    {
        if (sem != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device, sem, nullptr);
            sem = VK_NULL_HANDLE;
        }
    }
    renderFinishedSemaphores.clear();
}

void DisplayManager::cleanupSwapChainImages()
{
    VkDevice device = displayDevice ? displayDevice->deviceManager.getLogicalDevice() : VK_NULL_HANDLE;
    if (device == VK_NULL_HANDLE)
        return;

    for (auto &image : swapChainImages)
    {
        if (image.imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, image.imageView, nullptr);
            image.imageView = VK_NULL_HANDLE;
        }
        // 交换链图像由交换链管理，不需要手动销毁
        image.imageHandle = VK_NULL_HANDLE;
        image.imageAlloc = VK_NULL_HANDLE;
    }
    swapChainImages.clear();
}

void DisplayManager::cleanupStagingBuffers()
{
    if (srcStaging.bufferHandle != VK_NULL_HANDLE && srcStaging.resourceManager)
    {
        srcStaging.resourceManager->destroyBuffer(srcStaging);
        srcStaging = {};
    }

    if (dstStaging.bufferHandle != VK_NULL_HANDLE && dstStaging.resourceManager)
    {
        dstStaging.resourceManager->destroyBuffer(dstStaging);
        dstStaging = {};
    }
}

void DisplayManager::cleanupDisplayImage()
{
    if (displayImage.imageHandle != VK_NULL_HANDLE &&
        displayImage.imageAlloc != VK_NULL_HANDLE &&
        displayDevice)
    {
        //displayDevice->resourceManager.destroyImage(displayImage);
        displayImage = {};
    }
}

bool DisplayManager::initDisplayManager(void *surface)
{
    if (surface == nullptr)
    {
        return false;
    }

    try
    {
        createVkSurface(surface);
        choosePresentDevice();

        if (!displayDevice || presentQueues.empty())
        {
            throw std::runtime_error("Failed to find suitable present device/queue");
        }

        createSwapChain();
        createSyncObjects();
        return true;
    }
    catch (const std::exception &e)
    {
        // 初始化失败时清理
        cleanUpDisplayManager();
        return false;
    }
}

void DisplayManager::createSyncObjects()
{
    const size_t imageCount = swapChainImages.size();
    if (imageCount == 0)
    {
        throw std::runtime_error("Cannot create sync objects: no swapchain images");
    }

    imageAvailableSemaphores.resize(imageCount);
    renderFinishedSemaphores.resize(imageCount);
    inFlightFences.resize(imageCount);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice device = displayDevice->deviceManager.getLogicalDevice();

    for (size_t i = 0; i < imageCount; i++)
    {
        coronaHardwareCheck(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
        coronaHardwareCheck(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
        coronaHardwareCheck(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]));
    }
}

void DisplayManager::createVkSurface(void* surface)
{
    if (surface == nullptr)
    {
        throw std::runtime_error("Surface pointer is null");
    }

#if _WIN32 || _WIN64
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = static_cast<HWND>(surface);
    createInfo.hinstance = GetModuleHandle(nullptr);

    coronaHardwareCheck(vkCreateWin32SurfaceKHR(globalHardwareContext.getVulkanInstance(),
                                                &createInfo,
                                                nullptr,
                                                &vkSurface));

#elif __APPLE__
    VkMacOSSurfaceCreateInfoMVK createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pView = surface;

    coronaHardwareCheck(vkCreateMacOSSurfaceMVK(globalHardwareContext.getVulkanInstance(),
                                                &createInfo,
                                                nullptr,
                                                &vkSurface););
#elif __linux__
    // TODO: Linux surface creation
    throw std::runtime_error("Linux surface creation not implemented");
#endif
}

void DisplayManager::choosePresentDevice()
{
#ifdef USE_SAME_DEVICE
    displayDevice = globalHardwareContext.getMainDevice();

    auto pickQueuesRoles = [&](const DeviceManager::QueueUtils &queues) -> bool
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(displayDevice->deviceManager.getPhysicalDevice(),
                                             queues.queueFamilyIndex,
                                             vkSurface,
                                             &presentSupport);
        return presentSupport;
    };

    presentQueues = displayDevice->deviceManager.pickAvailableQueues(pickQueuesRoles);
#else
    // 优先选择与主设备不同的设备（如果支持）
    for (const auto &device : globalHardwareContext.getAllDevices())
    {
        auto pickQueuesRoles = [&](const DeviceManager::QueueUtils &queues) -> bool {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device->deviceManager.getPhysicalDevice(),
                                                 queues.queueFamilyIndex,
                                                 vkSurface,
                                                 &presentSupport);
            return presentSupport;
        };

        auto queues = device->deviceManager.pickAvailableQueues(pickQueuesRoles);
        if (!queues.empty())
        {
            displayDevice = device;
            presentQueues = std::move(queues);

            // 如果找到与主设备不同的设备，优先使用
            if (displayDevice != globalHardwareContext.getMainDevice())
            {
                break;
            }
        }
    }
#endif

    if (!displayDevice || presentQueues.empty())
    {
        throw std::runtime_error("Failed to find suitable present device");
    }

    mainDeviceExecutor = std::make_shared<HardwareExecutorVulkan>(globalHardwareContext.getMainDevice());
    displayDeviceExecutor = std::make_shared<HardwareExecutorVulkan>(displayDevice);
}

void DisplayManager::createSwapChain()
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        displayDevice->deviceManager.getPhysicalDevice(), vkSurface, &capabilities);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get surface capabilities");
    }

    displaySize = ktm::uvec2{
        std::clamp(capabilities.currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(capabilities.currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};

    // 选择 Surface Format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(displayDevice->deviceManager.getPhysicalDevice(),
                                         vkSurface, &formatCount, nullptr);

    if (formatCount == 0)
    {
        throw std::runtime_error("No surface formats available");
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(displayDevice->deviceManager.getPhysicalDevice(),
                                         vkSurface, &formatCount, formats.data());

    surfaceFormat = formats[0];
    for (const auto &availableFormat : formats)
    {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = availableFormat;
            break;
        }
    }

    // 选择 Present Mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(displayDevice->deviceManager.getPhysicalDevice(),
                                              vkSurface, &presentModeCount, nullptr);

    if (presentModeCount > 0)
    {
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(displayDevice->deviceManager.getPhysicalDevice(),
                                                  vkSurface, &presentModeCount, presentModes.data());

        for (const auto &mode : presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = mode;
                break;
            }
        }
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = {displaySize.x, displaySize.y};
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if ((capabilities.supportedUsageFlags & createInfo.imageUsage) != createInfo.imageUsage)
    {
        throw std::runtime_error("Swapchain does not support required image usage flags");
    }

    std::vector<uint32_t> queueFamilies(displayDevice->deviceManager.getQueueFamilyNumber());
    std::iota(queueFamilies.begin(), queueFamilies.end(), 0);

    if (queueFamilies.size() > 1)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
        createInfo.pQueueFamilyIndices = queueFamilies.data();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = swapChain;

    VkSwapchainKHR oldSwapChain = swapChain;
    result = vkCreateSwapchainKHR(displayDevice->deviceManager.getLogicalDevice(),
                                  &createInfo, nullptr, &swapChain);

    // 清理旧的交换链
    if (oldSwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(displayDevice->deviceManager.getLogicalDevice(), oldSwapChain, nullptr);
    }

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain");
    }

    // 获取交换链图像
    vkGetSwapchainImagesKHR(displayDevice->deviceManager.getLogicalDevice(),
                            swapChain, &imageCount, nullptr);

    std::vector<VkImage> swapChainVkImages(imageCount);
    vkGetSwapchainImagesKHR(displayDevice->deviceManager.getLogicalDevice(),
                            swapChain, &imageCount, swapChainVkImages.data());

    swapChainImages.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        swapChainImages[i].imageHandle = swapChainVkImages[i];
        swapChainImages[i].imageSize = displaySize;
        swapChainImages[i].imageFormat = surfaceFormat.format;
        swapChainImages[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        swapChainImages[i].arrayLayers = 1;
        swapChainImages[i].mipLevels = 1;
        swapChainImages[i].device = &displayDevice->deviceManager;
        swapChainImages[i].resourceManager = &displayDevice->resourceManager;
        swapChainImages[i].pixelSize = 4; // RGBA8

        swapChainImages[i].imageView = displayDevice->resourceManager.createImageView(swapChainImages[i]);
    }
}

void DisplayManager::recreateSwapChain()
{
    vkDeviceWaitIdle(displayDevice->deviceManager.getLogicalDevice());

    cleanupSwapChainImages();
    createSwapChain();
}

bool DisplayManager::needsSwapChainRecreation(const ktm::uvec2 &newSize) const
{
    return newSize != displaySize;
}

void DisplayManager::setupCrossDeviceTransfer(const ResourceManager::ImageHardwareWrap &sourceImage)
{
    VkDeviceSize imageSizeBytes = static_cast<VkDeviceSize>(sourceImage.imageSize.x) *
                                  sourceImage.imageSize.y * sourceImage.pixelSize;

    // 计算对齐要求
    uint64_t requiredAlign = 1;
    {
        VkPhysicalDeviceExternalMemoryHostPropertiesEXT hostProps{};
        hostProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT;

        VkPhysicalDeviceProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props2.pNext = &hostProps;

        vkGetPhysicalDeviceProperties2(globalHardwareContext.getMainDevice()->deviceManager.getPhysicalDevice(), &props2);
        requiredAlign = std::max(requiredAlign, hostProps.minImportedHostPointerAlignment);

        vkGetPhysicalDeviceProperties2(displayDevice->deviceManager.getPhysicalDevice(), &props2);
        requiredAlign = std::max(requiredAlign, hostProps.minImportedHostPointerAlignment);
    }

    // 分配宿主内存
    if (hostBufferPtr != nullptr)
    {
        Corona::Kernal::Memory::aligned_free(hostBufferPtr);
    }
    hostBufferPtr = Corona::Kernal::Memory::aligned_malloc(imageSizeBytes, requiredAlign);

    if (hostBufferPtr == nullptr)
    {
        throw std::runtime_error("Failed to allocate host buffer");
    }

    // 创建源和目标暂存缓冲区
    cleanupStagingBuffers();

    srcStaging = globalHardwareContext.getMainDevice()->resourceManager.importHostBuffer(hostBufferPtr, imageSizeBytes);
    dstStaging = displayDevice->resourceManager.importHostBuffer(hostBufferPtr, imageSizeBytes);

    /*srcStaging = globalHardwareContext.getMainDevice()->resourceManager.createBuffer(imageSizeBytes, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, true);
    ResourceManager::ExternalMemoryHandle memHandle = globalHardwareContext.getMainDevice()->resourceManager.exportBufferMemory(srcStaging);
    dstStaging = displayDevice->resourceManager.importBufferMemory(memHandle, srcStaging.elementCount, srcStaging.elementSize,srcStaging.bufferAllocInfo.size,srcStaging.bufferUsage);*/
}

bool DisplayManager::waitExecutor(HardwareExecutorVulkan &executor)
{
    waitedExecutor = std::make_shared<HardwareExecutorVulkan>(executor);
    return true;
}

bool DisplayManager::displayFrame(void *surface, HardwareImage displayImage)
{
    if (surface == nullptr)
    {
        return false;
    }

    try
    {
        ResourceManager::ImageHardwareWrap sourceImage = getImageFromHandle(*displayImage.getImageID());

        // 检查是否需要重新初始化
        if (this->displaySurface != surface)
        {
            this->displaySurface = surface;

            if (vkSurface != VK_NULL_HANDLE || swapChain != VK_NULL_HANDLE)
            {
                cleanUpDisplayManager();
            }

            if (!initDisplayManager(surface))
            {
                return false;
            }

            // 设置跨设备传输
            if (globalHardwareContext.getMainDevice() != displayDevice)
            //if (true)
            {
                this->displayImage = displayDevice->resourceManager.createImage(sourceImage.imageSize, sourceImage.imageFormat,sourceImage.pixelSize, sourceImage.imageUsage);

                setupCrossDeviceTransfer(sourceImage);
            }
            else
            {
                this->displayImage = sourceImage;
            }
        }

        // 等待之前的执行器
        if (waitedExecutor)
        {
            mainDeviceExecutor->wait(*waitedExecutor);
            displayDeviceExecutor->wait(*waitedExecutor);
        }

        // 检查交换链是否需要重建
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(displayDevice->deviceManager.getPhysicalDevice(), vkSurface, &capabilities);

        ktm::uvec2 newSize{
            std::clamp(capabilities.currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(capabilities.currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};

        if (needsSwapChainRecreation(newSize))
        {
            recreateSwapChain();
        }

        // 等待前一帧完成
        vkWaitForFences(displayDevice->deviceManager.getLogicalDevice(),
                        1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // 获取下一个交换链图像
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            displayDevice->deviceManager.getLogicalDevice(), swapChain, UINT64_MAX,
            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return true;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swap chain image");
        }

        vkResetFences(displayDevice->deviceManager.getLogicalDevice(), 1, &inFlightFences[currentFrame]);

        // 跨设备传输（如果需要）
        if (globalHardwareContext.getMainDevice() != displayDevice)
        //if (true)
        {
            CopyImageToBufferCommand copyCmd(sourceImage, srcStaging);
            (*mainDeviceExecutor) << &copyCmd << mainDeviceExecutor->commit();


            //std::vector<uint8_t> zeroData(dstStaging.bufferAllocInfo.size, 0);
            //displayDevice->resourceManager.copyBufferToHost(dstStaging, zeroData.data(), dstStaging.bufferAllocInfo.size);
            //vkDeviceWaitIdle(displayDevice->deviceManager.getLogicalDevice());

            CopyBufferToImageCommand copyCmd2(dstStaging, this->displayImage);
            (*displayDeviceExecutor) << &copyCmd2;

        }

        // 设置同步信息
        std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos;
        {
            VkSemaphoreSubmitInfo waitInfo{};
            waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            waitInfo.semaphore = imageAvailableSemaphores[currentFrame];
            waitInfo.value = 0;
            waitInfo.stageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
            waitSemaphoreInfos.push_back(waitInfo);
        }

        std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos;
        {
            VkSemaphoreSubmitInfo signalInfo{};
            signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            signalInfo.semaphore = renderFinishedSemaphores[currentFrame];
            signalInfo.value = 0;
            signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            signalSemaphoreInfos.push_back(signalInfo);
        }

        // 执行 Blit 和转换布局
        BlitImageCommand blitCmd(this->displayImage, swapChainImages[imageIndex]);
        TransitionImageLayoutCommand transitionCmd(
            swapChainImages[imageIndex], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_NONE);

        *displayDeviceExecutor << &blitCmd << &transitionCmd
                               << displayDeviceExecutor->wait(waitSemaphoreInfos, signalSemaphoreInfos, inFlightFences[currentFrame])
                               << displayDeviceExecutor->commit();

        // 呈现
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain;
        presentInfo.pImageIndices = &imageIndex;

        auto commitToQueue = [&](DeviceManager::QueueUtils *currentRecordQueue) -> bool {
            VkResult queueResult = vkQueuePresentKHR(currentRecordQueue->vkQueue, &presentInfo);
            return (queueResult == VK_SUCCESS || queueResult == VK_SUBOPTIMAL_KHR);
        };

        HardwareExecutorVulkan::pickQueueAndCommit(currentQueueIndex, presentQueues, commitToQueue);

        currentFrame = (currentFrame + 1) % swapChainImages.size();
        return true;
    }
    catch (const std::exception &e)
    {
        // 错误处理
        return false;
    }
}