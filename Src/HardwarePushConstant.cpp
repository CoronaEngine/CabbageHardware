#include"CabbageDisplayer.h"
#include<Hardware/GlobalContext.h>

struct PushConstant
{
    uint8_t *data = nullptr;
    uint64_t size = 0;
    bool isSub = false;
};

std::unordered_map<uint64_t, PushConstant> pushConstantGlobalPool;
std::unordered_map<uint64_t, uint64_t> pushConstantRefCount;
uint64_t currentPushConstantID = 0;

std::mutex pushConstantMutex;


HardwarePushConstant::HardwarePushConstant()
{
    std::unique_lock<std::mutex> lock(pushConstantMutex);

    this->pushConstantID = std::make_shared<uint64_t>(currentPushConstantID++);

    pushConstantRefCount[*pushConstantID] = 1;

    pushConstantGlobalPool[*pushConstantID] = PushConstant();
    pushConstantGlobalPool[*pushConstantID].size = 0;
    pushConstantGlobalPool[*pushConstantID].data = nullptr;
}

HardwarePushConstant::HardwarePushConstant(const HardwarePushConstant &other)
{
    std::unique_lock<std::mutex> lock(pushConstantMutex);

    this->pushConstantID = other.pushConstantID;
    pushConstantRefCount[*other.pushConstantID]++;
}

HardwarePushConstant::~HardwarePushConstant()
{
    std::unique_lock<std::mutex> lock(pushConstantMutex);

    pushConstantRefCount[*pushConstantID]--;
    if (pushConstantRefCount[*pushConstantID] == 0)
    {
        if (pushConstantGlobalPool[*pushConstantID].data != nullptr && !pushConstantGlobalPool[*pushConstantID].isSub)
        {
            free(pushConstantGlobalPool[*pushConstantID].data);
            pushConstantGlobalPool[*pushConstantID].data = nullptr;
        }
        pushConstantGlobalPool.erase(*pushConstantID);
        pushConstantRefCount.erase(*pushConstantID);
        //free(pushConstantID);
    }
}

HardwarePushConstant &HardwarePushConstant::operator=(const HardwarePushConstant &other)
{   
    std::unique_lock<std::mutex> lock(pushConstantMutex);

    if (pushConstantGlobalPool[*pushConstantID].isSub)
    {
        memcpy(pushConstantGlobalPool[*pushConstantID].data, pushConstantGlobalPool[*other.pushConstantID].data, pushConstantGlobalPool[*other.pushConstantID].size);
    }
    else
    {
        pushConstantRefCount[*other.pushConstantID]++;
        pushConstantRefCount[*pushConstantID]--;
        if (pushConstantRefCount[*pushConstantID] == 0)
        {
            if (pushConstantGlobalPool[*pushConstantID].data != nullptr && !pushConstantGlobalPool[*pushConstantID].isSub)
            {
                free(pushConstantGlobalPool[*pushConstantID].data);
                pushConstantGlobalPool[*pushConstantID].data = nullptr;
            }
            pushConstantGlobalPool.erase(*pushConstantID);
            pushConstantRefCount.erase(*pushConstantID);
        }
        *this->pushConstantID = *(other.pushConstantID);
    }

    return *this;
}

HardwarePushConstant::HardwarePushConstant(uint64_t size, uint64_t offset, HardwarePushConstant* whole)
{
    std::unique_lock<std::mutex> lock(pushConstantMutex);

    this->pushConstantID = std::make_shared<uint64_t>(currentPushConstantID++);
    //*this->pushConstantID = currentPushConstantID++;

    pushConstantRefCount[*this->pushConstantID] = 1;

    pushConstantGlobalPool[*this->pushConstantID] = PushConstant();
    pushConstantGlobalPool[*this->pushConstantID].size = size;
    if (whole != nullptr)
    {
        pushConstantGlobalPool[*this->pushConstantID].data = pushConstantGlobalPool[*(whole->pushConstantID)].data + offset;
        pushConstantGlobalPool[*this->pushConstantID].isSub = true;
    }
    else
    {
        pushConstantGlobalPool[*this->pushConstantID].data = (uint8_t *)malloc(size);
    }
}

uint8_t* HardwarePushConstant::getData() const
{
    std::unique_lock<std::mutex> lock(pushConstantMutex);

    return pushConstantGlobalPool.at(*pushConstantID).data;
}

uint64_t HardwarePushConstant::getSize() const
{
    std::unique_lock<std::mutex> lock(pushConstantMutex);

    return pushConstantGlobalPool.at(*pushConstantID).size;
}

void HardwarePushConstant::copyFromRaw(const void* src, uint64_t size)
{
    std::unique_lock<std::mutex> lock(pushConstantMutex);

    pushConstantID = std::make_shared<uint64_t>(currentPushConstantID++);
    //*this->pushConstantID = currentPushConstantID++;

    pushConstantRefCount[*this->pushConstantID] = 1;

    pushConstantGlobalPool[*pushConstantID] = PushConstant();
    pushConstantGlobalPool[*pushConstantID].size = size;
    pushConstantGlobalPool[*pushConstantID].data = (uint8_t *)malloc(size);
    memcpy(pushConstantGlobalPool[*pushConstantID].data, src, size);
}