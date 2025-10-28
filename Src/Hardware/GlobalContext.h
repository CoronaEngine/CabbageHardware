#pragma once

#include "Hardware/DeviceManager.h"
#include "Hardware/ResourceManager.h"
class DisplayManager;

struct HardwareContext
{
    HardwareContext();

    ~HardwareContext();

    // Explicitly shutdown to deterministically release all Vulkan resources
    void shutdown();

    struct HardwareUtils
    {
        DeviceManager deviceManager;
        ResourceManager resourceManager;
    };

    std::vector<std::shared_ptr<HardwareUtils>> hardwareUtils;

    std::shared_ptr<HardwareUtils> mainDevice;

    [[nodiscard]] VkInstance getVulkanInstance() const { return vkInstance; }

private:
    void prepareFeaturesChain();
    CreateCallback hardwareCreateInfos{};

    void createVkInstance(const CreateCallback &createInfo);

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    void chooseMainDevice();
};

extern HardwareContext globalHardwareContext;

extern std::unordered_map<uint64_t, ResourceManager::ImageHardwareWrap> imageGlobalPool;
extern std::unordered_map<uint64_t, ResourceManager::BufferHardwareWrap> bufferGlobalPool;

// Global displayer pool defined in HardwareDisplayer.cpp
extern std::unordered_map<void *, std::shared_ptr<DisplayManager>> displayerGlobalPool;

// Explicit shutdown API to deterministically release all resources
void CabbageHardwareShutdown();
