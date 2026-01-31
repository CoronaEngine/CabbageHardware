#include "HardwareExecutorVulkan.h"

#include "corona/kernel/core/i_logger.h"
#include <algorithm>

// ========== 析构函数：等待所有延迟释放的资源完成 ==========
HardwareExecutorVulkan::~HardwareExecutorVulkan()
{
    // ========== Step 1: 收集所有需要等待的 semaphore 和对应的 timeline 值 ==========
    std::vector<VkSemaphore> semaphoresToWait;
    std::vector<uint64_t> valuesToWait;

    // 从 deferredReleaseQueue 中收集所有唯一的 semaphore 及其最大 timeline 值
    std::unordered_map<VkSemaphore, uint64_t> semaphoreMaxValues;

    for (const auto &entry : deferredReleaseQueue)
    {
        auto it = semaphoreMaxValues.find(entry.semaphore);
        if (it == semaphoreMaxValues.end())
        {
            semaphoreMaxValues[entry.semaphore] = entry.timelineValue;
        }
        else
        {
            // 取最大值，确保等待所有工作完成
            it->second = std::max(it->second, entry.timelineValue);
        }
    }

    // 如果有 pendingResources 但还未提交，需要考虑当前队列
    // 注意：正常流程中 pendingResources 应该在 commit 后为空
    if (!pendingResources.empty())
    {
        CFW_LOG_WARNING("HardwareExecutorVulkan destructor called with {} pending resources not committed",
                        pendingResources.size());
        // 这些资源没有被提交，直接释放是安全的（GPU 从未使用）
        pendingResources.clear();
    }

    // ========== Step 2: 构建等待参数 ==========
    for (const auto &[semaphore, maxValue] : semaphoreMaxValues)
    {
        semaphoresToWait.push_back(semaphore);
        valuesToWait.push_back(maxValue);
    }

    // ========== Step 3: 等待所有 semaphore ==========
    if (!semaphoresToWait.empty() && hardwareContext)
    {
        VkSemaphoreWaitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.pNext = nullptr;
        waitInfo.flags = 0; // 等待所有 semaphore
        waitInfo.semaphoreCount = static_cast<uint32_t>(semaphoresToWait.size());
        waitInfo.pSemaphores = semaphoresToWait.data();
        waitInfo.pValues = valuesToWait.data();

        // 设置超时时间（5 秒），避免无限等待
        constexpr uint64_t timeoutNs = 5'000'000'000ULL; // 5 seconds

        VkResult result = vkWaitSemaphores(
            hardwareContext->deviceManager.logicalDevice,
            &waitInfo,
            timeoutNs);

        if (result == VK_TIMEOUT)
        {
            CFW_LOG_ERROR("HardwareExecutorVulkan destructor: timeout waiting for {} semaphores",
                          semaphoresToWait.size());
        }
        else if (result != VK_SUCCESS)
        {
            CFW_LOG_ERROR("HardwareExecutorVulkan destructor: vkWaitSemaphores failed with {}",
                          static_cast<int>(result));
        }
        else
        {
            CFW_LOG_TRACE("HardwareExecutorVulkan destructor: successfully waited for {} semaphores",
                          semaphoresToWait.size());
        }
    }

    // ========== Step 4: 清空队列（此时 GPU 已完成，安全释放） ==========
    deferredReleaseQueue.clear();
}

// ========== 清理已完成的资源（非阻塞） ==========
void HardwareExecutorVulkan::cleanupCompletedResources()
{
    if (deferredReleaseQueue.empty())
    {
        return;
    }

    // ========== Step 1: 收集所有不同的 semaphore ==========
    std::unordered_map<VkSemaphore, uint64_t> semaphoreCompletedValues;

    for (const auto &entry : deferredReleaseQueue)
    {
        // 只记录 semaphore，稍后批量查询
        semaphoreCompletedValues[entry.semaphore] = 0;
    }

    // ========== Step 2: 批量查询每个 semaphore 的当前值 ==========
    for (auto &[semaphore, completedValue] : semaphoreCompletedValues)
    {
        VkResult result = vkGetSemaphoreCounterValue(
            hardwareContext->deviceManager.logicalDevice,
            semaphore,
            &completedValue);

        if (result != VK_SUCCESS)
        {
            CFW_LOG_ERROR("Failed to query timeline semaphore value, VkResult: {}",
                          static_cast<int>(result));
            // 查询失败时设为 0，不释放该 semaphore 对应的资源
            completedValue = 0;
        }
    }

    // ========== Step 3: 分区并移除已完成的资源 ==========
    // 使用 stable_partition 保持未完成资源的相对顺序（FIFO 释放）
    auto partitionPoint = std::stable_partition(
        deferredReleaseQueue.begin(),
        deferredReleaseQueue.end(),
        [&semaphoreCompletedValues](const DeferredRelease &entry) {
            // 返回 true 表示保留（未完成），返回 false 表示移除（已完成）
            auto it = semaphoreCompletedValues.find(entry.semaphore);
            if (it != semaphoreCompletedValues.end())
            {
                // timelineValue <= completedValue 表示 GPU 已完成
                return entry.timelineValue > it->second;
            }
            // 未找到 semaphore 信息，保守处理：保留
            return true;
        });

    // ========== Step 4: 记录释放信息并清理 ==========
    size_t releasedCount = std::distance(partitionPoint, deferredReleaseQueue.end());

    if (releasedCount > 0)
    {
        CFW_LOG_TRACE("Released {} deferred resources, remaining: {}",
                      releasedCount,
                      std::distance(deferredReleaseQueue.begin(), partitionPoint));
    }

    // 实际释放资源（shared_ptr 析构）
    deferredReleaseQueue.erase(partitionPoint, deferredReleaseQueue.end());
}

// ========== 同步等待所有延迟释放的资源完成 ==========
void HardwareExecutorVulkan::waitForAllDeferredResources()
{
    if (deferredReleaseQueue.empty())
    {
        return;
    }

    // 收集所有 semaphore 及最大 timeline 值
    std::unordered_map<VkSemaphore, uint64_t> semaphoreMaxValues;

    for (const auto &entry : deferredReleaseQueue)
    {
        auto it = semaphoreMaxValues.find(entry.semaphore);
        if (it == semaphoreMaxValues.end())
        {
            semaphoreMaxValues[entry.semaphore] = entry.timelineValue;
        }
        else
        {
            it->second = std::max(it->second, entry.timelineValue);
        }
    }

    std::vector<VkSemaphore> semaphores;
    std::vector<uint64_t> values;

    for (const auto &[sem, val] : semaphoreMaxValues)
    {
        semaphores.push_back(sem);
        values.push_back(val);
    }

    if (semaphores.empty())
    {
        return;
    }

    VkSemaphoreWaitInfo waitInfo{};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.pNext = nullptr;
    waitInfo.flags = 0;
    waitInfo.semaphoreCount = static_cast<uint32_t>(semaphores.size());
    waitInfo.pSemaphores = semaphores.data();
    waitInfo.pValues = values.data();

    VkResult result = vkWaitSemaphores(
        hardwareContext->deviceManager.logicalDevice,
        &waitInfo,
        UINT64_MAX);

    if (result != VK_SUCCESS)
    {
        CFW_LOG_ERROR("waitForAllDeferredResources: vkWaitSemaphores failed with {}",
                      static_cast<int>(result));
        return;
    }

    // 等待完成后，清理所有资源
    size_t count = deferredReleaseQueue.size();
    deferredReleaseQueue.clear();

    CFW_LOG_TRACE("waitForAllDeferredResources: released {} resources", count);
}

// ========== 获取延迟释放统计信息 ==========
HardwareExecutorVulkan::DeferredReleaseStats HardwareExecutorVulkan::getDeferredReleaseStats() const
{
    DeferredReleaseStats stats;
    stats.currentPending = deferredReleaseQueue.size();

    if (deferredReleaseQueue.empty())
    {
        return stats;
    }

    // 统计不同的 semaphore 数量
    std::unordered_set<VkSemaphore> uniqueSemaphores;
    uint64_t minTimeline = UINT64_MAX;
    uint64_t maxTimeline = 0;

    for (const auto &entry : deferredReleaseQueue)
    {
        uniqueSemaphores.insert(entry.semaphore);
        minTimeline = std::min(minTimeline, entry.timelineValue);
        maxTimeline = std::max(maxTimeline, entry.timelineValue);
    }

    stats.totalSemaphores = uniqueSemaphores.size();
    stats.oldestTimeline = minTimeline;
    stats.newestTimeline = maxTimeline;

    return stats;
}

DeviceManager::QueueUtils *HardwareExecutorVulkan::pickQueueAndCommit(std::atomic_uint16_t &currentQueueIndex,
                                                                      std::vector<DeviceManager::QueueUtils> &currentQueues,
                                                                      std::function<bool(DeviceManager::QueueUtils *currentRecordQueue)> commitCommand,
                                                                      bool needsCommandBuffer)
{
    DeviceManager::QueueUtils *queue;
    uint16_t queueIndex = 0;

    while (true)
    {
        uint16_t queueIndex = currentQueueIndex.fetch_add(1) % currentQueues.size();
        queue = &currentQueues[queueIndex];

        if (queue->queueMutex->try_lock())
        {
            // if (!needsCommandBuffer)
            //{
            //     if (queue->queueWaitFence != VK_NULL_HANDLE)
            //     {
            //         VkResult status = vkGetFenceStatus(queue->deviceManager->logicalDevice, queue->queueWaitFence);
            //         if (status == VK_SUCCESS)
            //         {
            //             // fence已经是signaled状态
            //             queue->queueWaitFence = VK_NULL_HANDLE;
            //             break;
            //         }
            //         else if (status == VK_NOT_READY)
            //         {
            //             // fence还未signaled
            //             queue->queueMutex->unlock();
            //             std::this_thread::yield();
            //             continue;
            //         }
            //     }
            //     break;
            // }

            uint64_t timelineCounterValue = 0;
            VkResult result = vkGetSemaphoreCounterValue(queue->deviceManager->logicalDevice, queue->timelineSemaphore, &timelineCounterValue);
            if (result == VK_SUCCESS)
            {
                if (timelineCounterValue >= queue->timelineValue->load(std::memory_order_acquire))
                {
                    break;
                }
                else
                {
                    queue->queueMutex->unlock();
                }
            }
            else
            {
                queue->queueMutex->unlock();
                CFW_LOG_ERROR("Failed to get timeline semaphore counter value for queue index {}!", queueIndex);
                return nullptr;
            }
        }

        std::this_thread::yield();
        // std::this_thread::sleep_for(std::chrono::microseconds(10000));
    }

    commitCommand(queue);
    queue->queueMutex->unlock();

    return queue;
}

HardwareExecutorVulkan &HardwareExecutorVulkan::commit()
{
    // ===== 首先清理已完成的资源 =====
    cleanupCompletedResources();

    if (commandList.size() > 0)
    {
        // 保存当前 pendingResources 的引用，在提交成功后使用
        auto localPendingResources = std::move(pendingResources);
        pendingResources.clear();
        pendingResources.reserve(32);

        auto commitToQueue = [&](DeviceManager::QueueUtils *currentRecordQueue) -> bool {
            this->currentRecordQueue = currentRecordQueue;

            vkResetCommandBuffer(currentRecordQueue->commandBuffer, 0);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(currentRecordQueue->commandBuffer, &beginInfo);

            for (size_t i = 0; i < commandList.size(); i++)
            {
                CommandRecordVulkan::RequiredBarriers requiredBarriers = commandList[i]->getRequiredBarriers(*this);

                if (!requiredBarriers.memoryBarriers.empty() || !requiredBarriers.bufferBarriers.empty() || !requiredBarriers.imageBarriers.empty())
                {
                    VkDependencyInfo dependencyInfo{};
                    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                    dependencyInfo.memoryBarrierCount = static_cast<uint32_t>(requiredBarriers.memoryBarriers.size());
                    dependencyInfo.pMemoryBarriers = requiredBarriers.memoryBarriers.data();
                    dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(requiredBarriers.bufferBarriers.size());
                    dependencyInfo.pBufferMemoryBarriers = requiredBarriers.bufferBarriers.data();
                    dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(requiredBarriers.imageBarriers.size());
                    dependencyInfo.pImageMemoryBarriers = requiredBarriers.imageBarriers.data();
                    dependencyInfo.pNext = nullptr;

                    vkCmdPipelineBarrier2(currentRecordQueue->commandBuffer, &dependencyInfo);
                }

                if (commandList[i]->getExecutorType() != CommandRecordVulkan::ExecutorType::Invalid)
                {
                    commandList[i]->commitCommand(*this);
                }
            }

            vkEndCommandBuffer(currentRecordQueue->commandBuffer);

            VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
            commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandBufferSubmitInfo.commandBuffer = currentRecordQueue->commandBuffer;

            VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
            timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            timelineWaitSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
            timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->fetch_add(1);
            timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);

            VkSemaphoreSubmitInfo timelineSignalSemaphoreSubmitInfo{};
            timelineSignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            timelineSignalSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
            timelineSignalSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->load(std::memory_order_acquire);
            timelineSignalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            signalSemaphores.push_back(timelineSignalSemaphoreSubmitInfo);

            VkSubmitInfo2 submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
            submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphores.size());
            submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
            submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphores.size());
            submitInfo.pSignalSemaphoreInfos = signalSemaphores.data();
            submitInfo.commandBufferInfoCount = 1;
            submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

            VkResult result = vkQueueSubmit2(currentRecordQueue->vkQueue, 1, &submitInfo, waitFence);
            if (result != VK_SUCCESS)
            {
                CFW_LOG_ERROR("Failed to submit command buffer! VkResult: {}", coronaHardwareResultStr(result));
                return false;
            }

            // ===== 将待释放资源绑定到此次提交的 timeline 值 =====
            uint64_t signalValue = currentRecordQueue->timelineValue->load(std::memory_order_acquire);
            for (auto &resource : localPendingResources)
            {
                deferredReleaseQueue.push_back({signalValue,
                                                std::move(resource),
                                                currentRecordQueue->timelineSemaphore});
            }
            localPendingResources.clear();

            // 处理在 commitCommand 期间新增的 pendingResources（例如 RasterizerPipeline 的保活资源）
            for (auto &resource : pendingResources)
            {
                deferredReleaseQueue.push_back({signalValue,
                                                std::move(resource),
                                                currentRecordQueue->timelineSemaphore});
            }
            pendingResources.clear();

            return true;
        };

        CommandRecordVulkan::ExecutorType queueType = CommandRecordVulkan::ExecutorType::Transfer;
        for (size_t i = 0; i < commandList.size(); i++)
        {
            if (commandList[i]->getExecutorType() == CommandRecordVulkan::ExecutorType::Graphics)
            {
                queueType = CommandRecordVulkan::ExecutorType::Graphics;
                break;
            }
            else if (commandList[i]->getExecutorType() == CommandRecordVulkan::ExecutorType::Compute)
            {
                queueType = CommandRecordVulkan::ExecutorType::Compute;
            }
        }

        switch (queueType)
        {
        case CommandRecordVulkan::ExecutorType::Graphics:
            pickQueueAndCommit(hardwareContext->deviceManager.currentGraphicsQueueIndex, hardwareContext->deviceManager.graphicsQueues, commitToQueue);
            break;
        case CommandRecordVulkan::ExecutorType::Compute:
            pickQueueAndCommit(hardwareContext->deviceManager.currentComputeQueueIndex, hardwareContext->deviceManager.computeQueues, commitToQueue);
            break;
        case CommandRecordVulkan::ExecutorType::Transfer:
            pickQueueAndCommit(hardwareContext->deviceManager.currentTransferQueueIndex, hardwareContext->deviceManager.transferQueues, commitToQueue);
            break;
        case CommandRecordVulkan::ExecutorType::Invalid:
            CFW_LOG_ERROR("No valid command to commit in HardwareExecutorVulkan!");
            break;
        default:
            CFW_LOG_ERROR("Unknown executor type in HardwareExecutorVulkan!");
            break;
        }

        commandList.clear();
    }

    {
        waitSemaphores.clear();
        signalSemaphores.clear();
        waitFence = VK_NULL_HANDLE;

        if (this->currentRecordQueue != nullptr)
        {
            VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
            timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            timelineWaitSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
            timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->load(std::memory_order_acquire);
            timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);
        }
    }

    return *this;
}

HardwareExecutorVulkan &HardwareExecutorVulkan::commitTest()
{
    if (commandList.size() > 0)
    {
        auto commitToQueue = [&](DeviceManager::QueueUtils *currentRecordQueue) -> bool {
            this->currentRecordQueue = currentRecordQueue;

            vkResetCommandBuffer(currentRecordQueue->commandBuffer, 0);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(currentRecordQueue->commandBuffer, &beginInfo);

            for (size_t i = 0; i < commandList.size(); i++)
            {
                CommandRecordVulkan::RequiredBarriers requiredBarriers = commandList[i]->getRequiredBarriers(*this);

                if (!requiredBarriers.memoryBarriers.empty() || !requiredBarriers.bufferBarriers.empty() || !requiredBarriers.imageBarriers.empty())
                {
                    VkDependencyInfo dependencyInfo{};
                    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                    dependencyInfo.memoryBarrierCount = static_cast<uint32_t>(requiredBarriers.memoryBarriers.size());
                    dependencyInfo.pMemoryBarriers = requiredBarriers.memoryBarriers.data();
                    dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(requiredBarriers.bufferBarriers.size());
                    dependencyInfo.pBufferMemoryBarriers = requiredBarriers.bufferBarriers.data();
                    dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(requiredBarriers.imageBarriers.size());
                    dependencyInfo.pImageMemoryBarriers = requiredBarriers.imageBarriers.data();
                    dependencyInfo.pNext = nullptr;

                    vkCmdPipelineBarrier2(currentRecordQueue->commandBuffer, &dependencyInfo);
                }

                if (commandList[i]->getExecutorType() != CommandRecordVulkan::ExecutorType::Invalid)
                {
                    commandList[i]->commitCommand(*this);
                }
            }

            vkEndCommandBuffer(currentRecordQueue->commandBuffer);

            VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
            commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandBufferSubmitInfo.commandBuffer = currentRecordQueue->commandBuffer;

            VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
            timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            timelineWaitSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
            timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->fetch_add(1);
            timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);

            VkSemaphoreSubmitInfo timelineSignalSemaphoreSubmitInfo{};
            timelineSignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            timelineSignalSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
            timelineSignalSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->load(std::memory_order_acquire);
            timelineSignalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            signalSemaphores.push_back(timelineSignalSemaphoreSubmitInfo);

            VkSubmitInfo2 submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
            submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphores.size());
            submitInfo.pWaitSemaphoreInfos = waitSemaphores.data();
            submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphores.size());
            submitInfo.pSignalSemaphoreInfos = signalSemaphores.data();
            submitInfo.commandBufferInfoCount = 1;
            submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

            VkResult result = vkQueueSubmit2(currentRecordQueue->vkQueue, 1, &submitInfo, waitFence);
            if (result != VK_SUCCESS)
            {
                CFW_LOG_ERROR("Failed to submit command buffer! VkResult: {}", coronaHardwareResultStr(result));
                return false;
            }

            return true;
        };

        CommandRecordVulkan::ExecutorType queueType = CommandRecordVulkan::ExecutorType::Transfer;
        for (size_t i = 0; i < commandList.size(); i++)
        {
            if (commandList[i]->getExecutorType() == CommandRecordVulkan::ExecutorType::Graphics)
            {
                queueType = CommandRecordVulkan::ExecutorType::Graphics;
                break;
            }
            else if (commandList[i]->getExecutorType() == CommandRecordVulkan::ExecutorType::Compute)
            {
                queueType = CommandRecordVulkan::ExecutorType::Compute;
            }
        }

        switch (queueType)
        {
        case CommandRecordVulkan::ExecutorType::Graphics:
            pickQueueAndCommit(hardwareContext->deviceManager.currentGraphicsQueueIndex, hardwareContext->deviceManager.graphicsQueues, commitToQueue);
            break;
        case CommandRecordVulkan::ExecutorType::Compute:
            pickQueueAndCommit(hardwareContext->deviceManager.currentComputeQueueIndex, hardwareContext->deviceManager.computeQueues, commitToQueue);
            break;
        case CommandRecordVulkan::ExecutorType::Transfer:
            pickQueueAndCommit(hardwareContext->deviceManager.currentTransferQueueIndex, hardwareContext->deviceManager.transferQueues, commitToQueue);
            break;
        case CommandRecordVulkan::ExecutorType::Invalid:
            CFW_LOG_ERROR("No valid command to commit in HardwareExecutorVulkan!");
            break;
        default:
            CFW_LOG_ERROR("Unknown executor type in HardwareExecutorVulkan!");
            break;
        }

        commandList.clear();
    }

    {
        waitSemaphores.clear();
        signalSemaphores.clear();
        waitFence = VK_NULL_HANDLE;

        /*VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
        timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        timelineWaitSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
        timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->load(std::memory_order_acquire);
        timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);*/
    }

    return *this;
}