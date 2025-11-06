#pragma once

#include "Hardware/DeviceManager.h"
#include "Hardware/ResourceManager.h"
#include "../VulkanUtils.h"
#include "corona/kernel/utils/storage.h"

// 增加宏以启用导出内存功能
#define USE_EXPORT_MEMORY

class DisplayManager;

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

private:
    void prepareFeaturesChain();
    CreateCallback hardwareCreateInfos{};

    void createVkInstance(const CreateCallback &createInfo);

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    void chooseMainDevice();
};

extern HardwareContext globalHardwareContext;

extern Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap> globalBufferStorages;
extern Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap> globalImageStorages;
