#include "CabbageHardware.h"
#include "HardwareWrapperVulkan/DisplayVulkan/DisplayManager.h"
#include "HardwareWrapperVulkan/HardwareVulkan/HardwareExecutorVulkan.h"
#include "HardwareWrapperVulkan/ResourcePool.h"

#include <utility>

HardwareDisplayer::HardwareDisplayer(void *surface)
{
    displayHandle_ = globalDisplayerStorages.allocate_handle();
    auto const handle = globalDisplayerStorages.acquire_write(getDisplayerID());
    handle->displaySurface = surface;
    handle->displayManager = std::make_shared<DisplayManager>();
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer &other)
    : displayHandle_(other.displayHandle_)
{
}

HardwareDisplayer::HardwareDisplayer(HardwareDisplayer &&other) noexcept
    : displayHandle_(std::move(other.displayHandle_))
{
}

HardwareDisplayer::~HardwareDisplayer() = default;

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{
    if (this != &other)
    {
        displayHandle_ = other.displayHandle_;
    }
    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator=(HardwareDisplayer &&other) noexcept
{
    if (this != &other)
    {
        displayHandle_ = std::move(other.displayHandle_);
    }
    return *this;
}

HardwareDisplayer &HardwareDisplayer::wait(const HardwareExecutor &executor)
{
    auto const executorId = executor.getExecutorID();
    auto const selfId = getDisplayerID();
    if (executorId == 0 || selfId == 0)
    {
        return *this;
    }

    auto const executorHandle = gExecutorStorage.acquire_read(executorId);
    if (!executorHandle->impl)
    {
        return *this;
    }

    auto const displayHandle = globalDisplayerStorages.acquire_write(selfId);
    if (displayHandle->displayManager)
    {
        displayHandle->displayManager->waitExecutor(*executorHandle->impl);
    }

    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator<<(const HardwareImage &image)
{
    auto const selfId = getDisplayerID();
    if (selfId == 0)
    {
        return *this;
    }

    auto const handle = globalDisplayerStorages.acquire_read(selfId);
    if (handle->displayManager && handle->displaySurface)
    {
        handle->displayManager->displayFrame(handle->displaySurface, image);
    }

    return *this;
}
