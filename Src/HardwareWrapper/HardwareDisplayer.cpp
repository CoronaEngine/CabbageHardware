#include "CabbageHardware.h"
#include "Display/DisplayManager.h"
#include "HardwareWrapper/InternalAccessor.h"

#include "Hardware/HardwareExecutorVulkan.h"

struct DisplayerHardwareWrap
{
    void *displaySurface = nullptr;
    std::shared_ptr<DisplayManager> displayManager;
    uint64_t refCount = 0;
};

Corona::Kernel::Utils::Storage<DisplayerHardwareWrap> globalDisplayerStorages;

void incrementDisplayerRefCount(uintptr_t id)
{
    if (id != 0)
    {
        globalDisplayerStorages.write(id, [](DisplayerHardwareWrap &displayer) {
            ++displayer.refCount;
        });
    }
}

void decrementDisplayerRefCount(uintptr_t id)
{
    if (id == 0)
    {
        return;
    }

    bool shouldDestroy = false;
    globalDisplayerStorages.write(id, [&](DisplayerHardwareWrap &displayer) {
        if (--displayer.refCount == 0)
        {
            displayer.displayManager.reset();
            displayer.displaySurface = nullptr;
            shouldDestroy = true;
        }
    });

    if (shouldDestroy)
    {
        globalDisplayerStorages.deallocate(id);
    }
}

HardwareDisplayer::HardwareDisplayer(void *surface)
{
    const auto handle = globalDisplayerStorages.allocate([surface](DisplayerHardwareWrap &displayer) {
        displayer.displaySurface = surface;
        displayer.displayManager = std::make_shared<DisplayManager>();
        displayer.refCount = 1;
    });

    displaySurfaceID = std::make_shared<uintptr_t>(handle);
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer &other)
    : displaySurfaceID(other.displaySurfaceID)
{
    incrementDisplayerRefCount(*displaySurfaceID);
}

HardwareDisplayer::~HardwareDisplayer()
{
    if (displaySurfaceID)
    {
        decrementDisplayerRefCount(*displaySurfaceID);
    }
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{
    if (this != &other)
    {
        incrementDisplayerRefCount(*other.displaySurfaceID);
        decrementDisplayerRefCount(*displaySurfaceID);
        displaySurfaceID = other.displaySurfaceID;
    }
    return *this;
}

HardwareDisplayer &HardwareDisplayer::wait(HardwareExecutor &executor)
{
    globalDisplayerStorages.read(*displaySurfaceID,
                                 [&executor](const DisplayerHardwareWrap &displayer) {
                                     if (displayer.displayManager && executor.getExecutorID())
                                     {
                                         if (HardwareExecutorVulkan *executorImpl = getExecutorImpl(*executor.getExecutorID()))
                                         {
                                             displayer.displayManager->waitExecutor(*executorImpl);
                                         }
                                     }
                                 });

    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator<<(HardwareImage &image)
{
    globalDisplayerStorages.read(*displaySurfaceID,
                                 [&image](const DisplayerHardwareWrap &displayer) {
                                     if (displayer.displayManager && displayer.displaySurface)
                                     {
                                         displayer.displayManager->displayFrame(displayer.displaySurface, image);
                                     }
                                 });

    return *this;
}