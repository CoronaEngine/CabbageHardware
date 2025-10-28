#include<Hardware/GlobalContext.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

HardwareContext globalHardwareContext;

HardwareContext::HardwareContext()
{
    prepareFeaturesChain();

    if (volkInitialize() != VK_SUCCESS)
    {
        throw std::runtime_error("Failed volkInitialize!");
    }

    createVkInstance(hardwareCreateInfos);

    volkLoadInstance(vkInstance);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    if (deviceCount <= 0)
    {
        throw std::runtime_error("Failed to find GPUs! Please buy a GPU!");
    }

    //hardwareUtils.resize(devices.size());
    for (size_t i = 0; i < devices.size(); i++)
    {
        hardwareUtils.push_back(std::make_shared<HardwareUtils>());
        hardwareUtils[i]->deviceManager.initDeviceManager(hardwareCreateInfos, vkInstance, devices[i]);
        hardwareUtils[i]->resourceManager.initResourceManager(hardwareUtils[i]->deviceManager);
    }

    chooseMainDevice();

    // demo of mutilple devices
    //if (hardwareUtils.size()>1)
    //{
    //    ResourceManager::ImageHardwareWrap a = hardwareUtils[0].resourceManager.createImage(ktm::uvec2(800, 800), VK_FORMAT_R8G8B8A8_UINT, 4, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    //    ResourceManager::ImageHardwareWrap b = hardwareUtils[1].resourceManager.createImage(ktm::uvec2(800, 800), VK_FORMAT_R8G8B8A8_UINT, 4, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    //    hardwareUtils[0].resourceManager.copyImageMemory(a, b);
    //    hardwareUtils[0].resourceManager.copyImageMemory(b, a);
    //    hardwareUtils[1].resourceManager.copyImageMemory(a, b);
    //    hardwareUtils[1].resourceManager.copyImageMemory(b, a);
    //}
    // demo of mutilple devices
}

HardwareContext::~HardwareContext()
{
    // Prefer explicit shutdown via CabbageHardwareShutdown(), but clean up as a fallback
    //try
    //{
    //    shutdown();
    //}
    //catch (...)
    //{
    //    // swallow exceptions during static destruction
    //}
}


void HardwareContext::prepareFeaturesChain()
{
    hardwareCreateInfos.requiredInstanceExtensions = [&](const VkInstance &instance, const VkPhysicalDevice &device) {
        std::set<const char *> requiredExtensions;

        requiredExtensions.insert("VK_KHR_surface");
        requiredExtensions.insert(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);

#if _WIN32 || _WIN64
        requiredExtensions.insert(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __APPLE__
        requiredExtensions.insert(VK_MVK_MOLTENVK_EXTENSION_NAME);
        requiredExtensions.insert(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif __linux__
        requiredExtensions.insert(VK_KHR_XLIB_SURFACE_EXTENSION_NAME); // or VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#endif
        return requiredExtensions;
    };

    hardwareCreateInfos.requiredDeviceExtensions = [&](const VkInstance &instance, const VkPhysicalDevice &device) {
        std::set<const char *> requiredExtensions;
        requiredExtensions.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        requiredExtensions.insert(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

        requiredExtensions.insert(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        requiredExtensions.insert(VK_AMD_GPU_SHADER_HALF_FLOAT_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        requiredExtensions.insert(VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        requiredExtensions.insert(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

        requiredExtensions.insert(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        requiredExtensions.insert(VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME);
#if _WIN32 || _WIN64
        requiredExtensions.insert(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
        requiredExtensions.insert(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
#elif __APPLE__
#elif __linux__
#endif
        return requiredExtensions;
    };

    hardwareCreateInfos.requiredDeviceFeatures = [&](const VkInstance &instance, const VkPhysicalDevice &device) {

        // Vulkan 1.0 core features
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.shaderInt16 = VK_TRUE;
        deviceFeatures.wideLines = VK_TRUE;

        // Vulkan 1.1 core features
        VkPhysicalDeviceVulkan11Features deviceFeatures11{};
        deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;
        deviceFeatures11.multiview = VK_TRUE;

        // Vulkan 1.2 core features
        VkPhysicalDeviceVulkan12Features deviceFeatures12{};
        deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        deviceFeatures12.bufferDeviceAddress = VK_TRUE;
        deviceFeatures12.shaderFloat16 = VK_TRUE;

        deviceFeatures12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        deviceFeatures12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        deviceFeatures12.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
        deviceFeatures12.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
        deviceFeatures12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        deviceFeatures12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        deviceFeatures12.descriptorBindingPartiallyBound = VK_TRUE;
        deviceFeatures12.runtimeDescriptorArray = VK_TRUE;
        deviceFeatures12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        deviceFeatures12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        deviceFeatures12.descriptorIndexing = VK_TRUE;

        deviceFeatures12.timelineSemaphore = VK_TRUE;

        // Vulkan 1.3 core features
        VkPhysicalDeviceVulkan13Features deviceFeatures13{};
        deviceFeatures13.synchronization2 = VK_TRUE;

        return (DeviceFeaturesChain() | deviceFeatures | deviceFeatures13 | deviceFeatures12 | deviceFeatures11);
    };
}


void HardwareContext::createVkInstance(const CreateCallback &initInfo)
{
    std::set<const char *> inputExtensions = initInfo.requiredInstanceExtensions(vkInstance, nullptr);

    std::vector<const char *> requiredExtensions(inputExtensions.begin(), inputExtensions.end());
    std::vector<const char *> requiredLayers{};

#ifdef CABBAGE_ENGINE_DEBUG
    bool enableDebugLayer = false;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto &layerProperties : availableLayers)
    {
        if (strcmp("VK_LAYER_KHRONOS_validation", layerProperties.layerName) == 0)
        {
            enableDebugLayer = true;

            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
            break;
        }
    }
#endif

    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

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
                std::cerr << " ---------- Extensions Warning ---------- Instance not support : " << requiredExtensions[i] << std::endl;

                requiredExtensions.erase(requiredExtensions.begin() + i);
                i--;
            }
        }
    }

    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (size_t i = 0; i < requiredLayers.size(); i++)
        {
            bool extensionSupported = false;
            for (size_t j = 0; j < availableLayers.size(); j++)
            {
                if (strcmp(requiredLayers[i], availableLayers[j].layerName) == 0)
                {
                    extensionSupported = true;
                    break;
                }
            }
            if (!extensionSupported)
            {
                std::cerr << " ---------- Extensions Warning ---------- Instance not support : " << requiredLayers[i] << std::endl;

                requiredLayers.erase(requiredLayers.begin() + i);
                i--;
            }
        }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
    createInfo.ppEnabledLayerNames = requiredLayers.data();
    createInfo.pNext = nullptr;

#ifdef CABBAGE_ENGINE_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableDebugLayer)
    {
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        auto debugCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) -> VkBool32 {
            std::cerr << " ---------- Vulkan Validation Layer ------ \n"
                      << pCallbackData->pMessage << std::endl;
            return VK_FALSE;
        };

        debugCreateInfo.pfnUserCallback = debugCallback;

        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
#endif

    VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }

#ifdef CABBAGE_ENGINE_DEBUG
    if (enableDebugLayer)
    {
        auto CreateDebugUtilsMessengerEXT = [](VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) -> VkResult {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr)
            {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            }
            else
            {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        };

        if (CreateDebugUtilsMessengerEXT(vkInstance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }
#endif
}

// 我觉得还是手动释放比较合理
//void HardwareContext::shutdown()
//{
//    static bool s_shutdown = false;
//    if (s_shutdown)
//        return;
//    s_shutdown = true;
//
//    // 1) Clean up display managers (swapchains/surfaces and sync objects)
//    // 清理
//    try
//    {
//        for (auto &kv : displayerGlobalPool)
//        {
//            if (kv.second)
//            {
//                kv.second->shutdown();
//            }
//        }
//        displayerGlobalPool.clear();
//    }
//    catch (...)
//    {
//    }
//
//    // 2) Release any remaining global images/buffers to avoid referencing devices later
//    try
//    {
//        for (auto &kv : imageGlobalPool)
//        {
//            auto &img = kv.second;
//            if (img.resourceManager)
//            {
//                img.resourceManager->destroyImage(img);
//            }
//        }
//        imageGlobalPool.clear();
//    }
//    catch (...)
//    {
//    }
//
//    try
//    {
//        for (auto &kv : bufferGlobalPool)
//        {
//            auto &buf = kv.second;
//            if (buf.resourceManager)
//            {
//                buf.resourceManager->destroyBuffer(buf);
//            }
//        }
//        bufferGlobalPool.clear();
//    }
//    catch (...)
//    {
//    }
//
//    // 3) Tear down ResourceManagers and Devices for all hardware utils
//    for (auto &h : hardwareUtils)
//    {
//        if (!h)
//            continue;
//        // Wait idle and clean resource manager first
//        try
//        {
//            h->resourceManager.cleanUpResourceManager();
//        }
//        catch (...)
//        {
//        }
//        try
//        {
//            h->deviceManager.cleanUpDeviceManager();
//        }
//        catch (...)
//        {
//        }
//    }
//    hardwareUtils.clear();
//    mainDevice.reset();
//
//    // 4) Destroy debug messenger (if any), then the instance
//    if (vkInstance != VK_NULL_HANDLE)
//    {
//#ifdef CABBAGE_ENGINE_DEBUG
//        if (debugMessenger != VK_NULL_HANDLE)
//        {
//            auto DestroyDebugUtilsMessengerEXT = [](VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) -> VkResult {
//                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//                if (func != nullptr)
//                {
//                    func(instance, debugMessenger, pAllocator);
//                    return VK_SUCCESS;
//                }
//                else
//                {
//                    return VK_ERROR_EXTENSION_NOT_PRESENT;
//                }
//            };
//            DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
//            debugMessenger = VK_NULL_HANDLE;
//        }
//#endif
//        vkDestroyInstance(vkInstance, nullptr);
//        vkInstance = VK_NULL_HANDLE;
//    }
//}

void CabbageHardwareShutdown()
{
    //globalHardwareContext.shutdown();
}

void HardwareContext::chooseMainDevice()
{
    //for (size_t i = 0; i < hardwareUtils.size(); i++)
    //{
    //    if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceName[0] == 'N')
    //    {
    //        mainDevice = &hardwareUtils[i];
    //        return;
    //    }
    //}

    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    for (size_t i = 0; i < hardwareUtils.size(); i++)
    {
        if (hardwareUtils[i]->deviceManager.deviceFeaturesUtils.supportedProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
        {
            mainDevice = hardwareUtils[i];
            return;
        }
    }

    if (hardwareUtils.size() > 0)
    {
        mainDevice = hardwareUtils[0];
    }
    else
    {
        throw std::runtime_error("Failed to find GPUs! Please buy a GPU!");
    }
}
