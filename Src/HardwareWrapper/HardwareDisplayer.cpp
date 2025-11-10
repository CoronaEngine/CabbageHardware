#include"CabbageHardware.h"
#include<Display/DisplayManager.h>

struct DisplayerHardwareWrap
{
    void *displaySurface = nullptr;
    std::shared_ptr<DisplayManager> displayManager;
    uint64_t refCount = 0;
};

Corona::Kernel::Utils::Storage<DisplayerHardwareWrap> globalDisplayerStorages;

HardwareDisplayer::HardwareDisplayer(void* surface)
{
    if (displaySurfaceID != nullptr)
    {
        globalDisplayerStorages.write(*this->displaySurfaceID, [](DisplayerHardwareWrap &disPlayer) {
            disPlayer.refCount++;
        });
    }
    else
    {
        auto handle = globalDisplayerStorages.allocate([&](DisplayerHardwareWrap &disPlayer) {
            DisplayerHardwareWrap newDisplayer;
            newDisplayer.displaySurface = surface;
            newDisplayer.refCount = 1;
            newDisplayer.displayManager = std::make_shared<DisplayManager>();
            disPlayer = newDisplayer;
        });

        this->displaySurfaceID = std::make_shared<uintptr_t>(handle);
    }
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer &other)
{
    this->displaySurfaceID = other.displaySurfaceID;

    globalDisplayerStorages.write(*other.displaySurfaceID, [](DisplayerHardwareWrap &disPlayer) {
        disPlayer.refCount++;
    });
}

HardwareDisplayer::~HardwareDisplayer()
{
    bool destroySelf = false;
    globalDisplayerStorages.write(*displaySurfaceID, [&](DisplayerHardwareWrap &disPlayer) {
        disPlayer.refCount--;
        if (disPlayer.refCount == 0)
        {
            disPlayer.displayManager.reset();
            disPlayer.displaySurface = nullptr;
            destroySelf = true;
            globalDisplayerStorages.deallocate(*displaySurfaceID);
        }
    });
    if (destroySelf)
    {
        globalDisplayerStorages.deallocate(*displaySurfaceID);
    }
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{

    globalDisplayerStorages.write(*other.displaySurfaceID, [](DisplayerHardwareWrap &disPlayer) {
        disPlayer.refCount++;
    });
    globalDisplayerStorages.write(*displaySurfaceID, [&](DisplayerHardwareWrap &disPlayer) {
        disPlayer.refCount--;
        if (disPlayer.refCount == 0)
        {
            disPlayer.displayManager.reset();
            disPlayer.displaySurface = nullptr;
            globalDisplayerStorages.deallocate(*displaySurfaceID);
        }
    });
    this->displaySurfaceID = other.displaySurfaceID;
    return *this;
}


HardwareDisplayer &HardwareDisplayer::wait(HardwareExecutor &executor)
{
    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator<<(HardwareImage &image)
{
    globalDisplayerStorages.read(*displaySurfaceID, [&](const DisplayerHardwareWrap &disPlayer) {
        disPlayer.displayManager->displayFrame(disPlayer.displaySurface, image);
    });

    return *this;
}