#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "HardwareWrapperVulkan/HardwareContext.h"
#include "CabbageHardware.h"

struct HardwareExecutorVulkan;
struct CommandRecordVulkan;

struct CopyCommandImpl
{
    virtual ~CopyCommandImpl() = default;
    virtual CommandRecordVulkan* getCommandRecord() = 0;
};

// Generic resource holder for keeping objects alive until GPU work is done
struct ResourceHolderCommand : public CopyCommandImpl
{
    std::vector<HardwareBuffer> buffers;
    std::vector<HardwareImage> images;
    // Add other resources if needed (e.g., PushConstants)

    CommandRecordVulkan* getCommandRecord() override { return nullptr; }
};

// ========== 延迟释放条目 ==========
struct DeferredRelease
{
    uint64_t timelineValue;                    // GPU 完成此值后可释放
    std::shared_ptr<CopyCommandImpl> resource; // 持有资源的引用
    VkSemaphore semaphore;                     // 对应的 timeline semaphore
};

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
        // 预分配以减少重分配
        pendingResources.reserve(32);
        deferredReleaseQueue.reserve(128);
    }

    // 兼容旧代码的构造函数，但建议逐步废弃，建议在调用处显式传递
    explicit HardwareExecutorVulkan()
        : hardwareContext(globalHardwareContext.getMainDevice())
    {
        // 预分配以减少重分配
        pendingResources.reserve(32);
        deferredReleaseQueue.reserve(128);
    }

    ~HardwareExecutorVulkan();

    HardwareExecutorVulkan &operator<<(CommandRecordVulkan *commandRecord)
    {
        if (commandRecord && commandRecord->getExecutorType() != CommandRecordVulkan::ExecutorType::Invalid)
        {
            commandList.push_back(commandRecord);
        }
        return *this;
    }

    HardwareExecutorVulkan &operator<<(HardwareExecutorVulkan &other)
    {
        return other;
    }

    HardwareExecutorVulkan &wait(const std::vector<VkSemaphoreSubmitInfo> &waitInfos = {},
                                 const std::vector<VkSemaphoreSubmitInfo> &signalInfos = {})
    {
        // waitFence = fence;
        waitSemaphores.insert(waitSemaphores.end(), waitInfos.begin(), waitInfos.end());
        signalSemaphores.insert(signalSemaphores.end(), signalInfos.begin(), signalInfos.end());
        return *this;
    }

    HardwareExecutorVulkan &wait(HardwareExecutorVulkan &other)
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

    HardwareExecutorVulkan &commit();
    //HardwareExecutorVulkan &commitTest();

    // ========== 延迟释放相关接口 ==========
    void cleanupCompletedResources();
    void waitForAllDeferredResources();

    // 调试：获取统计信息
    struct DeferredReleaseStats
    {
        size_t currentPending = 0;   // 当前等待中的资源数
        size_t totalSemaphores = 0;  // 涉及的 semaphore 数量
        uint64_t oldestTimeline = 0; // 最老的等待 timeline
        uint64_t newestTimeline = 0; // 最新的等待 timeline
    };
    DeferredReleaseStats getDeferredReleaseStats() const;

    // void waitUntilCommitIsComplete();
    // void waitUntilAllCommitAreComplete();
    // void disposeWhenCommitCompletes(std::shared_ptr<Buffer> buffer);
    // void disposeWhenCommitCompletes(std::function<void()> &&deallocator);

    static DeviceManager::QueueUtils *pickQueueAndCommit(std::atomic_uint16_t &queueIndex,
                                                         std::vector<DeviceManager::QueueUtils> &queues,
                                                         std::function<bool(DeviceManager::QueueUtils *currentRecordQueue)> commitCommand);

    DeviceManager::QueueUtils *currentRecordQueue{nullptr};
    std::shared_ptr<HardwareContext::HardwareUtils> hardwareContext;
    std::vector<CommandRecordVulkan *> commandList;
    std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
    std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
    // std::vector<VkFence> prentFences;
    VkFence waitFence{VK_NULL_HANDLE};
    // std::unordered_map<VkFence, DeviceManager::QueueUtils*> fenceToPresent;
    // std::vector<std::vector<std::shared_ptr<Buffer>>> buffer_to_dispose_;

    // ========== 延迟释放成员 ==========
    std::vector<std::shared_ptr<CopyCommandImpl>> pendingResources;
    std::vector<DeferredRelease> deferredReleaseQueue;
};