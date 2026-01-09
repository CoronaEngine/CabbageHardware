#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "HardwareWrapperVulkan/HardwareContext.h"

struct HardwareExecutorVulkan;
struct CopyCommandImpl;  // 前向声明

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
    HardwareExecutorVulkan& commitTest();

    // 延迟销毁相关方法
    void addPendingCommand(std::shared_ptr<CopyCommandImpl> cmdImpl);
    void scheduleCleanup();  // 通知清理线程有新的批次需要检查

    //void waitUntilCommitIsComplete();
    //void waitUntilAllCommitAreComplete();
    //void disposeWhenCommitCompletes(std::shared_ptr<Buffer> buffer);
    //void disposeWhenCommitCompletes(std::function<void()> &&deallocator);

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

    // 延迟销毁队列：存储已提交但 GPU 尚未执行完成的命令
    struct SubmittedBatch
    {
        VkDevice device{VK_NULL_HANDLE};
        VkSemaphore semaphore{VK_NULL_HANDLE};
        uint64_t timelineValue{0};
        std::vector<std::shared_ptr<CopyCommandImpl>> commands;
    };
    std::vector<std::shared_ptr<CopyCommandImpl>> pendingCommands;  // 当前批次待提交的命令

    // 异步清理相关 - 静态成员，所有 Executor 共享
    static void initCleanupThread();
    static void shutdownCleanupThread();
    static void addBatchToCleanup(SubmittedBatch batch);

  private:
    static void cleanupThreadFunc();
    
    static std::mutex cleanupMutex;
    static std::condition_variable cleanupCV;
    static std::deque<SubmittedBatch> globalPendingBatches;
    static std::thread cleanupThread;
    static std::atomic<bool> cleanupThreadRunning;
    static std::once_flag cleanupThreadInitFlag;
};