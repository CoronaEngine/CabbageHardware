#include"CabbageHardware.h"
#include<Display/DisplayManager.h>

std::unordered_map<void *, std::shared_ptr<DisplayManager>> displayerGlobalPool;
std::unordered_map<void *, uint64_t> displayerRefCount;
std::mutex displayerMutex;

HardwareDisplayer::HardwareDisplayer(void* surface): displaySurface(surface)
{
    if (displaySurface != nullptr)
    {
        std::unique_lock<std::mutex> lock(displayerMutex);

        if (!displayerGlobalPool.count(displaySurface))
        {
            displayerRefCount[this->displaySurface] = 1;
            displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
        }
        else
        {
            displayerRefCount[this->displaySurface]++;
        }
    }
}

HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer &other)
{
    std::unique_lock<std::mutex> lock(displayerMutex);

    displaySurface = other.displaySurface;

    if (displaySurface != nullptr)
    {
        if (displayerGlobalPool.count(displaySurface))
        {
            displayerRefCount[displaySurface]++;
        }
    }
}

HardwareDisplayer::~HardwareDisplayer()
{
    std::unique_lock<std::mutex> lock(displayerMutex);

    if (displaySurface != nullptr)
    {
        if (displayerGlobalPool.count(displaySurface))
        {
            displayerRefCount[displaySurface]--;
            if (displayerRefCount[displaySurface] == 0)
            {
                displayerGlobalPool.erase(displaySurface);
                displayerRefCount.erase(displaySurface);
            }
        }
        displaySurface = nullptr;
    }
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{
    std::unique_lock<std::mutex> lock(displayerMutex);

    if (displayerGlobalPool.count(other.displaySurface))
    {
        displayerRefCount[other.displaySurface]++;
    }
    if (displayerGlobalPool.count(displaySurface))
    {
        displayerRefCount[displaySurface]--;
        if (displayerRefCount[displaySurface] == 0)
        {
            displayerGlobalPool.erase(displaySurface);
            displayerRefCount.erase(displaySurface);
        }
    }
    displaySurface = other.displaySurface;
    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareImage &image)
{
    std::unique_lock<std::mutex> lock(displayerMutex);
    if (displayerGlobalPool.count(displaySurface))
    {
        displayerGlobalPool[displaySurface]->displayFrame(displaySurface, image);
    }
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
