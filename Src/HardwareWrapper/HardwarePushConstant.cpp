#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <utility>

HardwarePushConstant::HardwarePushConstant()
    : pushConstantHandle_(globalPushConstantStorages.allocate_handle())
{
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant *whole)
{
    pushConstantHandle_ = globalPushConstantStorages.allocate_handle();
    auto const self_id = getPushConstantID();
    auto pushConstantHandle = globalPushConstantStorages.acquire_write(self_id);
    pushConstantHandle->size = size;
    pushConstantHandle->isSub = false;

    if (whole != nullptr)
    {
        auto wholeHandle = globalPushConstantStorages.acquire_read(whole->getPushConstantID());
        if (wholeHandle->data != nullptr && offset <= wholeHandle->size && size <= wholeHandle->size - offset)
        {
            pushConstantHandle->data = wholeHandle->data + offset;
            pushConstantHandle->isSub = true;
            pushConstantHandle->parentRef = whole->pushConstantHandle_;
        }
    }

    if (!pushConstantHandle->isSub)
    {
        pushConstantHandle->data = static_cast<uint8_t *>(std::malloc(size));
    }
}

HardwarePushConstant::HardwarePushConstant(const HardwarePushConstant &other)
    : pushConstantHandle_(other.pushConstantHandle_)
{
}

HardwarePushConstant::HardwarePushConstant(HardwarePushConstant &&other) noexcept
    : pushConstantHandle_(std::move(other.pushConstantHandle_))
{
}

HardwarePushConstant::~HardwarePushConstant() = default;

HardwarePushConstant &HardwarePushConstant::operator=(const HardwarePushConstant &other)
{
    if (this == &other)
    {
        return *this;
    }

    auto const self_id = getPushConstantID();
    auto const other_id = other.getPushConstantID();

    if (self_id > 0 && other_id > 0)
    {
        auto thisPc = globalPushConstantStorages.acquire_write(self_id);
        if (thisPc->isSub)
        {
            auto otherPc = globalPushConstantStorages.acquire_read(other_id);
            if (thisPc->data != nullptr && otherPc->data != nullptr)
            {
                std::memcpy(thisPc->data, otherPc->data, static_cast<size_t>(std::min(thisPc->size, otherPc->size)));
            }
            return *this;
        }
    }

    pushConstantHandle_ = other.pushConstantHandle_;
    return *this;
}

HardwarePushConstant &HardwarePushConstant::operator=(HardwarePushConstant &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    pushConstantHandle_ = std::move(other.pushConstantHandle_);
    return *this;
}

uint8_t *HardwarePushConstant::getData() const
{
    auto const self_id = getPushConstantID();
    if (self_id > 0)
    {
        return globalPushConstantStorages.acquire_write(self_id)->data;
    }
    return nullptr;
}

uint64_t HardwarePushConstant::getSize() const
{
    auto const self_id = getPushConstantID();
    if (self_id > 0)
    {
        return globalPushConstantStorages.acquire_read(self_id)->size;
    }
    return 0;
}

void HardwarePushConstant::copyFromRaw(const void *src, uint64_t size)
{
    if (src == nullptr || size == 0)
    {
        return;
    }

    pushConstantHandle_ = globalPushConstantStorages.allocate_handle();
    auto const new_id = getPushConstantID();
    auto handle = globalPushConstantStorages.acquire_write(new_id);

    handle->size = size;
    handle->isSub = false;
    handle->data = static_cast<uint8_t *>(std::malloc(size));

    if (handle->data != nullptr)
    {
        std::memcpy(handle->data, src, static_cast<size_t>(size));
    }
}
