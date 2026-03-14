#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/DisplayVulkan/DisplayManager.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

static void incrementDisplayerRefCount(uint32_t id, const Corona::Kernel::Utils::Storage<DisplayerHardwareWrap>::WriteHandle &handle)
{
    ++handle->refCount;
    // CFW_LOG_TRACE("HardwareDisplayer ref++: id={}, count={}", id, handle->refCount);
}

// 返回 displayManager shared_ptr，由调用方在锁外释放（避免 ~DisplayManager 中的 vkDeviceWaitIdle 在 slot lock 内执行）
static std::shared_ptr<DisplayManager> decrementDisplayerRefCount(uint32_t id, const Corona::Kernel::Utils::Storage<DisplayerHardwareWrap>::WriteHandle &handle)
{
    int count = --handle->refCount;
    // CFW_LOG_TRACE("HardwareDisplayer ref--: id={}, count={}", id, count);
    if (count == 0)
    {
        auto mgr = std::move(handle->displayManager);
        handle->displaySurface = nullptr;
        // CFW_LOG_TRACE("HardwareDisplayer destroyed: id={}", id);
        return mgr;
    }

    return nullptr;
}

HardwareDisplayer::HardwareDisplayer(void *surface)
{
    auto id = globalDisplayerStorages.allocate();
    displaySurfaceID.store(id, std::memory_order_release);
    auto const handle = globalDisplayerStorages.acquire_write(id);
    handle->displaySurface = surface;
    handle->displayManager = std::make_shared<DisplayManager>();
    // CFW_LOG_TRACE("HardwareDisplayer created: id={}", id);
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer &other)
{
    std::lock_guard<std::mutex> lock(other.displayerMutex);
    auto const other_id = other.displaySurfaceID.load(std::memory_order_acquire);
    displaySurfaceID.store(other_id, std::memory_order_release);
    if (other_id > 0)
    {
        auto const handle = globalDisplayerStorages.acquire_write(other_id);
        incrementDisplayerRefCount(other_id, handle);
    }
}

HardwareDisplayer::HardwareDisplayer(HardwareDisplayer &&other) noexcept
{
    std::lock_guard<std::mutex> lock(other.displayerMutex);
    auto const other_id = other.displaySurfaceID.load(std::memory_order_acquire);
    displaySurfaceID.store(other_id, std::memory_order_release);
    other.displaySurfaceID.store(0, std::memory_order_release);
}

HardwareDisplayer::~HardwareDisplayer()
{
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    if (self_id > 0)
    {
        std::shared_ptr<DisplayManager> mgr;
        bool destroy = false;
        {
            auto const handle = globalDisplayerStorages.acquire_write(self_id);
            mgr = decrementDisplayerRefCount(self_id, handle);
            destroy = (mgr != nullptr);
        }
        mgr.reset(); // ~DisplayManager (含 vkDeviceWaitIdle) 在 slot lock 外执行
        if (destroy)
        {
            globalDisplayerStorages.deallocate(self_id);
        }
        displaySurfaceID.store(0, std::memory_order_release);
    }
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{
    if (this == &other)
    {
        return *this;
    }
    std::scoped_lock lock(displayerMutex, other.displayerMutex);
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    auto const other_id = other.displaySurfaceID.load(std::memory_order_acquire);

    if (self_id == 0 && other_id == 0)
    {
        return *this;
    }
    if (self_id == other_id)
    {
        return *this;
    }

    if (other_id == 0)
    {
        std::shared_ptr<DisplayManager> mgr;
        bool should_destroy_self = false;
        {
            auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
            mgr = decrementDisplayerRefCount(self_id, self_handle);
            should_destroy_self = (mgr != nullptr);
        }
        mgr.reset();
        if (should_destroy_self)
        {
            globalDisplayerStorages.deallocate(self_id);
        }
        displaySurfaceID.store(0, std::memory_order_release);
        return *this;
    }

    if (self_id == 0)
    {
        displaySurfaceID.store(other_id, std::memory_order_release);
        auto const other_handle = globalDisplayerStorages.acquire_write(other_id);
        incrementDisplayerRefCount(other_id, other_handle);
        return *this;
    }

    std::shared_ptr<DisplayManager> mgr;
    bool should_destroy_self = false;
    if (self_id < other_id)
    {
        auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
        auto const other_handle = globalDisplayerStorages.acquire_write(other_id);
        incrementDisplayerRefCount(other_id, other_handle);
        mgr = decrementDisplayerRefCount(self_id, self_handle);
        should_destroy_self = (mgr != nullptr);
    }
    else
    {
        auto const other_handle = globalDisplayerStorages.acquire_write(other_id);
        auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
        incrementDisplayerRefCount(other_id, other_handle);
        mgr = decrementDisplayerRefCount(self_id, self_handle);
        should_destroy_self = (mgr != nullptr);
    }
    // ↓ slot lock 已释放
    mgr.reset();
    if (should_destroy_self)
    {
        globalDisplayerStorages.deallocate(self_id);
    }
    displaySurfaceID.store(other_id, std::memory_order_release);
    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator=(HardwareDisplayer &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }
    std::scoped_lock lock(displayerMutex, other.displayerMutex);
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    auto const other_id = other.displaySurfaceID.load(std::memory_order_acquire);

    if (self_id > 0)
    {
        std::shared_ptr<DisplayManager> mgr;
        bool should_destroy_self = false;
        {
            auto const self_handle = globalDisplayerStorages.acquire_write(self_id);
            mgr = decrementDisplayerRefCount(self_id, self_handle);
            should_destroy_self = (mgr != nullptr);
        }
        mgr.reset();
        if (should_destroy_self)
        {
            globalDisplayerStorages.deallocate(self_id);
        }
    }
    displaySurfaceID.store(other_id, std::memory_order_release);
    other.displaySurfaceID.store(0, std::memory_order_release);
    return *this;
}

HardwareDisplayer &HardwareDisplayer::wait(const HardwareExecutor &executor)
{
    auto const executor_id = executor.getExecutorID();
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    if (executor_id > 0 && self_id > 0)
    {
        // Phase 1: 在 executor slot lock 内复制 executor 数据
        std::shared_ptr<HardwareExecutorVulkan> executorCopy;
        {
            auto const executor_handle = gExecutorStorage.acquire_read(executor_id);
            if (executor_handle->impl)
            {
                executorCopy = std::make_shared<HardwareExecutorVulkan>(*executor_handle->impl);
            }
        } // executor slot lock 释放

        // Phase 2: 在 displayer slot lock 内赋值（无跨 Storage 嵌套）
        if (executorCopy)
        {
            auto const display_handle = globalDisplayerStorages.acquire_write(self_id);
            if (display_handle->displayManager)
            {
                display_handle->displayManager->waitExecutor(*executorCopy);
            }
        }
    }
    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator<<(const HardwareImage &image)
{
    auto const self_id = displaySurfaceID.load(std::memory_order_acquire);
    if (self_id > 0)
    {
        if (auto const handle = globalDisplayerStorages.acquire_read(self_id); handle->displayManager && handle->displaySurface)
        {
            handle->displayManager->displayFrame(handle->displaySurface, image);
        }
    }
    return *this;
}