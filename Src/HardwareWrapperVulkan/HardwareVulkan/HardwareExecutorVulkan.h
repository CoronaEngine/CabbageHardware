#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include "HardwareWrapperVulkan/HardwareContext.h"

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

    virtual void commitCommand(HardwareExecutorVulkan &executor)
    {
    }

    virtual RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan &executor)
    {
        return RequiredBarriers{};
    }

    virtual ExecutorType getExecutorType()
    {
        return ExecutorType::Invalid;
    }

  protected:
    ExecutorType executorType{ExecutorType::Invalid};
};

struct HardwareExecutorVulkan
{
    explicit HardwareExecutorVulkan(std::shared_ptr<HardwareContext::HardwareUtils> context)
        : hardwareContext(std::move(context))
    {
        if (!hardwareContext)
        {
            throw std::invalid_argument("Hardware context cannot be null");
        }
    }

    // 兼容旧代码的构造函数，但建议逐步废弃，建议在调用处显式传递
    explicit HardwareExecutorVulkan()
        : hardwareContext(globalHardwareContext.getMainDevice())
    {
    }

    ~HardwareExecutorVulkan() = default;

    HardwareExecutorVulkan &operator<<(CommandRecordVulkan *commandRecord)
    {
        if (commandRecord && commandRecord->getExecutorType() != CommandRecordVulkan::ExecutorType::Invalid)
        {
            commandList.push_back(commandRecord);
        }
        return *this;
    }

    HardwareExecutorVulkan& operator<<(HardwareExecutorVulkan &other)
    {
        return other;
    }

    HardwareExecutorVulkan& wait(const std::vector<VkSemaphoreSubmitInfo> &waitInfos = {},
                                 const std::vector<VkSemaphoreSubmitInfo> &signalInfos = {})
    {
        // waitFence = fence;
        waitSemaphores.insert(waitSemaphores.end(), waitInfos.begin(), waitInfos.end());
        signalSemaphores.insert(signalSemaphores.end(), signalInfos.begin(), signalInfos.end());
        return *this;
    }

    HardwareExecutorVulkan& wait(HardwareExecutorVulkan &other)
    {
        if (other)
        {
            VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
            timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            timelineWaitSemaphoreSubmitInfo.semaphore = other.currentRecordQueue->timelineSemaphore;
            timelineWaitSemaphoreSubmitInfo.value = other.currentRecordQueue->timelineValue->load(std::memory_order_acquire);
            timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);
        }
        return *this;
    }

    explicit operator bool() const
    {
        return (!commandList.empty()) && (currentRecordQueue != nullptr);
    }

    HardwareExecutorVulkan& commit();

    static DeviceManager::QueueUtils* pickQueueAndCommit(std::atomic_uint16_t& queueIndex,
                                                         std::vector<DeviceManager::QueueUtils>& queues,
                                                         std::function<bool(DeviceManager::QueueUtils* currentRecordQueue)> commitCommand,
                                                         bool needsCommandBuffer = true);

    DeviceManager::QueueUtils *currentRecordQueue{nullptr};
    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;
    std::vector<CommandRecordVulkan *> commandList;
    std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
    std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
    //std::vector<VkFence> prentFences;
    VkFence waitFence{VK_NULL_HANDLE};
    //std::unordered_map<VkFence, DeviceManager::QueueUtils*> fenceToPresent;
};