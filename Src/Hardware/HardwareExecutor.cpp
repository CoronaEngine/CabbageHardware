#include "HardwareExecutor.h"
#include <Hardware/GlobalContext.h>


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

            if (currentRecordQueue->queueMutex->try_lock())
            {
                uint64_t timelineCounterValue = 0;
                vkGetSemaphoreCounterValue(hardwareContext->deviceManager.logicalDevice, currentRecordQueue->timelineSemaphore, &timelineCounterValue);
                if (timelineCounterValue >= currentRecordQueue->timelineValue)
                {
                    break;
                }
                else
                {
                    currentRecordQueue->queueMutex->unlock();
                }
            }

            std::this_thread::yield();
        }

        vkResetCommandBuffer(currentRecordQueue->commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(currentRecordQueue->commandBuffer, &beginInfo);

        // 在这里，我们在每个 CommandRecord 被 commitCommand() 前插入屏障。
        // 如果 CommandRecord 表示它将手动管理屏障 (useManualBarriers == true)，则调用其 recordBarriers()，
        // 否则执行一个保守的自动屏障：当上一个有效记录的 ExecutorType 与当前不同时，
        // 插入一个通用的 MEMORY -> MEMORY 屏障以保证写->读顺序（可覆盖为更精细的资源转换实现）。
        for (size_t i = 0; i < commandList.size(); i++)
        {
            // 找到上一个有效（非 Invalid）命令记录的类型（若有）
            CommandRecord::ExecutorType prevType = CommandRecord::ExecutorType::Invalid;
            for (int j = static_cast<int>(i) - 1; j >= 0; --j)
            {
                auto t = commandList[j]->getExecutorType();
                if (t != CommandRecord::ExecutorType::Invalid)
                {
                    prevType = t;
                    break;
                }
            }

            // 如果当前记录选择手动屏障，则调用其钩子
            if (commandList[i]->useManualBarriers())
            {
                commandList[i]->recordBarriers(*this, currentRecordQueue->commandBuffer);
            }
            else
            {
                // 自动插入保守的全局内存屏障，仅在执行类型发生变化时插入（可以根据需要放宽/细化）
                CommandRecord::ExecutorType curType = commandList[i]->getExecutorType();
                if (prevType != CommandRecord::ExecutorType::Invalid && prevType != curType)
                {
                    VkMemoryBarrier2 memBarrier{};
                    memBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
                    memBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    memBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
                    memBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    memBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;

                    VkDependencyInfo dep{};
                    dep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
                    dep.memoryBarrierCount = 1;
                    dep.pMemoryBarriers = &memBarrier;

                    vkCmdPipelineBarrier2(currentRecordQueue->commandBuffer, &dep);
                }
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
        timelineWaitSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue++;
        timelineWaitSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waitSemaphoreInfos.push_back(timelineWaitSemaphoreSubmitInfo);

        VkSemaphoreSubmitInfo timelineSignalSemaphoreSubmitInfo{};
        timelineSignalSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        timelineSignalSemaphoreSubmitInfo.semaphore = currentRecordQueue->timelineSemaphore;
        timelineSignalSemaphoreSubmitInfo.value = currentRecordQueue->timelineValue;
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
