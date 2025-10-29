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
        Transfer,
        Invalid
    };

    CommandRecord() = default;
    virtual ~CommandRecord() = default;

    virtual void commitCommand(HardwareExecutor &hardwareExecutor)
    {
    }

    virtual ExecutorType getExecutorType()
    {
        return ExecutorType::Invalid;
    }

  protected:
    ExecutorType executorType = ExecutorType::Invalid;
};

struct HardwareExecutor
{
    HardwareExecutor(std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext = globalHardwareContext.mainDevice)
        : hardwareContext(hardwareContext)
    {
    }

    ~HardwareExecutor() = default;

    HardwareExecutor &operator<<(CommandRecord *commandRecord)
    {
        if (commandRecord->getExecutorType() != CommandRecord::ExecutorType::Invalid)
        {
            commandList.push_back(commandRecord);
        }
        return *this;
    }

    HardwareExecutor &operator<<(HardwareExecutor &other)
    {
        return other;
    }

    HardwareExecutor &commit(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                             std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                             VkFence fence = VK_NULL_HANDLE);

    static DeviceManager::QueueUtils *pickQueueAndCommit(std::atomic_uint16_t &queueIndex, std::vector<DeviceManager::QueueUtils> &queues, std::function<bool(DeviceManager::QueueUtils *currentRecordQueue)> commitCommand);

    // private:
    //     friend struct CommandRecord;
    DeviceManager::QueueUtils *currentRecordQueue = nullptr;

    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;

    std::vector<CommandRecord *> commandList;
};
