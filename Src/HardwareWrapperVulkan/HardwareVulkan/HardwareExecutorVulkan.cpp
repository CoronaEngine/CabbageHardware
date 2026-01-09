#include "HardwareExecutorVulkan.h"

// ================= 静态成员初始化 =================
std::mutex HardwareExecutorVulkan::cleanupMutex;
std::condition_variable HardwareExecutorVulkan::cleanupCV;
std::deque<HardwareExecutorVulkan::SubmittedBatch> HardwareExecutorVulkan::globalPendingBatches;
std::thread HardwareExecutorVulkan::cleanupThread;
std::atomic<bool> HardwareExecutorVulkan::cleanupThreadRunning{false};
std::once_flag HardwareExecutorVulkan::cleanupThreadInitFlag;

// ================= 异步清理线程实现 =================

void HardwareExecutorVulkan::initCleanupThread()
{
    std::call_once(cleanupThreadInitFlag, []() {
        cleanupThreadRunning.store(true, std::memory_order_release);
        cleanupThread = std::thread(&HardwareExecutorVulkan::cleanupThreadFunc);
    });
}

void HardwareExecutorVulkan::shutdownCleanupThread()
{
    if (cleanupThreadRunning.load(std::memory_order_acquire))
    {
        cleanupThreadRunning.store(false, std::memory_order_release);
        cleanupCV.notify_all();
        if (cleanupThread.joinable())
        {
            cleanupThread.join();
        }
    }
}

void HardwareExecutorVulkan::cleanupThreadFunc()
{
    while (cleanupThreadRunning.load(std::memory_order_acquire))
    {
        std::vector<SubmittedBatch> completedBatches;
        
        {
            std::unique_lock<std::mutex> lock(cleanupMutex);
            
            // 等待有新的批次或退出信号
            cleanupCV.wait_for(lock, std::chrono::milliseconds(100), []() {
                return !globalPendingBatches.empty() || !cleanupThreadRunning.load(std::memory_order_acquire);
            });
            
            if (!cleanupThreadRunning.load(std::memory_order_acquire) && globalPendingBatches.empty())
            {
                break;
            }
            
            // 检查并移除已完成的批次
            auto it = globalPendingBatches.begin();
            while (it != globalPendingBatches.end())
            {
                uint64_t currentValue = 0;
                VkResult result = vkGetSemaphoreCounterValue(it->device, it->semaphore, &currentValue);
                
                if (result == VK_SUCCESS && currentValue >= it->timelineValue)
                {
                    // GPU 已完成，移到待释放列表
                    completedBatches.push_back(std::move(*it));
                    it = globalPendingBatches.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        
        // 在锁外释放资源，避免长时间持有锁
        completedBatches.clear();
    }
    
    // 线程退出前清理所有剩余批次
    std::lock_guard<std::mutex> lock(cleanupMutex);
    globalPendingBatches.clear();
}

void HardwareExecutorVulkan::addBatchToCleanup(SubmittedBatch batch)
{
    // 确保清理线程已启动
    initCleanupThread();
    
    {
        std::lock_guard<std::mutex> lock(cleanupMutex);
        globalPendingBatches.push_back(std::move(batch));
    }
    cleanupCV.notify_one();
}

void HardwareExecutorVulkan::addPendingCommand(std::shared_ptr<CopyCommandImpl> cmdImpl)
{
    if (cmdImpl)
    {
        pendingCommands.push_back(std::move(cmdImpl));
    }
}

void HardwareExecutorVulkan::scheduleCleanup()
{
    // 如果有 pending commands，在 commit 后会自动添加到清理队列
    // 这个方法可以用于手动触发清理检查
    cleanupCV.notify_one();
}

DeviceManager::QueueUtils* HardwareExecutorVulkan::pickQueueAndCommit(std::atomic_uint16_t &currentQueueIndex,
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
            //if (!needsCommandBuffer)
            //{
            //    if (queue->queueWaitFence != VK_NULL_HANDLE)
            //    {
            //        VkResult status = vkGetFenceStatus(queue->deviceManager->logicalDevice, queue->queueWaitFence);
            //        if (status == VK_SUCCESS)
            //        {
            //            // fence已经是signaled状态
            //            queue->queueWaitFence = VK_NULL_HANDLE;
            //            break;
            //        }
            //        else if (status == VK_NOT_READY)
            //        {
            //            // fence还未signaled
            //            queue->queueMutex->unlock();
            //            std::this_thread::yield();
            //            continue;
            //        }
            //    }
            //    break;
            //}

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
        //std::this_thread::sleep_for(std::chrono::microseconds(10000));
    }

    commitCommand(queue);
    queue->queueMutex->unlock();

    return queue;
}

HardwareExecutorVulkan &HardwareExecutorVulkan::commit()
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

        // 将 pending commands 提交到异步清理队列，等待 GPU 执行完成后释放
        if (!pendingCommands.empty() && currentRecordQueue != nullptr)
        {
            SubmittedBatch batch;
            batch.device = hardwareContext->deviceManager.logicalDevice;
            batch.semaphore = currentRecordQueue->timelineSemaphore;
            batch.timelineValue = currentRecordQueue->timelineValue->load(std::memory_order_acquire);
            batch.commands = std::move(pendingCommands);
            addBatchToCleanup(std::move(batch));
            pendingCommands.clear();
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