#include"CabbageHardware.h"
#include<Display/DisplayManager.h>

std::unordered_map<void*, std::shared_ptr<DisplayManager>> displayerGlobalPool;
std::mutex displayerMutex;

// 同一个窗口只能显示一个，这里没有必要引用计数

HardwareDisplayer::HardwareDisplayer(void* surface): displaySurface(surface)
{
    if (displaySurface != nullptr)
    {
        std::unique_lock<std::mutex> lock(displayerMutex);
        if (!displayerGlobalPool.count(displaySurface))
        {
            displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
        }
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
    }
}

HardwareDisplayer::~HardwareDisplayer()
{
    std::unique_lock<std::mutex> lock(displayerMutex);

    if (displaySurface != nullptr)
    {
        if (displayerGlobalPool.count(displaySurface))
        {
            // 这里智能指针会自动释放，调用析构函数
            displayerGlobalPool.erase(displaySurface);
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
    if (displaySurface == surface)
    {
        return;
    }

    if (surface != nullptr && !displayerGlobalPool.count(surface))
    {
        displaySurface = surface;
        std::unique_lock<std::mutex> lock(displayerMutex);
        displayerGlobalPool[displaySurface] = std::make_shared<DisplayManager>();
    }
}
