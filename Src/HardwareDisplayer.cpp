#include"CabbageDisplayer.h"
#include"DisplayManager.h"

std::unordered_map < void *, std::shared_ptr<DisplayManager>> displayerGlobalPool;

std::mutex displayerMutex;


HardwareDisplayer::HardwareDisplayer(void* surface): displaySurface(surface)
{
    if (displaySurface != nullptr && !displayerGlobalPool.count(displaySurface))
    {
        std::unique_lock<std::mutex> lock(displayerMutex);
        displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
    }
}

HardwareDisplayer::~HardwareDisplayer()
{
}

HardwareDisplayer &HardwareDisplayer::operator=(const HardwareDisplayer &other)
{
    displaySurface = other.displaySurface;
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
    if (surface != nullptr && !displayerGlobalPool.count(surface))
    {
        displaySurface = surface;
        std::unique_lock<std::mutex> lock(displayerMutex);
        displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
    }
}
