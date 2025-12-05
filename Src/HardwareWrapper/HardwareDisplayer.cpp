#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/DisplayVulkan/DisplayManager.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

static void incrementDisplayerRefCount(const Corona::Kernel::Utils::Storage<DisplayerHardwareWrap>::WriteHandle& handle) {
    ++handle->refCount;
}

static bool decrementDisplayerRefCount(const Corona::Kernel::Utils::Storage<DisplayerHardwareWrap>::WriteHandle& handle) {
    if (--handle->refCount == 0) {
        handle->displayManager.reset();
        handle->displaySurface = nullptr;
        return true;
    }

    return false;
}

HardwareDisplayer::HardwareDisplayer(void* surface)
    : displaySurfaceID(globalDisplayerStorages.allocate()) {
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
    self_handle->displaySurface = surface;
    self_handle->displayManager = std::make_shared<DisplayManager>();
    CFW_LOG_TRACE("HardwareDisplayer@{} constructed with surface, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  self_id);
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer& other)
    : displaySurfaceID(other.displaySurfaceID.load(std::memory_order_acquire)) {
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    if (self_id > 0) {
        auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
        incrementDisplayerRefCount(self_handle);
    }
    CFW_LOG_TRACE("HardwareDisplayer@{} copy constructed from @{}, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  self_id);
}

HardwareDisplayer::HardwareDisplayer(HardwareDisplayer&& other) noexcept
    : displaySurfaceID(other.displaySurfaceID.load(std::memory_order_acquire)) {
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwareDisplayer@{} move constructed from @{}, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  self_id);
    other.displaySurfaceID.store(0, std::memory_order_release);
}

HardwareDisplayer::~HardwareDisplayer() {
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwareDisplayer@{} destructor called, ID: {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  self_id);
    if (self_id > 0) {
        bool destroy = false;
        if (auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
            decrementDisplayerRefCount(self_handle)) {
            destroy = true;
        }
        if (destroy) {
            CFW_LOG_TRACE("HardwareDisplayer@{} destroying, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_id);
            globalDisplayerStorages.deallocate(self_id);
        }
    }
}

HardwareDisplayer& HardwareDisplayer::operator=(const HardwareDisplayer& other) {
    if (this == &other) {
        return *this;
    }
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    auto const other_id = other.displaySurfaceID.load(std::memory_order_acquire);
    CFW_LOG_TRACE("HardwareDisplayer@{} copy assigned from @{}, ID: {} -> {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  self_id, other_id);
    if (self_id == 0 && other_id == 0) {
        // 都未初始化，直接返回
        return *this;
    }
    if (self_id == other_id) {
        // 已经指向同一个资源，无需操作
        return *this;
    }
    bool should_destroy_self = false;
    if (other_id == 0) {
        // 释放自身资源
        if (auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
            decrementDisplayerRefCount(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            CFW_LOG_TRACE("HardwareDisplayer@{} destroying in copy assignment, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_id);
            globalDisplayerStorages.deallocate(self_id);
        }
        displaySurfaceID.store(0, std::memory_order_release);
        return *this;
    }
    if (self_id == 0) {
        // 直接拷贝
        displaySurfaceID.store(other_id, std::memory_order_release);
        auto const other_handle = globalDisplayerStorages.acquire_write(other_id);
        incrementDisplayerRefCount(other_handle);
        return *this;
    }
    if (self_id < other_id) {
        auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
        auto const other_handle = globalDisplayerStorages.acquire_write(other_id);
        incrementDisplayerRefCount(other_handle);
        if (decrementDisplayerRefCount(self_handle)) {
            should_destroy_self = true;
        }
    } else {
        auto const other_handle = globalDisplayerStorages.acquire_write(other_id);
        auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
        incrementDisplayerRefCount(other_handle);
        if (decrementDisplayerRefCount(self_handle)) {
            should_destroy_self = true;
        }
    }
    if (should_destroy_self) {
        CFW_LOG_TRACE("HardwareDisplayer@{} destroying in copy assignment, ID: {}",
                      reinterpret_cast<std::uintptr_t>(this),
                      self_id);
        globalDisplayerStorages.deallocate(self_id);
    }
    displaySurfaceID.store(other_id, std::memory_order_release);
    return *this;
}

HardwareDisplayer& HardwareDisplayer::operator=(HardwareDisplayer&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    CFW_LOG_TRACE("HardwareDisplayer@{} move assigned from @{}, ID: {} -> {}",
                  reinterpret_cast<std::uintptr_t>(this),
                  reinterpret_cast<std::uintptr_t>(&other),
                  displaySurfaceID.load(std::memory_order_acquire),
                  other.displaySurfaceID.load(std::memory_order_acquire));
    if (auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
        self_id > 0) {
        bool should_destroy_self = false;
        if (auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
            decrementDisplayerRefCount(self_handle)) {
            should_destroy_self = true;
        }
        if (should_destroy_self) {
            CFW_LOG_TRACE("HardwareDisplayer@{} destroying in move assignment, ID: {}",
                          reinterpret_cast<std::uintptr_t>(this),
                          self_id);
            globalDisplayerStorages.deallocate(self_id);
        }
    }
    displaySurfaceID.store(other.displaySurfaceID.load(std::memory_order_acquire), std::memory_order_release);
    other.displaySurfaceID.store(0, std::memory_order_release);
    return *this;
}

HardwareDisplayer& HardwareDisplayer::wait(const HardwareExecutor& executor) {
    // 确保在锁内完成所有操作
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    if (executor.getExecutorID() > 0) {
        if (auto const executor_handle = gExecutorStorage.acquire_read(executor.getExecutorID());
            executor_handle->impl) {
            if (auto const display_handle = globalDisplayerStorages.acquire_write(self_id);
                display_handle->displayManager) {
                display_handle->displayManager->waitExecutor(*executor_handle->impl);
            }
        }
    } else {
        CFW_LOG_WARNING("HardwareDisplayer@{} wait invalid HardwareExecutor.",
                        reinterpret_cast<std::uintptr_t>(this));
    }

    return *this;
}

HardwareDisplayer& HardwareDisplayer::operator<<(const HardwareImage& image) {
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    if (auto const handle = globalDisplayerStorages.acquire_read(self_id);
        handle->displayManager && handle->displaySurface) {
        handle->displayManager->displayFrame(handle->displaySurface, image);
    }
    return *this;
}