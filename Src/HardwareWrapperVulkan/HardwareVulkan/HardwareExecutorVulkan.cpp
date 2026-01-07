#include "HardwareExecutorVulkan.h"

DeviceManager::QueueUtils* HardwareExecutorVulkan::pickQueueAndCommit(std::atomic_uint16_t& currentQueueIndex,
                                                                      std::vector<DeviceManager::QueueUtils>& currentQueues,
                                                                      std::function<bool(DeviceManager::QueueUtils* currentRecordQueue)> commitCommand,
                                                                      bool needsCommandBuffer) {
    DeviceManager::QueueUtils* queue;
    uint16_t queueIndex = 0;
    uint32_t yields = 0;

    while (true) {
        uint16_t queueIndex = currentQueueIndex.fetch_add(1) % currentQueues.size();
        queue = &currentQueues[queueIndex];

        if (queue->queueMutex->try_lock()) {
            if (!needsCommandBuffer) {
                break;
            }

            uint64_t timelineCounterValue = 0;
            VkResult result = vkGetSemaphoreCounterValue(queue->deviceManager->logicalDevice, queue->timelineSemaphore, &timelineCounterValue);
            if (result == VK_SUCCESS) {
                if (timelineCounterValue >= queue->timelineValue->load(std::memory_order_acquire)) {
                    break;
                } else {
                    queue->queueMutex->unlock();
                }
            } else {
                queue->queueMutex->unlock();
                CFW_LOG_ERROR("Failed to get timeline semaphore counter value for queue index {}!", queueIndex);
                return nullptr;
            }
        }

         if (++yields < 8) {
             std::this_thread::yield();
             continue;
         }

        //std::this_thread::sleep_for(std::chrono::microseconds(10000));
    }

    commitCommand(queue);
    queue->queueMutex->unlock();

    return queue;
}

//DeviceManager::QueueUtils* HardwareExecutorVulkan::pickQueueAndCommit(std::atomic_uint16_t& currentQueueIndex,
//                                                                      std::vector<DeviceManager::QueueUtils>& currentQueues,
//                                                                      std::function<bool(DeviceManager::QueueUtils* currentRecordQueue)> commitCommand) {
//    DeviceManager::QueueUtils* queue = nullptr;
//    size_t queueCount = currentQueues.size();
//
//    // 1. 快速路径：尝试遍历所有队列，寻找立即可用的队列 (Fast Path)
//    // 避免不必要的阻塞，如果运气好能直接找到空闲队列
//    for (size_t i = 0; i < queueCount; ++i) {
//        uint16_t index = currentQueueIndex.fetch_add(1) % queueCount;
//        queue = &currentQueues[index];
//
//        if (queue->queueMutex->try_lock()) {
//            uint64_t timelineCounterValue = 0;
//            VkResult result = vkGetSemaphoreCounterValue(queue->deviceManager->logicalDevice, queue->timelineSemaphore, &timelineCounterValue);
//            if (result == VK_SUCCESS) {
//                // 检查 GPU 是否完成且当前不在 Present 状态
//                if (timelineCounterValue >= queue->timelineValue->load(std::memory_order_acquire) &&
//                    queue->isPresent->load(std::memory_order_acquire) == false) {
//                    goto COMMIT_AND_RETURN;
//                }
//            }
//            queue->queueMutex->unlock();
//        }
//    }
//
//    // 2. 慢速路径：所有队列都忙，必须阻塞等待 (Slow Path)
//    // 既然都忙，就选一个队列（这里选下一个）进行阻塞等待，直到它可用。
//    {
//        uint16_t index = currentQueueIndex.fetch_add(1) % queueCount;
//        queue = &currentQueues[index];
//
//        // 阻塞获取锁，不再 try_lock
//        queue->queueMutex->lock();
//
//        // 等待 isPresent 标志清除
//        // 如果该队列正在被显示引擎使用，我们必须等待。
//        // 这里使用 yield 循环，因为 Present 通常很快。
//        while (queue->isPresent->load(std::memory_order_acquire)) {
//            queue->queueMutex->unlock();
//            std::this_thread::yield();
//            queue->queueMutex->lock();
//        }
//
//        // 检查 GPU 进度
//        uint64_t targetValue = queue->timelineValue->load(std::memory_order_acquire);
//        uint64_t currentValue = 0;
//        VkResult result = vkGetSemaphoreCounterValue(queue->deviceManager->logicalDevice, queue->timelineSemaphore, &currentValue);
//
//        if (result == VK_SUCCESS) {
//            if (currentValue < targetValue) {
//                // GPU 还没跑完，使用 vkWaitSemaphores 高效阻塞等待
//                // 这会让操作系统挂起当前线程，直到 GPU 发出信号，不会占用 CPU 且响应最快
//                VkSemaphoreWaitInfo waitInfo{};
//                waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
//                waitInfo.semaphoreCount = 1;
//                waitInfo.pSemaphores = &queue->timelineSemaphore;
//                waitInfo.pValues = &targetValue;
//
//                // 等待直到 GPU 追上进度 (使用 UINT64_MAX 表示无限等待)
//                VkResult waitResult = vkWaitSemaphores(queue->deviceManager->logicalDevice, &waitInfo, UINT64_MAX);
//                if (waitResult != VK_SUCCESS) {
//                    queue->queueMutex->unlock();
//                    CFW_LOG_ERROR("Failed to wait for timeline semaphore! VkResult: {}", coronaHardwareResultStr(waitResult));
//                    return nullptr;
//                }
//            }
//        } else {
//            queue->queueMutex->unlock();
//            CFW_LOG_ERROR("Failed to get timeline semaphore counter value for queue index {}!", index);
//            return nullptr;
//        }
//    }
//
//COMMIT_AND_RETURN:
//    commitCommand(queue);
//    queue->queueMutex->unlock();
//
//    return queue;
//}

HardwareExecutorVulkan& HardwareExecutorVulkan::commit() {
    if (commandList.size() > 0) {
        auto commitToQueue = [&](DeviceManager::QueueUtils* currentRecordQueue) -> bool {
            this->currentRecordQueue = currentRecordQueue;

            vkResetCommandBuffer(currentRecordQueue->commandBuffer, 0);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(currentRecordQueue->commandBuffer, &beginInfo);

            for (size_t i = 0; i < commandList.size(); i++) {
                CommandRecordVulkan::RequiredBarriers requiredBarriers = commandList[i]->getRequiredBarriers(*this);

                if (!requiredBarriers.memoryBarriers.empty() || !requiredBarriers.bufferBarriers.empty() || !requiredBarriers.imageBarriers.empty()) {
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

                if (commandList[i]->getExecutorType() != CommandRecordVulkan::ExecutorType::Invalid) {
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
            if (result != VK_SUCCESS) {
                CFW_LOG_ERROR("Failed to submit command buffer! VkResult: {}", coronaHardwareResultStr(result));
                return false;
            }

            return true;
        };

        CommandRecordVulkan::ExecutorType queueType = CommandRecordVulkan::ExecutorType::Transfer;
        for (size_t i = 0; i < commandList.size(); i++) {
            if (commandList[i]->getExecutorType() == CommandRecordVulkan::ExecutorType::Graphics) {
                queueType = CommandRecordVulkan::ExecutorType::Graphics;
                break;
            } else if (commandList[i]->getExecutorType() == CommandRecordVulkan::ExecutorType::Compute) {
                queueType = CommandRecordVulkan::ExecutorType::Compute;
            }
        }

        switch (queueType) {
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

        VkSemaphoreSubmitInfo timelineWaitSemaphoreSubmitInfo{};
        timelineWaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        timelineWaitSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
        timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->load(std::memory_order_acquire);
        timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);
    }

    return *this;
}
