#pragma once

#include <Hardware/DeviceManager.h>
#include <Hardware/GlobalContext.h>

class RasterizerPipeline;
class ComputePipeline;
struct HardwareExecutor;

struct CommandRecord
{
    enum class ExecutorType
    {
        Graphics,
        Compute,
        Transfer
    };

    virtual ~CommandRecord() = default;

    virtual void commitCommand(HardwareExecutor &hardwareExecutor) = 0;
    virtual ExecutorType getExecutorType() = 0;

protected:
    ExecutorType executorType;
};

struct HardwareExecutor
{
    HardwareExecutor(std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext = globalHardwareContext.mainDevice)
        : hardwareContext(hardwareContext)
    {}

    ~HardwareExecutor() = default;


    HardwareExecutor &operator<<(CommandRecord* commandRecord)
    {
        commandList.push_back(commandRecord);
        return *this;
    }

    HardwareExecutor &operator<<(HardwareExecutor& other)
    {
        return other;
    }

    HardwareExecutor &commit(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                             std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                             VkFence fence = VK_NULL_HANDLE);

  //private:
    //friend struct CommandRecord;
    DeviceManager::QueueUtils *currentRecordQueue = nullptr;

    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;

    std::vector<CommandRecord*> commandList;
};
