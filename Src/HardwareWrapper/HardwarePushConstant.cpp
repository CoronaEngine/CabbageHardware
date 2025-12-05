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
        }
        handle->data = nullptr;
        return true;
    }
    return false;
}

HardwarePushConstant::HardwarePushConstant()
    : pushConstantID(globalPushConstantStorages.allocate()) {
    CFW_LOG_TRACE("HardwarePushConstant@{} default constructed, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  pushConstantID.load(std::memory_order_acquire));
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole)
    : pushConstantID(globalPushConstantStorages.allocate()) {
    auto const self_id = pushConstantID.load(std::memory_order_acquire);
    auto pushConstantHandle = globalPushConstantStorages.acquire_write(self_id);
    pushConstantHandle->size = size;

    // NOTE: 只有在外部传入有效的whole时才作为子资源
    if (whole != nullptr) {
        if (auto wholeHandle = globalPushConstantStorages.acquire_write(whole->pushConstantID.load(std::memory_order_acquire));
            wholeHandle->data != nullptr) {
            pushConstantHandle->data = wholeHandle->data + offset;
            pushConstantHandle->isSub = true;
        }
    }

    if (!pushConstantHandle->isSub) {
        pushConstantHandle->data = static_cast<uint8_t*>(std::malloc(size));
    }
    CFW_LOG_TRACE("HardwarePushConstant@{} constructed with size, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  self_id);
}

HardwarePushConstant::HardwarePushConstant(const HardwarePushConstant& other)
    : pushConstantID(other.pushConstantID.load(std::memory_order_acquire)) {
    auto const self_id = pushConstantID.load(std::memory_order_acquire);
    if (self_id > 0) {
        auto const self_handle = globalPushConstantStorages.acquire_write(self_id);
        incrementPushConstantRefCount(self_handle);
    }
    CFW_LOG_TRACE("HardwarePushConstant@{} copy constructed from @{}, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  self_id);
}

HardwarePushConstant::HardwarePushConstant(HardwarePushConstant&& other) noexcept
    : pushConstantID(other.pushConstantID.load(std::memory_order_acquire)) {
    auto const self_id = pushConstantID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwarePushConstant@{} move constructed from @{}, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  self_id);
    other.pushConstantID.store(0, std::memory_order_release);
}

HardwarePushConstant::~HardwarePushConstant() {
    auto const self_id = pushConstantID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwarePushConstant@{} destructor called, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  self_id);
    if (self_id > 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = globalPushConstantStorages.acquire_write(self_id);
            decrementPushConstantRefCount(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            CFW_LOG_TRACE("HardwarePushConstant@{} destroying, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_id);
            globalPushConstantStorages.deallocate(self_id);
        }
        pushConstantID.store(0, std::memory_order_release);
    }
}

HardwarePushConstant& HardwarePushConstant::operator=(const HardwarePushConstant& other) {
    if (this == &other) {
        return *this;
    }
    bool should_destroy_self = false;
    auto const self_id = pushConstantID.load(std::memory_order_acquire);
    auto const other_id = other.pushConstantID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwarePushConstant@{} copy assigned from @{}, ID: {} -> {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  self_id, other_id);
    if (self_id == 0 && other_id == 0) {
        return *this;
    }
    if (other_id == 0) {
        if (auto const self_handle = globalPushConstantStorages.acquire_write(self_id);
            !self_handle->isSub && decrementPushConstantRefCount(self_handle)) {  // 非子资源
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            CFW_LOG_TRACE("HardwarePushConstant@{} destroying in copy assignment, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_id);
            globalPushConstantStorages.deallocate(self_id);
        }
        pushConstantID.store(0, std::memory_order_release);
        CFW_LOG_WARNING("Copying from an uninitialized HardwarePushConstant.");
        return *this;
    }
    if (self_id == 0) {
        // 直接拷贝
        if (auto const other_handle = globalPushConstantStorages.acquire_write(other_id);
            !other_handle->isSub) {  // 非子资源
            incrementPushConstantRefCount(other_handle);
        }
        pushConstantID.store(other_id, std::memory_order_release);
        return *this;
    }

    if (self_id < other_id) {
        auto const self_handle = globalPushConstantStorages.acquire_write(self_id);
        auto const other_handle = globalPushConstantStorages.acquire_write(other_id);
        if (self_handle->isSub) {  // 子资源
            if (self_handle->data != nullptr && other_handle->data != nullptr) {
                std::memcpy(self_handle->data, other_handle->data, std::min(self_handle->size, other_handle->size));
            }
            return *this;  // 子资源直接返回
        }
        // 非子资源
        incrementPushConstantRefCount(other_handle);
        if (decrementPushConstantRefCount(self_handle)) {
            should_destroy_self = true;
        }
    } else {
        auto const other_handle = globalPushConstantStorages.acquire_write(other_id);
        auto const self_handle = globalPushConstantStorages.acquire_write(self_id);
        if (self_handle->isSub) {  // 子资源
            if (self_handle->data != nullptr && other_handle->data != nullptr) {
                std::memcpy(self_handle->data, other_handle->data, std::min(self_handle->size, other_handle->size));
            }
            return *this;  // 子资源直接返回
        }
        // 非子资源
        incrementPushConstantRefCount(other_handle);
        if (decrementPushConstantRefCount(self_handle)) {
            should_destroy_self = true;
        }
    }

    if (should_destroy_self) {
        CFW_LOG_TRACE("HardwarePushConstant@{} destroying in copy assignment, ID: {}",
                      reinterpret_cast<std::uintptr_t>(this),
                      self_id);
        globalPushConstantStorages.deallocate(self_id);
    }
    pushConstantID.store(other_id, std::memory_order_release);
    return *this;
}

HardwarePushConstant& HardwarePushConstant::operator=(HardwarePushConstant&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    auto const self_id = pushConstantID.load(std::memory_order_acquire);
    auto const other_id = other.pushConstantID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwarePushConstant@{} move assigned from @{}, ID: {} -> {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  self_id, other_id);
    if (self_id > 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = globalPushConstantStorages.acquire_write(self_id);
            decrementPushConstantRefCount(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            CFW_LOG_TRACE("HardwarePushConstant@{} destroying in move assignment, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_id);
            globalPushConstantStorages.deallocate(self_id);
        }
    }
    pushConstantID.store(other_id, std::memory_order_release);
    other.pushConstantID.store(0, std::memory_order_release);
    return *this;
}

uint8_t* HardwarePushConstant::getData() const {
    auto const self_id = pushConstantID.load(std::memory_order_acquire);
    return globalPushConstantStorages.acquire_write(self_id)->data;
}

uint64_t HardwarePushConstant::getSize() const {
    auto const self_id = pushConstantID.load(std::memory_order_acquire);
    return globalPushConstantStorages.acquire_read(self_id)->size;
}

void HardwarePushConstant::copyFromRaw(const void* src, uint64_t size) {
    if (src == nullptr || size == 0) {
        return;
    }
    CFW_LOG_TRACE("HardwarePushConstant@{} copyFromRaw called with size {}, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  size,
                  pushConstantID.load(std::memory_order_acquire));
    if (auto const self_id = pushConstantID.load(std::memory_order_acquire);
        self_id > 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = globalPushConstantStorages.acquire_write(self_id);
            decrementPushConstantRefCount(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            globalPushConstantStorages.deallocate(self_id);
            CFW_LOG_TRACE("HardwarePushConstant@{} destroying in copyFromRaw, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_id);
        }
    }
    auto const new_self_id = globalPushConstantStorages.allocate();
    CFW_LOG_TRACE("HardwarePushConstant@{} allocated in copyFromRaw, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  new_self_id);
    pushConstantID.store(new_self_id, std::memory_order_release);
    auto handle = globalPushConstantStorages.acquire_write(new_self_id);
    handle->size = size;
    handle->data = static_cast<uint8_t*>(std::malloc(size));
    std::memset(handle->data, 0, size);
    std::memcpy(handle->data, src, size);
}