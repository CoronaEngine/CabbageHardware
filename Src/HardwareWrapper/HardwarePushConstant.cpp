#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/HardwareContext.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

static void incrementPushConstantRefCount(const Corona::Kernel::Utils::Storage<PushConstantWrap, 4, 2>::WriteHandle& handle) {
    ++handle->refCount;
}

static bool decrementPushConstantRefCount(const Corona::Kernel::Utils::Storage<PushConstantWrap, 4, 2>::WriteHandle& handle) {
    if (--handle->refCount == 0) {
        if (handle->data != nullptr && !handle->isSub) {
            std::free(handle->data);
            handle->data = nullptr;
        }
        return true;
    }
    return false;
}

HardwarePushConstant::HardwarePushConstant()
    : pushConstantID(std::make_shared<uintptr_t>(globalPushConstantStorages.allocate())) {
    // CFW_LOG_DEBUG("Allocated HardwarePushConstant with ID: {}", *pushConstantID);
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole) {
    pushConstantID = std::make_shared<uintptr_t>(globalPushConstantStorages.allocate());
    // CFW_LOG_DEBUG("Allocated HardwarePushConstant with ID: {}", *pushConstantID);
    auto pushConstantHandle = globalPushConstantStorages.acquire_write(*pushConstantID);
    pushConstantHandle->size = size;
    pushConstantHandle->refCount = 1;
    pushConstantHandle->isSub = false;

    if (whole != nullptr) {
        auto wholeHandle = globalPushConstantStorages.acquire_read(*(whole->pushConstantID));
        if (wholeHandle->data != nullptr) {
            pushConstantHandle->data = wholeHandle->data + offset;
            pushConstantHandle->isSub = true;
        }
    }

    if (!pushConstantHandle->isSub) {
        pushConstantHandle->data = static_cast<uint8_t*>(std::malloc(size));
    }
}

HardwarePushConstant::HardwarePushConstant(const HardwarePushConstant& other)
    : pushConstantID(other.pushConstantID) {
    if (*pushConstantID > 0) {
        auto const handle = globalPushConstantStorages.acquire_write(*pushConstantID);
        incrementPushConstantRefCount(handle);
    }
}

HardwarePushConstant::~HardwarePushConstant() {
    if (pushConstantID && *pushConstantID > 0) {
        bool destroy = false;
        if (auto const handle = globalPushConstantStorages.acquire_write(*pushConstantID); decrementPushConstantRefCount(handle)) {
            destroy = true;
        }
        if (destroy) {
            globalPushConstantStorages.deallocate(*pushConstantID);
            // CFW_LOG_DEBUG("Deallocated HardwarePushConstant with ID: {}", *pushConstantID);
            pushConstantID = nullptr;
        }
    }
}

HardwarePushConstant& HardwarePushConstant::operator=(const HardwarePushConstant& other) {
    if (this != &other) {
        bool isSub = false;
        {
            auto thisPc = globalPushConstantStorages.acquire_write(*pushConstantID);
            auto otherPc = globalPushConstantStorages.acquire_read(*other.pushConstantID);

            if (thisPc->isSub) {
                if (thisPc->data != nullptr && otherPc->data != nullptr) {
                    std::memcpy(thisPc->data, otherPc->data, std::min(thisPc->size, otherPc->size));
                }
                isSub = thisPc->isSub;
            }
        }

        if (!isSub) {
            {
                auto const handle = globalPushConstantStorages.acquire_write(*other.pushConstantID);
                incrementPushConstantRefCount(handle);
            }
            {
                if (pushConstantID && *pushConstantID > 0) {
                    bool destroy = false;
                    if (auto const handle = globalPushConstantStorages.acquire_write(*pushConstantID); decrementPushConstantRefCount(handle)) {
                        destroy = true;
                    }
                    if (destroy) {
                        globalPushConstantStorages.deallocate(*pushConstantID);
                        // CFW_LOG_DEBUG("Deallocated HardwarePushConstant with ID: {}", *pushConstantID);
                        pushConstantID = nullptr;
                    }
                }
            }
            pushConstantID = other.pushConstantID;
        }
    }
    return *this;
}

uint8_t* HardwarePushConstant::getData() const {
    return globalPushConstantStorages.acquire_write(*pushConstantID)->data;
}

uint64_t HardwarePushConstant::getSize() const {
    return globalPushConstantStorages.acquire_read(*pushConstantID)->size;
}

void HardwarePushConstant::copyFromRaw(const void* src, uint64_t size) {
    if (src != nullptr || size != 0) {
        if (pushConstantID && *pushConstantID > 0) {
            bool destroy = false;
            if (auto const handle = globalPushConstantStorages.acquire_write(*pushConstantID); decrementPushConstantRefCount(handle)) {
                destroy = true;
            }
            if (destroy) {
                globalPushConstantStorages.deallocate(*pushConstantID);
                // CFW_LOG_DEBUG("Deallocated HardwarePushConstant with ID: {}", *pushConstantID);
                pushConstantID = nullptr;
            }
        }
        pushConstantID = std::make_shared<uintptr_t>(globalPushConstantStorages.allocate());
        // CFW_LOG_DEBUG("Allocated HardwarePushConstant with ID: {}", *pushConstantID);

        auto handle = globalPushConstantStorages.acquire_write(*pushConstantID);

        handle->size = size;
        handle->refCount = 1;
        handle->isSub = false;
        handle->data = static_cast<uint8_t*>(std::malloc(size));

        if (handle->data != nullptr) {
            std::memcpy(handle->data, src, size);
        }
    }
}