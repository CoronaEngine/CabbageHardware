#include "HardwareExecutor.h"
#include <Hardware/GlobalContext.h>

DeviceManager::QueueUtils *HardwareExecutor::pickQueueAndCommit(
    std::atomic_uint16_t &currentQueueIndex,
    std::vector<DeviceManager::QueueUtils> &currentQueues,
    std::function<bool(DeviceManager::QueueUtils *currentRecordQueue)> commitCommand)
{
    DeviceManager::QueueUtils *queue;
    uint16_t queueIndex = 0;

    while (true)
    {
        uint16_t queueIndex = currentQueueIndex.fetch_add(1) % currentQueues.size();
        queue = &currentQueues[queueIndex];

        if (queue->queueMutex->try_lock())
        {
            uint64_t timelineCounterValue = 0;
            vkGetSemaphoreCounterValue(queue->deviceManager->logicalDevice, queue->timelineSemaphore, &timelineCounterValue);
            if (timelineCounterValue >= queue->timelineValue->fetch_add(0))
            {
                break;
            }
            else
            {
                queue->queueMutex->unlock();
            }
        }

        //std::this_thread::yield();
    }


    commitCommand(queue);


    queue->queueMutex->unlock();

    return queue;
}



HardwareExecutor &HardwareExecutor::commit()
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
                CommandRecord::RequiredBarriers requiredBarriers = commandList[i]->getRequiredBarriers(*this);

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

                if (commandList[i]->getExecutorType() != CommandRecord::ExecutorType::Invalid)
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
            timelineSignalSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->fetch_add(0);
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
                throw std::runtime_error("Failed to submit command buffer!");
            }

            return true;
        };


        CommandRecord::ExecutorType queueType = CommandRecord::ExecutorType::Transfer;
        for (size_t i = 0; i < commandList.size(); i++)
        {
            if (commandList[i]->getExecutorType() == CommandRecord::ExecutorType::Graphics)
            {
                queueType = CommandRecord::ExecutorType::Graphics;
                break;
            }
            else if (commandList[i]->getExecutorType() == CommandRecord::ExecutorType::Compute)
            {
                queueType = CommandRecord::ExecutorType::Compute;
            }
        }

        switch (queueType)
        {
        case CommandRecord::ExecutorType::Graphics:
            pickQueueAndCommit(hardwareContext->deviceManager.currentGraphicsQueueIndex, hardwareContext->deviceManager.graphicsQueues, commitToQueue);
            break;
        case CommandRecord::ExecutorType::Compute:
            pickQueueAndCommit(hardwareContext->deviceManager.currentComputeQueueIndex, hardwareContext->deviceManager.computeQueues, commitToQueue);
            break;
        case CommandRecord::ExecutorType::Transfer:
            pickQueueAndCommit(hardwareContext->deviceManager.currentTransferQueueIndex, hardwareContext->deviceManager.transferQueues, commitToQueue);
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
        timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue->fetch_add(0);
        timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waitSemaphores.push_back(timelineWaitSemaphoreSubmitInfo);
    }

    return *this;
}
