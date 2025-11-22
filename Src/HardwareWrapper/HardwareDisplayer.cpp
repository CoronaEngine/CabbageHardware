#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/DisplayVulkan/DisplayManager.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/ResourcePool.h"
#include "corona/kernel/utils/storage.h"

void incrementDisplayerRefCount(uintptr_t id) {
    if (id > 0) {
        auto handle = globalDisplayerStorages.acquire_write(id);
        ++handle->refCount;
    }
}

void decrementDisplayerRefCount(uintptr_t id) {
    if (id == 0) {
        return;
    }

    bool shouldDestroy = false;
    auto handle = globalDisplayerStorages.acquire_write(id);
    if (--handle->refCount == 0) {
        handle->displayManager.reset();
        handle->displaySurface = nullptr;
        shouldDestroy = true;
    }

    if (shouldDestroy) {
        globalDisplayerStorages.deallocate(id);
    }
}

HardwareDisplayer::HardwareDisplayer(void* surface) {
    auto id = globalDisplayerStorages.allocate();
    auto handle = globalDisplayerStorages.acquire_write(id);
    handle->displaySurface = surface;
    handle->displayManager = std::make_shared<DisplayManager>();
    handle->refCount = 1;

    displaySurfaceID = std::make_shared<uintptr_t>(id);
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer& other)
    : displaySurfaceID(other.displaySurfaceID) {
    incrementDisplayerRefCount(*displaySurfaceID);
}

HardwareDisplayer::~HardwareDisplayer() {
    if (displaySurfaceID) {
        decrementDisplayerRefCount(*displaySurfaceID);
    }
}

HardwareDisplayer& HardwareDisplayer::operator=(const HardwareDisplayer& other) {
    if (this != &other) {
        incrementDisplayerRefCount(*other.displaySurfaceID);
        decrementDisplayerRefCount(*displaySurfaceID);
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