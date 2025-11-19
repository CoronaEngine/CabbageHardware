#pragma once

#include "HardwareWrapperVulkan/HardwareVulkan/GlobalContext.h"

struct HardwareExecutorVulkan;

struct CommandRecordVulkan
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

    CommandRecordVulkan() = default;
    virtual ~CommandRecordVulkan() = default;

    virtual void commitCommand(HardwareExecutorVulkan &HardwareExecutorVulkan)
    {
    }

    virtual RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan &HardwareExecutorVulkan)
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

struct HardwareExecutorVulkan
{
    HardwareExecutorVulkan(std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext = globalHardwareContext.getMainDevice())
        : hardwareContext(hardwareContext)
    {
    }

    ~HardwareExecutorVulkan() = default;

    HardwareExecutorVulkan &operator<<(CommandRecordVulkan *CommandRecordVulkan)
    {
        if (CommandRecordVulkan->getExecutorType() != CommandRecordVulkan::ExecutorType::Invalid)
        {
            commandList.push_back(CommandRecordVulkan);
        }
        return *this;
    }

    HardwareExecutorVulkan &operator<<(HardwareExecutorVulkan &other)
    {
        return other;
    }

    HardwareExecutorVulkan &wait(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos = std::vector<VkSemaphoreSubmitInfo>(),
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

    HardwareExecutorVulkan &wait(HardwareExecutorVulkan &other)
    {
        VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
        timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        timelineWaitSemaphoreSubmitInfo.semaphore = other.currentRecordQueue->timelineSemaphore;
        timelineWaitSemaphoreSubmitInfo.value = other.currentRecordQueue->timelineValue->fetch_add(0);
        timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);
        return *this;
    }

    HardwareExecutorVulkan &commit();

    static DeviceManager::QueueUtils *pickQueueAndCommit(std::atomic_uint16_t &queueIndex, std::vector<DeviceManager::QueueUtils> &queues, std::function<bool(DeviceManager::QueueUtils *currentRecordQueue)> commitCommand);

    // private:
    //     friend struct CommandRecordVulkan;
    DeviceManager::QueueUtils *currentRecordQueue = nullptr;

    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;

    std::vector<CommandRecordVulkan *> commandList;

    std::vector<VkSemaphoreSubmitInfo> waitSemaphores = std::vector<VkSemaphoreSubmitInfo>();
    std::vector<VkSemaphoreSubmitInfo> signalSemaphores = std::vector<VkSemaphoreSubmitInfo>();
    VkFence waitFence = VK_NULL_HANDLE;
};
