#pragma once

#include <chrono>  

#include<Hardware/GlobalContext.h>
#include<Hardware/HardwareExecutor.h>

#include<CabbageHardware.h>


class DisplayManager
{
public:

	DisplayManager();
	~DisplayManager();

	bool initDisplayManager(void* surface);

	bool displayFrame(void *surface, HardwareImage displayImage);

private:

    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    ktm::uvec2 displaySize = ktm::uvec2(0, 0);
    void *displaySurface = nullptr;

    std::vector<ResourceManager::ImageHardwareWrap> swapChainImages;

	VkSurfaceFormatKHR surfaceFormat;


	ResourceManager::ImageHardwareWrap displayImage;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    //VkSemaphore timelineSemaphore = VK_NULL_HANDLE;
    //uint64_t timelineCounter = 0;
    //std::vector<uint64_t> frameTimelineValues;
    
    std::atomic_uint16_t currentQueueIndex = 0;
    std::vector<DeviceManager::QueueUtils> presentQueues;

    std::shared_ptr<HardwareContext::HardwareUtils> displayDevice;


	void cleanUpDisplayManager();


	void createVkSurface(void* surface);

	void choosePresentDevice();


	void createSyncObjects();

	void createSwapChain();

    void recreateSwapChain();

	uint32_t currentFrame = 0;

    
    ResourceManager::BufferHardwareWrap srcStaging;
    std::vector<char> srcCpuData;
    ResourceManager::BufferHardwareWrap dstStaging;
    std::vector<char> dstCpuData;

    std::shared_ptr<HardwareExecutor> hardwareExecutor;
};