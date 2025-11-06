#include"CabbageHardware.h"
#include<Hardware/GlobalContext.h>

struct PushConstantWraper
{
    uint8_t *data = nullptr;
    uint64_t size = 0;
    bool isSub = false;
    uint64_t refCount = 0;
};

Corona::Kernel::Utils::Storage<PushConstantWraper> globalPushConstantStorages;

HardwarePushConstant::HardwarePushConstant()
{
    auto handle = globalPushConstantStorages.allocate([&](ResourceManager::PushConstant &pushConstant) {
        pushConstant = globalHardwareContext.mainDevice->resourceManager.createPushConstant(0);
        pushConstant.refCount = 1;
        pushConstant.data = nullptr;
    });

    this->pushConstantIdPtr = std::make_shared<uint64_t>(static_cast<uint64_t>(handle));
}

HardwarePushConstant::HardwarePushConstant(const HardwarePushConstant &other)
{
    this->pushConstantIdPtr = other.pushConstantIdPtr;

    auto otherHandle = static_cast<uintptr_t>(*other.pushConstantIdPtr);
    bool write_success = globalPushConstantStorages.write(otherHandle, [](ResourceManager::PushConstant &pushConstant) {
        pushConstant.refCount++;
    });

    if (!write_success)
    {
        throw std::runtime_error("Failed to write PushConstant!");
    }
}

HardwarePushConstant::~HardwarePushConstant()
{
    auto thisHandle = static_cast<uintptr_t>(*this->pushConstantIdPtr);
    bool write_success = globalPushConstantStorages.write(thisHandle, [&](ResourceManager::PushConstant &pushConstant) {
        pushConstant.refCount--;
        if (pushConstant.refCount == 0)
        {
            if (pushConstant.data != nullptr && !pushConstant.isSub)
            {
                free(pushConstant.data);
                pushConstant.data = nullptr;
            }

            //globalHardwareContext.mainDevice->resourceManager.destroyBuffer(buffer);
            globalPushConstantStorages.deallocate(thisHandle);
        }
    });

    if (!write_success)
    {
        throw std::runtime_error("Failed to write PushConstant!");
    }
}

HardwarePushConstant &HardwarePushConstant::operator=(const HardwarePushConstant &other)
{
    auto otherHandle = static_cast<uintptr_t>(*other.pushConstantIdPtr);
    auto thisHandle = static_cast<uintptr_t>(*this->pushConstantIdPtr);

    ResourceManager::PushConstant otherPushConstant;
    ResourceManager::PushConstant thisPushConstant;

    bool write_success = globalPushConstantStorages.read(otherHandle, [&](const ResourceManager::PushConstant &pushConstant) {
        otherPushConstant = pushConstant;
    });

    bool write_success = globalPushConstantStorages.read(thisHandle, [&](const ResourceManager::PushConstant &pushConstant) {
        thisPushConstant = pushConstant;
    });

    if (thisPushConstant.isSub)
    {
        memcpy(thisPushConstant.data, otherPushConstant.data, otherPushConstant.size);
        bool write_success = globalPushConstantStorages.write(otherHandle, [&](ResourceManager::PushConstant &pushConstant) {
            pushConstant = otherPushConstant;
        });

    }
    else
    {
        otherPushConstant.refCount++;
        thisPushConstant.refCount--;
        if (thisPushConstant.refCount == 0)
        {
            if (thisPushConstant.data != nullptr && !thisPushConstant.isSub)
            {
                free(thisPushConstant.data);
                thisPushConstant.data = nullptr;
            }
            globalPushConstantStorages.deallocate(thisHandle);
        }
        *(this->pushConstantIdPtr) = *(other.pushConstantIdPtr);
    }

    return *this;
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole)
{
    auto handle = globalPushConstantStorages.allocate([&](ResourceManager::PushConstant &pushConstant) {
        pushConstant = globalHardwareContext.mainDevice->resourceManager.createPushConstant(size);
        pushConstant.refCount = 1;
        if (whole != nullptr)
        {
            auto wholeHandle = static_cast<uintptr_t>(*(whole->pushConstantIdPtr));
            ResourceManager::PushConstant wholePushConstant;
            bool read_success = globalPushConstantStorages.read(wholeHandle, [&](const ResourceManager::PushConstant &pushConstant) {
                wholePushConstant = pushConstant;
            });

            pushConstant.data = wholePushConstant.data + offset;
            pushConstant.isSub = true;
        }
        else
        {
            pushConstant.data = (uint8_t *)malloc(size);
        }
    });

    this->pushConstantIdPtr = std::make_shared<uint64_t>(static_cast<uint64_t>(handle));
}

uint8_t* HardwarePushConstant::getData() const
{
    bool read_success = globalPushConstantStorages.read(static_cast<uintptr_t>(*pushConstantIdPtr), [&](const ResourceManager::PushConstant &pushConstant) {
        return pushConstant.data;
    });
}

uint64_t HardwarePushConstant::getSize() const
{
    bool read_success = globalPushConstantStorages.read(static_cast<uintptr_t>(*pushConstantIdPtr), [&](const ResourceManager::PushConstant &pushConstant) {
        return pushConstant.size;
    });
}

void HardwarePushConstant::copyFromRaw(const void* src, uint64_t size)
{
    auto handle = globalPushConstantStorages.allocate([&](ResourceManager::PushConstant &pushConstant) {
        pushConstant = globalHardwareContext.mainDevice->resourceManager.createPushConstant(size);
        pushConstant.refCount = 1;
        pushConstant.data = (uint8_t *)malloc(size);
        memcpy(pushConstant.data, src, size);
    });

    this->pushConstantIdPtr = std::make_shared<uint64_t>(static_cast<uint64_t>(handle));
}