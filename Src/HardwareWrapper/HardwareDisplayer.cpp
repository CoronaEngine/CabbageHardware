﻿#include"CabbageHardware.h"
#include<Display/DisplayManager.h>

std::unordered_map<void*, std::shared_ptr<DisplayManager>> displayerGlobalPool;
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
            // 清理过程中，确保DisplayManager被正确销毁
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
        displayerGlobalPool[displaySurface]->displayFrame(displaySurface, image);
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
