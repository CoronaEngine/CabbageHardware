#pragma once

#include "HardwareWrapperVulkan/HardwareVulkan/ResourceManager.h"
#include "corona/kernel/utils/storage.h"

class DisplayManager;

struct HardwareContext
{
  public:
    struct HardwareUtils
    {
        DeviceManager deviceManager;
        ResourceManager resourceManager;
    };

    HardwareContext();
    ~HardwareContext();

    HardwareContext(const HardwareContext &) = delete;
    HardwareContext &operator=(const HardwareContext &) = delete;
    HardwareContext(HardwareContext &&) = delete;
    HardwareContext &operator=(HardwareContext &&) = delete;

    [[nodiscard]] VkInstance getVulkanInstance() const
    {
        return vkInstance;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<HardwareUtils>> &getAllDevices() const
    {
        return hardwareUtils;
    }

    [[nodiscard]] std::shared_ptr<HardwareUtils> getMainDevice() const
    {
        return mainDevice;
    }

  private:
    void prepareFeaturesChain();
    void createVkInstance(const CreateCallback &createInfo);
    void chooseMainDevice();
    void setupDebugMessenger();
    void cleanupDebugMessenger();

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    CreateCallback hardwareCreateInfos{};
    std::vector<std::shared_ptr<HardwareUtils>> hardwareUtils;
    std::shared_ptr<HardwareUtils> mainDevice;
};

extern HardwareContext globalHardwareContext;
extern Corona::Kernel::Utils::Storage<ResourceManager::BufferHardwareWrap> globalBufferStorages;
extern Corona::Kernel::Utils::Storage<ResourceManager::ImageHardwareWrap> globalImageStorages;