#include "HardwareExecutor.h"
#include <Hardware/GlobalContext.h>
#include <thread>

HardwareExecutor &HardwareExecutor::commit(std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos, std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos, VkFence fence)
{
    if (commandList.size() > 0)
    {
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

        uint16_t queueIndex = 0;

        while (true)
        {
            switch (queueType)
            {
            case CommandRecord::ExecutorType::Graphics:
                queueIndex = hardwareContext->deviceManager.currentGraphicsQueueIndex.fetch_add(1) % hardwareContext->deviceManager.graphicsQueues.size();
                currentRecordQueue = &hardwareContext->deviceManager.graphicsQueues[queueIndex];
                break;
            case CommandRecord::ExecutorType::Compute:
                queueIndex = hardwareContext->deviceManager.currentComputeQueueIndex.fetch_add(1) % hardwareContext->deviceManager.computeQueues.size();
                currentRecordQueue = &hardwareContext->deviceManager.computeQueues[queueIndex];
                break;
            case CommandRecord::ExecutorType::Transfer:
                queueIndex = hardwareContext->deviceManager.currentTransferQueueIndex.fetch_add(1) % hardwareContext->deviceManager.transferQueues.size();
                currentRecordQueue = &hardwareContext->deviceManager.transferQueues[queueIndex];
                break;
            }

            // Try to acquire exclusive access to this queue. If it's busy, try the next one.
            if (currentRecordQueue->queueMutex->try_lock())
            {
                break;
            }

            std::this_thread::yield();
        }

        // Ensure the previous submission that used this per-queue command buffer has finished
        // by waiting on the per-queue timeline semaphore reaching the last signaled value.
        // This avoids busy-waiting and guarantees the command buffer is no longer in-flight
        // before resetting/re-recording it.
        {
            VkSemaphoreWaitInfo waitInfo{};
            waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            waitInfo.flags = 0;
            VkSemaphore semaphore = currentRecordQueue->timelineSemaphore;
            uint64_t value = currentRecordQueue->timelineValue; // last signaled value
            waitInfo.semaphoreCount = 1;
            waitInfo.pSemaphores = &semaphore;
            waitInfo.pValues = &value;
            vkWaitSemaphores(hardwareContext->deviceManager.logicalDevice, &waitInfo, UINT64_MAX);
        }

        vkResetCommandBuffer(currentRecordQueue->commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(currentRecordQueue->commandBuffer, &beginInfo);

        for (size_t i = 0; i < commandList.size(); i++)
        {
            if (commandList[i]->getExecutorType() != CommandRecord::ExecutorType::Invalid)
            {
                commandList[i]->commitCommand(*this);
            }
        }

        vkEndCommandBuffer(currentRecordQueue->commandBuffer);

        VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
        commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferSubmitInfo.commandBuffer = currentRecordQueue->commandBuffer;

        VkSemaphoreSubmitInfo timelineSignalSemaphoreSubmitInfo{};
        timelineSignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        timelineSignalSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
        // Signal the next timeline value to mark completion of this submission
        timelineSignalSemaphoreSubmitInfo.value = ++currentRecordQueue->timelineValue;
        timelineSignalSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        signalSemaphoreInfos.push_back(timelineSignalSemaphoreSubmitInfo);

        VkSubmitInfo2 submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphoreInfos.size());
        submitInfo.pWaitSemaphoreInfos = waitSemaphoreInfos.data();
        submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphoreInfos.size());
        submitInfo.pSignalSemaphoreInfos = signalSemaphoreInfos.data();
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

        VkResult result = vkQueueSubmit2(currentRecordQueue->vkQueue, 1, &submitInfo, fence);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit command buffer!");
        }

        currentRecordQueue->queueMutex->unlock();

        // Todo: 例如拷贝图片，如果直接拷贝命令提交后，马上clear，会导致图片资源被销毁，拷贝命令未执行完成
        // 这里先简单通过等待队列空闲解决，后续可以通过更精细的资源生命周期管理优化
        vkQueueWaitIdle(currentRecordQueue->vkQueue);
        commandList.clear();
    }

    return *this;
}
