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

    struct RequiredBarriers
    {
        std::vector<VkMemoryBarrier2> memoryBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;
        std::vector<VkImageMemoryBarrier2> imageBarriers;
    };

    CommandRecord() = default;
    virtual ~CommandRecord() = default;

    virtual void commitCommand(HardwareExecutor &hardwareExecutor)
    {
    }

    virtual RequiredBarriers getRequiredBarriers(HardwareExecutor &hardwareExecutor)
    {
        return RequiredBarriers{};
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

    HardwareExecutor &wait(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                           std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
                           VkFence fence = VK_NULL_HANDLE)
    {
        waitFence = fence;
        for (size_t i = 0; i < waitSemaphoreInfos.size(); i++)
        {
            waitSemaphores.push_back(waitSemaphoreInfos[i]);
        }
        for (size_t i = 0; i < signalSemaphoreInfos.size(); i++)
        {
            signalSemaphores.push_back(signalSemaphoreInfos[i]);
        }
        return *this;
    }

    HardwareExecutor &wait(HardwareExecutor &other)
    {
        VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
        timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        timelineWaitSemaphoreSubmitInfo.semaphore = other.currentRecordQueue->timelineSemaphore;
        timelineWaitSemaphoreSubmitInfo.value = other.currentRecordQueue->timelineValue->load(std::memory_order_seq_cst);
        timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);
        return *this;
    }

    HardwareExecutor &commit();

    static DeviceManager::QueueUtils *pickQueueAndCommit(std::atomic_uint16_t &queueIndex, std::vector<DeviceManager::QueueUtils> &queues, std::function<bool(DeviceManager::QueueUtils *currentRecordQueue)> commitCommand);

    // private:
    //     friend struct CommandRecord;
    DeviceManager::QueueUtils *currentRecordQueue = nullptr;

    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;

    std::vector<CommandRecord *> commandList;

    std::vector<VkSemaphoreSubmitInfo> waitSemaphores = std::vector<VkSemaphoreSubmitInfo>();
    std::vector<VkSemaphoreSubmitInfo> signalSemaphores = std::vector<VkSemaphoreSubmitInfo>();
    VkFence waitFence = VK_NULL_HANDLE;
};
