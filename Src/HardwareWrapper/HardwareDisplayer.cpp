#include"CabbageHardware.h"
#include<Display/DisplayManager.h>

std::unordered_map<void*, std::shared_ptr<DisplayManager>> displayerGlobalPool;
std::unordered_map<void *, uint64_t> displayerRefCount;
uint64_t currentDisplayerID = 0;

std::mutex displayerMutex;

HardwareDisplayer::HardwareDisplayer(void* surface): displaySurface(surface)
{
    if (displaySurface != nullptr)
    {
        std::unique_lock<std::mutex> lock(displayerMutex);
        if (!displayerGlobalPool.count(displaySurface))
        {
            displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
        }

        displayerRefCount[displaySurface]++;
    }
}


HardwareDisplayer::HardwareDisplayer(const HardwareDisplayer &other)
{
    std::unique_lock<std::mutex> lock(displayerMutex);

    displaySurface = other.displaySurface;
    if (displaySurface != nullptr)
    {
        if (!displayerGlobalPool.count(displaySurface))
        {
            displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
        }

        displayerRefCount[displaySurface]++;
    }
}

HardwareDisplayer::~HardwareDisplayer()
{
    std::unique_lock<std::mutex> lock(displayerMutex);

    if (displaySurface != nullptr)
    {
        auto displayer = displayerRefCount.find(displaySurface);
        if (displayer != displayerRefCount.end())
        {
            if (--(displayer->second) == 0)
            {
                displayerRefCount.erase(displayer);
                displayerGlobalPool.erase(displaySurface);
            }
        }
        displaySurface = nullptr;
    }
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{
    if (this == &other)
    {
        return *this;
    }

    std::unique_lock<std::mutex> lock(displayerMutex);

    if (displaySurface != nullptr)
    {
        auto it = displayerRefCount.find(displaySurface);
        if (it != displayerRefCount.end())
        {
            if (--(it->second) == 0)
            {
                displayerRefCount.erase(it);
                displayerGlobalPool.erase(displaySurface);
            }
        }
    }

    displaySurface = other.displaySurface;

    if (displaySurface != nullptr)
    {
        if (!displayerGlobalPool.count(displaySurface))
        {
            displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
        }
        displayerRefCount[displaySurface]++;
    }

    return *this;
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareImage &image)
{
    std::unique_lock<std::mutex> lock(displayerMutex);
    if (displayerGlobalPool.count(displaySurface))
    {
        displayerGlobalPool[displaySurface]->displayFrame(displaySurface,image);
    }
    return *this;
}

void HardwareDisplayer::setSurface(void *surface)
{
    std::unique_lock<std::mutex> lock(displayerMutex);

    if (displaySurface == surface)
    {
        return;
    }

    if (displaySurface != nullptr)
    {
        auto it = displayerRefCount.find(displaySurface);
        if (it != displayerRefCount.end())
        {
            if (--(it->second) == 0)
            {
                displayerRefCount.erase(it);
                displayerGlobalPool.erase(displaySurface);
            }
        }
    }

    displaySurface = surface;

    if (surface != nullptr)
    {
        if (!displayerGlobalPool.count(surface))
        {
            displayerGlobalPool[surface] = std::make_shared<DisplayManager>();
        }
        displayerRefCount[surface]++;
    }
}
