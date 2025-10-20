#include"DisplayManager.h"

#include <algorithm>
#include<vector>

#include<volk.h>

#include<Hardware/GlobalContext.h>

#define USE_SAME_DEVICE

//#define TEST_CPU_DATA

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
	//cleaarupDisplayManager();

	//vkDestroySwapchainKHR(displayDevice.logicalDevice, swapChain, nullptr);
	// Destroy the contexts
	//if (m_UpscalingContext)
	//{
	//	ffx::DestroyContext(m_UpscalingContext);
	//	m_UpscalingContext = nullptr;
	//}
}


void DisplayManager::cleanUpDisplayManager()
{
	//if (displayDevice.logicalDevice != VK_NULL_HANDLE)
	//{

	//	for (size_t i = 0; i < swapChainImages.size(); i++)
	//	{
 //          displayDevice->deviceManager->resourceManager.destroyImage(swapChainImages[i]);
	//	}
	//	swapChainImages.clear();

	//	if (swapChain != VK_NULL_HANDLE)
	//	{
	//		vkDestroySwapchainKHR(displayDevice.logicalDevice, swapChain, nullptr);
	//		swapChain = VK_NULL_HANDLE;
	//	}
	//	if (vkSurface != VK_NULL_HANDLE)
	//	{
 //           vkDestroySurfaceKHR(globalHardwareContext.getVulkanInstance(), vkSurface, nullptr);
	//		vkSurface = VK_NULL_HANDLE;
	//	}
	//}
}


bool DisplayManager::initDisplayManager(void* surface)
{
	if (surface != nullptr)
	{
		//cleaarupDisplayManager();

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

    hardwareExecutor = HardwareExecutor(displayDevice);
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

    if (queueFamilys.size() > 0)
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

            initDisplayManager(displaySurface);


            this->displayImage = displayDevice->resourceManager.createImage(imageGlobalPool[*displayImage.imageID].imageSize, imageGlobalPool[*displayImage.imageID].imageFormat,
                                                                            imageGlobalPool[*displayImage.imageID].pixelSize, imageGlobalPool[*displayImage.imageID].imageUsage);

            VkDeviceSize imageSizeBytes = this->displayImage.imageSize.x * this->displayImage.imageSize.y * this->displayImage.pixelSize;

            srcStaging = globalHardwareContext.mainDevice->resourceManager.createBuffer(imageSizeBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

            {
                // 导出缓冲区内存
                ResourceManager::ExternalMemoryHandle memHandle = globalHardwareContext.mainDevice->resourceManager.exportBufferMemory(srcStaging);

                // 确保在导入前释放旧的资源
                if (dstStaging.bufferHandle != VK_NULL_HANDLE)
                {
                    displayDevice->resourceManager.destroyBuffer(dstStaging);
                }

                // 导入到目标设备
                dstStaging = displayDevice->resourceManager.importBufferMemory(memHandle, srcStaging);
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
            }

            createSwapChain();
        }

        vkWaitForFences(displayDevice->deviceManager.logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(displayDevice->deviceManager.logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        vkResetFences(displayDevice->deviceManager.logicalDevice, 1, &inFlightFences[currentFrame]);

		if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
        {
            // 在主设备上：源图像 -> srcStaging
            hardwareExecutor(HardwareExecutor::ExecutorType::Graphics) << globalHardwareContext.mainDevice->resourceManager.copyImageToBuffer(
                &hardwareExecutor,
                sourceImage.imageHandle,
                srcStaging.bufferHandle,
                sourceImage.imageSize.x,
                sourceImage.imageSize.y);

#ifdef TEST_CPU_DATA
            vkDeviceWaitIdle(displayDevice->deviceManager.logicalDevice);
            vkDeviceWaitIdle(globalHardwareContext.mainDevice->deviceManager.logicalDevice);

            srcCpuData.resize(srcStaging.bufferAllocInfo.size);
            globalHardwareContext.mainDevice->resourceManager.copyBufferToCpu(srcStaging, srcCpuData.data());
#endif

            // 在显示设备上：dstStaging -> 目标图像
            hardwareExecutor << displayDevice->resourceManager.copyBufferToImage(
                &hardwareExecutor,
                dstStaging.bufferHandle,
                this->displayImage.imageHandle,
                this->displayImage.imageSize.x,
                this->displayImage.imageSize.y);

#ifdef TEST_CPU_DATA
            dstCpuData.resize(dstStaging.bufferAllocInfo.size);
            displayDevice->resourceManager.copyBufferToCpu(dstStaging.device->logicalDevice, dstStaging.bufferAllocInfo.deviceMemory, dstStaging.bufferAllocInfo.size, dstCpuData.data());
#endif

            auto runCommand = [&](const VkCommandBuffer &commandBuffer) {

                VkImageBlit imageBlit;
                imageBlit.dstOffsets[0] = VkOffset3D{0, 0, 0};
                imageBlit.dstOffsets[1] = VkOffset3D{int32_t(swapChainImages[imageIndex].imageSize.x), int32_t(swapChainImages[imageIndex].imageSize.y), 1};

                imageBlit.srcOffsets[0] = VkOffset3D{0, 0, 0};
                imageBlit.srcOffsets[1] = VkOffset3D{int32_t(this->displayImage.imageSize.x), int32_t(this->displayImage.imageSize.y), 1};

                imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

                imageBlit.dstSubresource.baseArrayLayer = 0;
                imageBlit.srcSubresource.baseArrayLayer = 0;

                imageBlit.dstSubresource.layerCount = 1;
                imageBlit.srcSubresource.layerCount = 1;

                imageBlit.dstSubresource.mipLevel = 0;
                imageBlit.srcSubresource.mipLevel = 0;

                vkCmdBlitImage(commandBuffer, this->displayImage.imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               swapChainImages[imageIndex].imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &imageBlit, VK_FILTER_LINEAR);
            };

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

             hardwareExecutor << runCommand  << hardwareExecutor.commit(waitSemaphoreInfos, signalSemaphoreInfos, inFlightFences[currentFrame]);

             // 准备呈现信息，等待 timeline semaphore
             VkPresentInfoKHR presentInfo{};
             presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
             presentInfo.waitSemaphoreCount = 1;
             presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

             VkSwapchainKHR swapChains[] = {swapChain};
             presentInfo.swapchainCount = 1;
             presentInfo.pSwapchains = swapChains;
             presentInfo.pImageIndices = &imageIndex;

             DeviceManager::QueueUtils *queue;
             uint16_t queueIndex = 0;

             while (true)
             {
                 uint16_t queueIndex = currentQueueIndex.fetch_add(1) % presentQueues.size();
                 queue = &presentQueues[queueIndex];

                 if (queue->queueMutex->try_lock())
                 {
                     uint64_t timelineCounterValue = 0;
                     vkGetSemaphoreCounterValue(displayDevice->deviceManager.logicalDevice, queue->timelineSemaphore, &timelineCounterValue);
                     if (timelineCounterValue >= queue->timelineValue)
                     {
                         break;
                     }
                     else
                     {
                         queue->queueMutex->unlock();
                     }
                 }

                 std::this_thread::yield();
             }

             // std::cout << "Present Queue Index: " << queueIndex << std::endl;

             VkResult result = vkQueuePresentKHR(queue->vkQueue, &presentInfo);

             queue->queueMutex->unlock();

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
