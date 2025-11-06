#include"CabbageHardware.h"
#include<Display/DisplayManager.h>

Corona::Kernel::Utils::Storage<ResourceManager::DisplayHardwareWrap> globalDisplayStorages;

HardwareDisplayer::HardwareDisplayer(void* surface)
{
    auto thisHandle = static_cast<uintptr_t>(*this->displaySurfacePtr);
    bool write_success = globalDisplayStorages.write(thisHandle, [](ResourceManager::DisplayHardwareWrap &disPlay) {
        disPlay.refCount++;
    });

    if (!write_success)
    {
        auto handle = globalDisplayStorages.allocate([&](ResourceManager::DisplayHardwareWrap &disPlay) {
            disPlay = globalHardwareContext.mainDevice->resourceManager.createDisplay(surface);
            disPlay.refCount = 1;
            disPlay.displayManager = std::make_shared<DisplayManager>();
        });

        this->displaySurfacePtr = std::make_shared<uint64_t>(static_cast<uint64_t>(handle));
    }
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer &other)
{
    this->displaySurfacePtr = other.displaySurfacePtr;

    auto otherHandle = static_cast<uintptr_t>(*other.displaySurfacePtr);
    bool write_success = globalDisplayStorages.write(otherHandle, [](ResourceManager::DisplayHardwareWrap &disPlay) {
        disPlay.refCount++;
    });

    if (!write_success)
    {
        throw std::runtime_error("Failed to write HardwareBuffer!");
    }
}

HardwareDisplayer::~HardwareDisplayer()
{
    auto thisHandle = static_cast<uintptr_t>(*this->displaySurfacePtr);
    bool write_success = globalDisplayStorages.write(thisHandle, [&](ResourceManager::DisplayHardwareWrap &disPlay) {
        disPlay.refCount--;
        if (disPlay.refCount == 0)
        {
            disPlay.displayManager.reset();
            disPlay.displaySurface = nullptr;
            globalDisplayStorages.deallocate(thisHandle);
        }
    });
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{
    auto otherHandle = static_cast<uintptr_t>(*other.displaySurfacePtr);
    auto thisHandle = static_cast<uintptr_t>(*this->displaySurfacePtr);

    bool write_success = globalDisplayStorages.write(otherHandle, [](ResourceManager::DisplayHardwareWrap &disPlay) {
        disPlay.refCount++;
    });

    bool write_success = globalDisplayStorages.write(thisHandle, [&](ResourceManager::DisplayHardwareWrap &disPlay) {
        disPlay.refCount--;
        if (disPlay.refCount == 0)
        {
            disPlay.displayManager.reset();
            disPlay.displaySurface = nullptr;
            globalDisplayStorages.deallocate(thisHandle);
        }
    });

    this->displaySurfacePtr = other.displaySurfacePtr;
    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareImage &image)
{
    auto thisHandle = static_cast<uintptr_t>(*this->displaySurfacePtr);

    bool write_success = globalDisplayStorages.write(thisHandle, [&](ResourceManager::DisplayHardwareWrap &disPlay) {
        disPlay.displayManager->displayFrame(disPlay.displaySurface, image);
    });

    return *this;
}

//void HardwareDisplayer::setSurface(void *surface)
//{
//    if (surface != nullptr && displaySurface != surface)
//    {
//        std::unique_lock<std::mutex> lock(displayerMutex);
//        this->displaySurface = surface;
//        if (displayerGlobalPool.count(surface))
//        {
//
//            displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
//        }
//        else
//        {
//        }
//    }
//}