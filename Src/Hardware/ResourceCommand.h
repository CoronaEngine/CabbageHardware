#include "HardwareExecutor.h"

struct CopyBufferCommand : public CommandRecord
{
    ResourceManager::BufferHardwareWrap &srcBuffer;
    ResourceManager::BufferHardwareWrap &dstBuffer;

    CopyBufferCommand(ResourceManager::BufferHardwareWrap &src, ResourceManager::BufferHardwareWrap &dst)
        : srcBuffer(src), dstBuffer(dst)
    {
        executorType = ExecutorType::Transfer;
    }

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyBuffer(hardwareExecutor.currentRecordQueue->commandBuffer, srcBuffer, dstBuffer);
    }

    CommandRecord::RequiredBarriers getRequiredBarriers(HardwareExecutor& hardwareExecutor)
    {
        CommandRecord::RequiredBarriers requiredBarriers;
        {
            VkBufferMemoryBarrier2 srcBufferBarrier{};
            srcBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            srcBufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            srcBufferBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
            srcBufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            srcBufferBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            srcBufferBarrier.srcQueueFamilyIndex = hardwareExecutor.currentRecordQueue->queueFamilyIndex;
            srcBufferBarrier.dstQueueFamilyIndex = hardwareExecutor.currentRecordQueue->queueFamilyIndex;
            srcBufferBarrier.buffer = srcBuffer.bufferHandle;
            srcBufferBarrier.offset = srcBuffer.bufferAllocInfo.offset;
            srcBufferBarrier.size = srcBuffer.bufferAllocInfo.size;
            srcBufferBarrier.pNext = nullptr;

            requiredBarriers.bufferBarriers.push_back(srcBufferBarrier);
        }
        {
            VkBufferMemoryBarrier2 dstBufferBarrier{};

            dstBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            dstBufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            dstBufferBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
            dstBufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            dstBufferBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            dstBufferBarrier.srcQueueFamilyIndex = hardwareExecutor.currentRecordQueue->queueFamilyIndex;
            dstBufferBarrier.dstQueueFamilyIndex = hardwareExecutor.currentRecordQueue->queueFamilyIndex;
            dstBufferBarrier.buffer = dstBuffer.bufferHandle;
            dstBufferBarrier.offset = dstBuffer.bufferAllocInfo.offset;
            dstBufferBarrier.size = dstBuffer.bufferAllocInfo.size;
            dstBufferBarrier.pNext = nullptr;

            requiredBarriers.bufferBarriers.push_back(dstBufferBarrier);
        }
        return requiredBarriers;
    }
};

struct CopyImageCommand : public CommandRecord
{
    ResourceManager::ImageHardwareWrap &srcImage;
    ResourceManager::ImageHardwareWrap &dstImage;

    CopyImageCommand(ResourceManager::ImageHardwareWrap &srcImg, ResourceManager::ImageHardwareWrap &dstImg)
        : srcImage(srcImg), dstImage(dstImg)
    {
        executorType = ExecutorType::Transfer;
    }

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstImage);
    }
};

struct CopyBufferToImageCommand : public CommandRecord
{
    ResourceManager::BufferHardwareWrap &srcBuffer;
    ResourceManager::ImageHardwareWrap &dstImage;

    CopyBufferToImageCommand(ResourceManager::BufferHardwareWrap &srcBuf, ResourceManager::ImageHardwareWrap &dstImg)
        : srcBuffer(srcBuf), dstImage(dstImg)
    {
        executorType = ExecutorType::Transfer;
    }

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyBufferToImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcBuffer, dstImage);
    }
};

struct CopyImageToBufferCommand : public CommandRecord
{
    ResourceManager::ImageHardwareWrap &srcImage;
    ResourceManager::BufferHardwareWrap &dstBuffer;

    CopyImageToBufferCommand(ResourceManager::ImageHardwareWrap &srcImg, ResourceManager::BufferHardwareWrap &dstBuf)
        : srcImage(srcImg), dstBuffer(dstBuf)
    {
        executorType = ExecutorType::Transfer;
    }

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.copyImageToBuffer(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstBuffer);
    }
};

struct BlitImageCommand : public CommandRecord
{
    ResourceManager::ImageHardwareWrap &srcImage;
    ResourceManager::ImageHardwareWrap &dstImage;

    BlitImageCommand(ResourceManager::ImageHardwareWrap &srcImg, ResourceManager::ImageHardwareWrap &dstImg)
        : srcImage(srcImg), dstImage(dstImg)
    {
        executorType = ExecutorType::Graphics;
    }

    ExecutorType getExecutorType() override
    {
        return CommandRecord::ExecutorType::Graphics;
    }

    void commitCommand(HardwareExecutor &hardwareExecutor) override
    {
        hardwareExecutor.hardwareContext->resourceManager.blitImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstImage);
    }
};
