#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareVulkan/GlobalContext.h"

struct PushConstantWrap
{
    uint8_t *data = nullptr;
    uint64_t size = 0;
    uint64_t refCount = 0;
    bool isSub = false;
};

Corona::Kernel::Utils::Storage<PushConstantWrap> globalPushConstantStorages;

void incrementPushConstantRefCount(uintptr_t id)
{
    if (id != 0)
    {
        globalPushConstantStorages.write(id, [](PushConstantWrap &pc) {
            ++pc.refCount;
        });
    }
}

void decrementPushConstantRefCount(uintptr_t id)
{
    if (id == 0)
    {
        return;
    }

    bool shouldDestroy = false;
    globalPushConstantStorages.write(id, [&](PushConstantWrap &pc) {
        if (--pc.refCount == 0)
        {
            if (pc.data != nullptr && !pc.isSub)
            {
                std::free(pc.data);
                pc.data = nullptr;
            }
            shouldDestroy = true;
        }
    });

    if (shouldDestroy)
    {
        globalPushConstantStorages.deallocate(id);
    }
}

HardwarePushConstant::HardwarePushConstant()
{
    const auto handle = globalPushConstantStorages.allocate([](PushConstantWrap &pc) {
        pc.size = 0;
        pc.data = nullptr;
        pc.refCount = 1;
        pc.isSub = false;
    });

    pushConstantID = std::make_shared<uintptr_t>(handle);
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant *whole)
{
    uint8_t *wholeData = nullptr;
    if (whole != nullptr)
    {
        globalPushConstantStorages.read(*whole->pushConstantID,
                                        [&](const PushConstantWrap &pc) {
                                            wholeData = pc.data;
                                        });
    }

    const auto handle = globalPushConstantStorages.allocate([&](PushConstantWrap &pc) {
        pc.size = size;
        pc.refCount = 1;

        if (whole != nullptr && wholeData != nullptr)
        {
            pc.data = wholeData + offset;
            pc.isSub = true;
        }
        else
        {
            pc.data = static_cast<uint8_t *>(std::malloc(size));
            pc.isSub = false;
        }
    });

    pushConstantID = std::make_shared<uintptr_t>(handle);
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
    if (this == &other)
    {
        return *this;
    }

    PushConstantWrap thisPc;
    PushConstantWrap otherPc;

    globalPushConstantStorages.read(*pushConstantID, [&](const PushConstantWrap &pc) {
        thisPc = pc;
    });

    globalPushConstantStorages.read(*other.pushConstantID, [&](const PushConstantWrap &pc) {
        otherPc = pc;
    });

    if (thisPc.isSub)
    {
        if (thisPc.data != nullptr && otherPc.data != nullptr)
        {
            const size_t copySize = std::min(thisPc.size, otherPc.size);
            std::memcpy(thisPc.data, otherPc.data, copySize);
            globalPushConstantStorages.write(*pushConstantID, [&](PushConstantWrap &pc) {
                pc = thisPc;
            });
        }
    }
    else
    {
        incrementPushConstantRefCount(*other.pushConstantID);
        decrementPushConstantRefCount(*pushConstantID);
        *(pushConstantID) = *(other.pushConstantID);
    }

    return *this;
}

uint8_t *HardwarePushConstant::getData() const
{
    uint8_t *data = nullptr;
    globalPushConstantStorages.read(*pushConstantID, [&](const PushConstantWrap &pc) {
        data = pc.data;
    });

    return data;
}

uint64_t HardwarePushConstant::getSize() const
{
    uint64_t size = 0;
    globalPushConstantStorages.read(*pushConstantID, [&](const PushConstantWrap &pc) {
        size = pc.size;
    });

    return size;
}

void HardwarePushConstant::copyFromRaw(const void *src, uint64_t size)
{
    if (src == nullptr || size == 0)
    {
        return;
    }

    const auto handle = globalPushConstantStorages.allocate([&](PushConstantWrap &pc) {
        pc.size = size;
        pc.refCount = 1;
        pc.isSub = false;
        pc.data = static_cast<uint8_t *>(std::malloc(size));

        if (pc.data != nullptr)
        {
            std::memcpy(pc.data, src, size);
        }
    });

    pushConstantID = std::make_shared<uintptr_t>(handle);
}