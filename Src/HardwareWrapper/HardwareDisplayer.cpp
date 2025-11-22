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

HardwareDisplayer::HardwareDisplayer(void* surface) {
    auto id = globalDisplayerStorages.allocate();
    auto const handle = globalDisplayerStorages.acquire_write(id);
    handle->displaySurface = surface;
    handle->displayManager = std::make_shared<DisplayManager>();
    handle->refCount = 1;

    displaySurfaceID = std::make_shared<uintptr_t>(id);
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer& other)
    : displaySurfaceID(other.displaySurfaceID) {
    if (*displaySurfaceID > 0) {
        auto const handle = globalDisplayerStorages.acquire_write(*displaySurfaceID);
        incrementDisplayerRefCount(handle);
    }
}

HardwareDisplayer::~HardwareDisplayer() {
    if (displaySurfaceID && *displaySurfaceID > 0) {
        bool destroy = false;
        if (auto const handle = globalDisplayerStorages.acquire_write(*displaySurfaceID); decrementDisplayerRefCount(handle)) {
            destroy = true;
        }
        if (destroy) {
            globalDisplayerStorages.deallocate(*displaySurfaceID);
        }
    }
}

HardwareDisplayer& HardwareDisplayer::operator=(const HardwareDisplayer& other) {
    if (this != &other) {
        {
            auto const handle = globalDisplayerStorages.acquire_write(*other.displaySurfaceID);
            incrementDisplayerRefCount(handle);
        }
        bool destroy = false;
        if (auto const handle = globalDisplayerStorages.acquire_write(*displaySurfaceID); decrementDisplayerRefCount(handle)) {
            destroy = true;
        }
        if (destroy) {
            globalDisplayerStorages.deallocate(*displaySurfaceID);
        }
        displaySurfaceID = other.displaySurfaceID;
    }
    return *this;
}

HardwareDisplayer& HardwareDisplayer::wait(HardwareExecutor& executor) {
    // 确保在锁内完成所有操作
    if (executor.getExecutorID()) {
        const uintptr_t execID = *executor.getExecutorID();
        auto handle = globalDisplayerStorages.acquire_read(*displaySurfaceID);
        if (handle->displayManager) {
            if (HardwareExecutorVulkan* executorImpl = getExecutorImpl(execID)) {
                handle->displayManager->waitExecutor(*executorImpl);
            }
        }
    }

    return *this;
}

HardwareDisplayer& HardwareDisplayer::operator<<(HardwareImage& image) {
    auto handle = globalDisplayerStorages.acquire_read(*displaySurfaceID);
    if (handle->displayManager && handle->displaySurface) {
        handle->displayManager->displayFrame(handle->displaySurface, image);
    }
    return *this;
}