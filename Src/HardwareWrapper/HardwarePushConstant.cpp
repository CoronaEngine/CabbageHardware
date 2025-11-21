#include "CabbageHardware.h"

#include "corona/kernel/utils/storage.h"
#include "HardwareWrapperVulkan/HardwareContext.h"


#include "HardwareWrapperVulkan/ResourcePool.h"


void incrementPushConstantRefCount(uintptr_t id)
{
    if (id != 0)
    {
        globalPushConstantStorages.acquire_write(id)->refCount++;
    }
}

void decrementPushConstantRefCount(uintptr_t id) {
    if (id != 0) {
        bool shouldDestroy = false;
        {
            auto pushConstantHandle = globalPushConstantStorages.acquire_write(id);
            --pushConstantHandle->refCount;
            if (pushConstantHandle->refCount == 0) {
                if (pushConstantHandle->data != nullptr && !pushConstantHandle->isSub) {
                    std::free(pushConstantHandle->data);
                    pushConstantHandle->data = nullptr;
                }
                shouldDestroy = true;
            }
        }
        if (shouldDestroy) {
            globalPushConstantStorages.deallocate(id);
        }
    }
}

HardwarePushConstant::HardwarePushConstant() 
    : pushConstantID(std::make_shared<uintptr_t>(globalPushConstantStorages.allocate()))
{
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant *whole)
{
    pushConstantID = std::make_shared<uintptr_t>(globalPushConstantStorages.allocate());

    auto pushConstantHandle = globalPushConstantStorages.acquire_write(*pushConstantID);
    pushConstantHandle->size = size;
    pushConstantHandle->refCount = 1;
    pushConstantHandle->isSub = false;

    if (whole != nullptr)
    {
        auto wholeHandle = globalPushConstantStorages.acquire_read(*(whole->pushConstantID));
        if (wholeHandle->data != nullptr) {
            pushConstantHandle->data = wholeHandle->data + offset;
            pushConstantHandle->isSub = true;
        }
    }

    if (!pushConstantHandle->isSub)
    {
        pushConstantHandle->data = static_cast<uint8_t *>(std::malloc(size));
    }
}

HardwarePushConstant::HardwarePushConstant(const HardwarePushConstant &other)
    : pushConstantID(other.pushConstantID)
{
    incrementPushConstantRefCount(*pushConstantID);
}

HardwarePushConstant::~HardwarePushConstant()
{
    if (pushConstantID)
    {
        decrementPushConstantRefCount(*pushConstantID);
    }
}

HardwarePushConstant &HardwarePushConstant::operator=(const HardwarePushConstant &other)
{
    if (this != &other) {
        auto thisPc = globalPushConstantStorages.acquire_write(*pushConstantID);
        auto otherPc = globalPushConstantStorages.acquire_read(*other.pushConstantID);

        if (thisPc->isSub) {
            if (thisPc->data != nullptr && otherPc->data != nullptr) {
                std::memcpy(thisPc->data, otherPc->data, std::min(thisPc->size, otherPc->size));
            }
        } else {
            incrementPushConstantRefCount(*other.pushConstantID);
            decrementPushConstantRefCount(*pushConstantID);
            *(pushConstantID) = *(other.pushConstantID);
        }
    }
    return *this;
}

uint8_t *HardwarePushConstant::getData() const
{
    return globalPushConstantStorages.acquire_write(*pushConstantID)->data;
}

uint64_t HardwarePushConstant::getSize() const
{
    return globalPushConstantStorages.acquire_read(*pushConstantID)->size;
}

void HardwarePushConstant::copyFromRaw(const void *src, uint64_t size) {
    if (src != nullptr || size != 0) {
        pushConstantID = std::make_shared<uintptr_t>(globalPushConstantStorages.allocate());

        auto handle = globalPushConstantStorages.acquire_write(*pushConstantID);

        handle->size = size;
        handle->refCount = 1;
        handle->isSub = false;
        handle->data = static_cast<uint8_t *>(std::malloc(size));

        if (handle->data != nullptr) {
            std::memcpy(handle->data, src, size);
        }
    }
}