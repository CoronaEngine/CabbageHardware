#pragma once

#include "Hardware/DeviceManager.h"
#include "Hardware/ResourceManager.h"

struct HardwareContext
{
    HardwareContext();

    ~HardwareContext();

    struct HardwareUtils
    {
        DeviceManager deviceManager;
        ResourceManager resourceManager;
    };

    std::vector<std::shared_ptr<HardwareUtils>> hardwareUtils;

    std::shared_ptr<HardwareUtils> mainDevice;

    [[nodiscard]] VkInstance getVulkanInstance() const { return vkInstance; }

    [[nodiscard]] bool getEnableValidationLayer() const { return enableDebugLayer; }

private:
    void prepareFeaturesChain();
    CreateCallback hardwareCreateInfos{};

    void createVkInstance(const CreateCallback &createInfo);

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    bool enableDebugLayer = false;

    void chooseMainDevice();
};

extern HardwareContext globalHardwareContext;

extern std::unordered_map<uint64_t, ResourceManager::ImageHardwareWrap> imageGlobalPool;
extern std::unordered_map<uint64_t, ResourceManager::BufferHardwareWrap> bufferGlobalPool;