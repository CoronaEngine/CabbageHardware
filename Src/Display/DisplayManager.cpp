#include"DisplayManager.h"
#include <algorithm>
#include<vector>
#include<volk.h>
#include<Hardware/GlobalContext.h>
#include<Hardware/ResourceCommand.h>

//#define USE_SAME_DEVICE

//#if _WIN32 || _WIN64
//#include<vulkan/vulkan_win32.h>
//#elif __APPLE__
//#include<vulkan/vulkan_macos.h>
//#elif __linux__
//#include<vulkan/vulkan_xcb.h>
//#endif

DisplayManager::DisplayManager()
{
}

DisplayManager::~DisplayManager()
{
    cleanUpDisplayManager();
}

void DisplayManager::cleanUpDisplayManager()
{
    if (hostBufferPtr != nullptr)
    {
        free(hostBufferPtr);
        hostBufferPtr = nullptr;
    }

    // 即便 displayDevice 为空，也尽可能释放 surface 等与实例相关的资源
    VkDevice device = (displayDevice ? displayDevice->deviceManager.logicalDevice : VK_NULL_HANDLE);

    if (device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(device);
    }

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

    if (displayImage.imageHandle != VK_NULL_HANDLE && displayImage.imageAlloc != VK_NULL_HANDLE)
    {
        //displayDevice->resourceManager.destroyImage(displayImage);
        displayImage = {};
    }

    if (srcStaging.bufferHandle != VK_NULL_HANDLE)
    {
        if (srcStaging.resourceManager)
        {
            srcStaging.resourceManager->destroyBuffer(srcStaging);
        }
        srcStaging = {};
    }
    if (dstStaging.bufferHandle != VK_NULL_HANDLE)
    {
        if (dstStaging.resourceManager)
        {
            dstStaging.resourceManager->destroyBuffer(dstStaging);
        }
        dstStaging = {};
    }

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        if (swapChainImages[i].imageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, swapChainImages[i].imageView, nullptr);
            swapChainImages[i].imageView = VK_NULL_HANDLE;
        }

        swapChainImages[i].imageAlloc = VK_NULL_HANDLE;
    }
    swapChainImages.clear();

    if (swapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }

    if (vkSurface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(globalHardwareContext.getVulkanInstance(), vkSurface, nullptr);
        vkSurface = VK_NULL_HANDLE;
    }

    // 清理执行器和状态
    presentQueues.clear();
    mainDeviceExecutor.reset();
    displayDeviceExecutor.reset();
    //displayDevice.reset();
    displaySurface = nullptr;
    displaySize = {0, 0};
}

bool DisplayManager::initDisplayManager(void* surface)
{
    if (surface != nullptr)
    {
        createVkSurface(surface);

        choosePresentDevice();

        createSwapChain();

        createSyncObjects();
    }

    return true;
}

void DisplayManager::createSyncObjects()
{
    imageAvailableSemaphores.resize(swapChainImages.size());
    renderFinishedSemaphores.resize(swapChainImages.size());
    inFlightFences.resize(swapChainImages.size());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        if (vkCreateSemaphore(displayDevice->deviceManager.logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(displayDevice->deviceManager.logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(displayDevice->deviceManager.logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void DisplayManager::createVkSurface(void *surface)
{
#if _WIN32 || _WIN64
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = (HWND)(surface);
    createInfo.hinstance = GetModuleHandle(NULL);

    if (vkCreateWin32SurfaceKHR(globalHardwareContext.getVulkanInstance(), &createInfo, nullptr, &vkSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

#elif __APPLE__
    VkMacOSSurfaceCreateInfoMVK createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pView = surface;

    if (vkCreateMacOSSurfaceMVK(deviceManager.getVulkanInstance(), &createInfo, nullptr, &vkSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

#elif __linux__

#endif
}

void DisplayManager::choosePresentDevice()
{
#ifdef USE_SAME_DEVICE

    auto pickQueuesRoles = [&](const DeviceManager::QueueUtils &queues) -> bool {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(globalHardwareContext.mainDevice->deviceManager.physicalDevice, queues.queueFamilyIndex, vkSurface, &presentSupport);
        return presentSupport;
    };
    displayDevice = globalHardwareContext.mainDevice;
    presentQueues = globalHardwareContext.mainDevice->deviceManager.pickAvailableQueues(pickQueuesRoles);

#else
    for (int i = 0; i < globalHardwareContext.hardwareUtils.size(); i++)
    {
        auto pickQueuesRoles = [&](const DeviceManager::QueueUtils &queues) -> bool {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(globalHardwareContext.hardwareUtils[i]->deviceManager.physicalDevice, queues.queueFamilyIndex, vkSurface, &presentSupport);
            return presentSupport;
        };

        presentQueues = globalHardwareContext.hardwareUtils[i]->deviceManager.pickAvailableQueues(pickQueuesRoles);
        if (presentQueues.size()>0)
        {
            displayDevice = globalHardwareContext.hardwareUtils[i];
        }

        if (globalHardwareContext.mainDevice != displayDevice)
        {
            break;
        }
    }
#endif

    mainDeviceExecutor = std::make_shared<HardwareExecutor>(globalHardwareContext.mainDevice);
    displayDeviceExecutor = std::make_shared<HardwareExecutor>(displayDevice);
}

void DisplayManager::createSwapChain()
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(displayDevice->deviceManager.physicalDevice, vkSurface, &capabilities);

    this->displaySize = ktm::uvec2{
        std::clamp(capabilities.currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(capabilities.currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
    };

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(displayDevice->deviceManager.physicalDevice, vkSurface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    if (formatCount != 0)
    {
        vkGetPhysicalDeviceSurfaceFormatsKHR(displayDevice->deviceManager.physicalDevice, vkSurface, &formatCount, formats.data());

        surfaceFormat = formats[0];
        for (const auto& availableFormat : formats)
        {
            if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surfaceFormat = availableFormat;
                break;
            }
        }
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(displayDevice->deviceManager.physicalDevice, vkSurface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    if (presentModeCount != 0)
    {
        vkGetPhysicalDeviceSurfacePresentModesKHR(displayDevice->deviceManager.physicalDevice, vkSurface, &presentModeCount, presentModes.data());
        for (const auto& availablePresentMode : presentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = availablePresentMode;
                break;
            }
        }
    }

    uint32_t imageCount = (capabilities.maxImageCount > 0 && (capabilities.minImageCount + 1) > capabilities.maxImageCount) ?
        capabilities.maxImageCount : (capabilities.minImageCount + 1);


    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = { this->displaySize.x, this->displaySize.y };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if ((capabilities.supportedUsageFlags & createInfo.imageUsage) != createInfo.imageUsage)
    {
        throw std::runtime_error("Swapchain does not support required image usage flags (COLOR_ATTACHMENT and TRANSFER_DST)!");
    }

    std::vector<uint32_t> queueFamilys(displayDevice->deviceManager.getQueueFamilyNumber());
    for (size_t i = 0; i < queueFamilys.size(); i++)
    {
        queueFamilys[i] = i;
    }

    if (queueFamilys.size() > 1)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = queueFamilys.size();
        createInfo.pQueueFamilyIndices = queueFamilys.data();
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

    if (vkCreateSwapchainKHR(displayDevice->deviceManager.logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    std::vector<VkImage> swapChainVkImages;
    vkGetSwapchainImagesKHR(displayDevice->deviceManager.logicalDevice, swapChain, &imageCount, nullptr);
    swapChainVkImages.resize(imageCount);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(displayDevice->deviceManager.logicalDevice, swapChain, &imageCount, swapChainVkImages.data());

    for (uint32_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImages[i].imageHandle = swapChainVkImages[i];
        swapChainImages[i].imageSize = this->displaySize;
        swapChainImages[i].imageFormat = surfaceFormat.format;
        swapChainImages[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        swapChainImages[i].arrayLayers = 1;
        swapChainImages[i].mipLevels = 1;

        swapChainImages[i].device = &displayDevice->deviceManager;
        swapChainImages[i].resourceManager = &displayDevice->resourceManager;

        swapChainImages[i].pixelSize = 8;

        swapChainImages[i].imageView =displayDevice->resourceManager.createImageView(swapChainImages[i]);
    }
}

void DisplayManager::recreateSwapChain()
{
}

bool DisplayManager::displayFrame(void *displaySurface, HardwareImage displayImage)
{
    if (displaySurface != nullptr)
    {
        ResourceManager::ImageHardwareWrap &sourceImage = imageGlobalPool[*displayImage.imageID];

        if (this->displaySurface != displaySurface)
        {
            //this->displaySize = displaySize;
            this->displaySurface = displaySurface;

            // 如果之前已经创建过交换链/Surface，先完整清理，避免资源泄漏
            if (vkSurface != VK_NULL_HANDLE || swapChain != VK_NULL_HANDLE)
            {
                cleanUpDisplayManager();
            }

            initDisplayManager(displaySurface);

            if (globalHardwareContext.mainDevice != displayDevice)
            //if (true)
            {
                this->displayImage = displayDevice->resourceManager.createImage(imageGlobalPool[*displayImage.imageID].imageSize, imageGlobalPool[*displayImage.imageID].imageFormat,
                                                                                imageGlobalPool[*displayImage.imageID].pixelSize, imageGlobalPool[*displayImage.imageID].imageUsage);

                VkDeviceSize imageSizeBytes = this->displayImage.imageSize.x * this->displayImage.imageSize.y * this->displayImage.pixelSize;

                hostBufferPtr = malloc(imageSizeBytes);

                //srcStaging = globalHardwareContext.mainDevice->resourceManager.createBuffer(imageSizeBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);

                srcStaging = globalHardwareContext.mainDevice->resourceManager.importHostBuffer(hostBufferPtr, imageSizeBytes);

                {
                    // 导出缓冲区内存
                    //ResourceManager::ExternalMemoryHandle memHandle = globalHardwareContext.mainDevice->resourceManager.exportBufferMemory(srcStaging);

                    // 确保在导入前释放旧的资源
                    if (dstStaging.bufferHandle != VK_NULL_HANDLE)
                    {
                        displayDevice->resourceManager.destroyBuffer(dstStaging);
                    }

                    // 导入到目标设备
                    //dstStaging = displayDevice->resourceManager.importBufferMemory(memHandle, srcStaging);

                    dstStaging = displayDevice->resourceManager.importHostBuffer(hostBufferPtr, imageSizeBytes);
                }
            }
            else
            {
                this->displayImage = sourceImage;
            }
        }

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(displayDevice->deviceManager.physicalDevice, vkSurface, &capabilities);

        ktm::uvec2 displaySize = ktm::uvec2{
            std::clamp(capabilities.currentExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp(capabilities.currentExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};

        if (displaySize != this->displaySize)
        {

            for (auto &image : swapChainImages)
            {
               displayDevice->resourceManager.destroyImage(image);
               image.imageView = VK_NULL_HANDLE; // 防止后续重复销毁
            }

            createSwapChain();
        }

        vkWaitForFences(displayDevice->deviceManager.logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(displayDevice->deviceManager.logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        vkResetFences(displayDevice->deviceManager.logicalDevice, 1, &inFlightFences[currentFrame]);

        if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
        {
            if (globalHardwareContext.mainDevice != displayDevice)
            //if (true)
            {
                // 在主设备上：源图像 -> srcStaging
                CopyImageToBufferCommand copyCmd(sourceImage, srcStaging);
                (*mainDeviceExecutor) << &copyCmd << mainDeviceExecutor->commit();

                //vkDeviceWaitIdle(globalHardwareContext.mainDevice->deviceManager.logicalDevice);
                //srcCpuData.resize(srcStaging.bufferAllocInfo.size);
                //globalHardwareContext.mainDevice->resourceManager.copyBufferToCpu(srcStaging, srcCpuData.data());

                // 在显示设备上：dstStaging -> 目标图像
                CopyBufferToImageCommand copyCmd2(dstStaging, this->displayImage);
                (*displayDeviceExecutor) << &copyCmd2;

                //vkDeviceWaitIdle(displayDevice->deviceManager.logicalDevice);
                //dstCpuData.resize(dstStaging.bufferAllocInfo.size);
                //displayDevice->resourceManager.copyBufferToCpu(dstStaging, dstCpuData.data());
            }

            std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos;
            {
                VkSemaphoreSubmitInfo waitInfo{};
                waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                waitInfo.semaphore = imageAvailableSemaphores[currentFrame];
                waitInfo.value = 0; // For binary semaphores, this must be 0
                waitInfo.stageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
                waitSemaphoreInfos.push_back(waitInfo);
            }

            std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos;
            {
                VkSemaphoreSubmitInfo signalInfo{};
                signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                signalInfo.semaphore = renderFinishedSemaphores[currentFrame];
                signalInfo.value = 0; // Assuming binary semaphore
                signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                signalSemaphoreInfos.push_back(signalInfo);
            }

            BlitImageCommand blitCmd(this->displayImage, swapChainImages[imageIndex]);
            *displayDeviceExecutor << &blitCmd
                                   << displayDeviceExecutor->commit(waitSemaphoreInfos, signalSemaphoreInfos, inFlightFences[currentFrame]);

             // 准备呈现信息，等待 timeline semaphore
             VkPresentInfoKHR presentInfo{};
             presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
             presentInfo.waitSemaphoreCount = 1;
             presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

             VkSwapchainKHR swapChains[] = {swapChain};
             presentInfo.swapchainCount = 1;
             presentInfo.pSwapchains = swapChains;
             presentInfo.pImageIndices = &imageIndex;

             //DeviceManager::QueueUtils *queue;
             //uint16_t queueIndex = 0;

             //while (true)
             //{
             //    uint16_t queueIndex = currentQueueIndex.fetch_add(1) % presentQueues.size();
             //    queue = &presentQueues[queueIndex];

             //    if (queue->queueMutex->try_lock())
             //    {
             //        uint64_t timelineCounterValue = 0;
             //        vkGetSemaphoreCounterValue(displayDevice->deviceManager.logicalDevice, queue->timelineSemaphore, &timelineCounterValue);
             //        if (timelineCounterValue >= queue->timelineValue)
             //        {
             //            break;
             //        }
             //        else
             //        {
             //            queue->queueMutex->unlock();
             //        }
             //    }

             //    std::this_thread::yield();
             //}

             // std::cout << "Present Queue Index: " << queueIndex << std::endl;

             VkResult result;
             auto commitToQueue = [&](DeviceManager::QueueUtils *currentRecordQueue) -> bool {
                 result = vkQueuePresentKHR(currentRecordQueue->vkQueue, &presentInfo);
                 return true;
                 };

             DeviceManager::QueueUtils *queue = HardwareExecutor::pickQueueAndCommit(currentQueueIndex, presentQueues, commitToQueue);


             if (result == VK_ERROR_OUT_OF_DATE_KHR)
             {
                 recreateSwapChain();
             }
             else if (result != VK_SUCCESS)
             {
                 throw std::runtime_error("failed to vkQueuePresentKHR for a frame!");
             }

            currentFrame = (currentFrame + 1) % swapChainImages.size();
        }

        return true;

    }
    else
    {
        return false;
    }
}

