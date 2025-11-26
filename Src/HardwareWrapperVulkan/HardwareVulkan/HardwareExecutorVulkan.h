#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include "HardwareWrapperVulkan/HardwareContext.h"

struct HardwareExecutorVulkan;

struct CommandRecordVulkan {
    enum class ExecutorType {
        Graphics,
        Compute,
        Transfer,
        Invalid
    };

    struct RequiredBarriers {
        std::vector<VkMemoryBarrier2> memoryBarriers;
        std::vector<VkBufferMemoryBarrier2> bufferBarriers;
        std::vector<VkImageMemoryBarrier2> imageBarriers;
    };

    CommandRecordVulkan() = default;
    virtual ~CommandRecordVulkan() = default;

    virtual void commitCommand(HardwareExecutorVulkan& executor) {}

    virtual RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& executor) {
        return RequiredBarriers{};
    }

    virtual ExecutorType getExecutorType() {
        return ExecutorType::Invalid;
    }

   protected:
    ExecutorType executorType = ExecutorType::Invalid;
};

struct HardwareExecutorVulkan {
    explicit HardwareExecutorVulkan(std::shared_ptr<HardwareContext::HardwareUtils> context)
        : hardwareContext(std::move(context)) {
        if (!hardwareContext) {
            throw std::invalid_argument("Hardware context cannot be null");
        }
    }

    // 兼容旧代码的构造函数，但建议逐步废弃，建议在调用处显式传递
    explicit HardwareExecutorVulkan()
        : hardwareContext(globalHardwareContext.getMainDevice()) {
    }

    HardwareExecutorVulkan() = delete;
    ~HardwareExecutorVulkan() = default;

    HardwareExecutorVulkan& operator<<(CommandRecordVulkan* commandRecord) {
        if (commandRecord && commandRecord->getExecutorType() != CommandRecordVulkan::ExecutorType::Invalid) {
            commandList.push_back(commandRecord);
        }
        return *this;
    }

    HardwareExecutorVulkan& operator<<(HardwareExecutorVulkan& other) {
        return other;
    }

    HardwareExecutorVulkan& wait(const std::vector<VkSemaphoreSubmitInfo>& waitInfos = {},
                                 const std::vector<VkSemaphoreSubmitInfo>& signalInfos = {},
                                 VkFence fence = VK_NULL_HANDLE) {
        waitFence = fence;
        waitSemaphores.insert(waitSemaphores.end(), waitInfos.begin(), waitInfos.end());
        signalSemaphores.insert(signalSemaphores.end(), signalInfos.begin(), signalInfos.end());
        return *this;
    }

    HardwareExecutorVulkan& wait(HardwareExecutorVulkan& other) {
        if (other.currentRecordQueue) {
            VkSemaphoreSubmitInfo timelineWaitInfo{};
            timelineWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            timelineWaitInfo.semaphore = other.currentRecordQueue->timelineSemaphore;
            // 注意：这里 fetch_add(0) 获取当前值，逻辑需确保 other 已经 commit 过了
            timelineWaitInfo.value = other.currentRecordQueue->timelineValue->load();
            timelineWaitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            waitSemaphores.push_back(timelineWaitInfo);
        }
        return *this;
    }

    explicit operator bool() const {
        return (!commandList.empty()) && (currentRecordQueue != nullptr);
    }

    HardwareExecutorVulkan& commit();

    static DeviceManager::QueueUtils* pickQueueAndCommit(std::atomic_uint16_t& queueIndex,
                                                         std::vector<DeviceManager::QueueUtils>& queues,
                                                         std::function<bool(DeviceManager::QueueUtils* currentRecordQueue)> commitCommand);

    DeviceManager::QueueUtils* currentRecordQueue = nullptr;
    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;

    std::vector<CommandRecordVulkan*> commandList;

    std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
    std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
    VkFence waitFence = VK_NULL_HANDLE;
};