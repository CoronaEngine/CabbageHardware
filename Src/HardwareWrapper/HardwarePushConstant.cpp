#include"CabbageHardware.h"
#include<Hardware/GlobalContext.h>

struct PushConstantWrap
{
    uint8_t *data = nullptr;
    uint64_t size = 0;
    bool isSub = false;
    uint64_t refCount = 0;
};

Corona::Kernel::Utils::Storage<PushConstantWrap> globalPushConstantStorages;

HardwarePushConstant::HardwarePushConstant()
{
    auto handle = globalPushConstantStorages.allocate([&](PushConstantWrap &pushConstant) {
        PushConstantWrap newPushConstant;
        newPushConstant.size = 0;
        newPushConstant.data = nullptr;
        newPushConstant.refCount = 1;
        pushConstant = newPushConstant;
    });

    this->pushConstantID = std::make_shared<uintptr_t>(handle);
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant *whole)
{
    auto handle = globalPushConstantStorages.allocate([&](PushConstantWrap &pushConstant) {
        PushConstantWrap newPushConstant;
        newPushConstant.size = size;
        newPushConstant.refCount = 1;
        if (whole != nullptr)
        {
            globalPushConstantStorages.read(*(whole->pushConstantID), [&](const PushConstantWrap &wholePushConstant) {
                newPushConstant.data = wholePushConstant.data + offset;
            });
            newPushConstant.isSub = true;
        }
        else
        {
            newPushConstant.data = (uint8_t *)malloc(size);
        }
        pushConstant = newPushConstant;
    });

    this->pushConstantID = std::make_shared<uintptr_t>(handle);
}

HardwarePushConstant::HardwarePushConstant(const HardwarePushConstant &other)
{
    this->pushConstantID = other.pushConstantID;
    globalPushConstantStorages.write(*other.pushConstantID, [](PushConstantWrap &pushConstant) {
        pushConstant.refCount++;
    });
}

HardwarePushConstant::~HardwarePushConstant()
{
    globalPushConstantStorages.write(*pushConstantID, [&](PushConstantWrap &pushConstant) {
        pushConstant.refCount--;
        if (pushConstant.refCount == 0)
        {
            if (pushConstant.data != nullptr && !pushConstant.isSub)
            {
                free(pushConstant.data);
                pushConstant.data = nullptr;
            }

            //globalHardwareContext.mainDevice->resourceManager.destroyBuffer(buffer);
            globalPushConstantStorages.deallocate(*pushConstantID);
        }
    });
}

HardwarePushConstant &HardwarePushConstant::operator=(const HardwarePushConstant &other)
{
    PushConstantWrap otherPushConstant;
    PushConstantWrap thisPushConstant;

    globalPushConstantStorages.read(*other.pushConstantID, [&](const PushConstantWrap &pushConstant) {
        otherPushConstant = pushConstant;
    });

    globalPushConstantStorages.read(*pushConstantID, [&](const PushConstantWrap &pushConstant) {
        thisPushConstant = pushConstant;
    });

    if (thisPushConstant.isSub)
    {
        memcpy(thisPushConstant.data, otherPushConstant.data, otherPushConstant.size);
        globalPushConstantStorages.write(*other.pushConstantID, [&](PushConstantWrap &pushConstant) {
            pushConstant = otherPushConstant;
        });

    }
    else
    {
        globalPushConstantStorages.write(*other.pushConstantID, [&](PushConstantWrap &pushConstant) {
            pushConstant.refCount++;
        });
        globalPushConstantStorages.write(*pushConstantID, [&](PushConstantWrap &pushConstant) {
            pushConstant.refCount--;
            if (pushConstant.refCount == 0)
            {
                if (pushConstant.data != nullptr && !pushConstant.isSub)
                {
                    free(pushConstant.data);
                    pushConstant.data = nullptr;
                }
                globalPushConstantStorages.deallocate(*pushConstantID);
            }

        });
        *(this->pushConstantID) = *(other.pushConstantID);
    }

    return *this;
}

uint8_t* HardwarePushConstant::getData() const
{
    globalPushConstantStorages.read(*pushConstantID, [&](const PushConstantWrap &pushConstant) {
        return pushConstant.data;
    });
}

uint64_t HardwarePushConstant::getSize() const
{

    globalPushConstantStorages.read(*pushConstantID, [&](const PushConstantWrap &pushConstant) {
        return pushConstant.size;
    });
}

void HardwarePushConstant::copyFromRaw(const void* src, uint64_t size)
{
    auto handle = globalPushConstantStorages.allocate([&](PushConstantWrap &pushConstant) {
        PushConstantWrap newPushConstant;
        newPushConstant.size = size;
        newPushConstant.refCount = 1;
        newPushConstant.data = (uint8_t *)malloc(size);
        memcpy(newPushConstant.data, src, size);
        pushConstant = newPushConstant;
    });

    this->pushConstantID = std::make_shared<uintptr_t>(handle);
}