#include "ResourceCommand.h"

// CopyBufferCommand implementations
CopyBufferCommand::CopyBufferCommand(ResourceManager::BufferHardwareWrap& src, ResourceManager::BufferHardwareWrap& dst)
    : srcBuffer(src), dstBuffer(dst) {
    executorType = ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType CopyBufferCommand::getExecutorType() {
    return CommandRecordVulkan::ExecutorType::Transfer;
}

void CopyBufferCommand::commitCommand(HardwareExecutorVulkan& hardwareExecutor) {
    hardwareExecutor.hardwareContext->resourceManager.copyBuffer(hardwareExecutor.currentRecordQueue->commandBuffer, srcBuffer, dstBuffer);
}

CommandRecordVulkan::RequiredBarriers CopyBufferCommand::getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) {
    CommandRecordVulkan::RequiredBarriers requiredBarriers;

    {
        VkBufferMemoryBarrier2 srcBufferBarrier{};
        srcBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        srcBufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        srcBufferBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        srcBufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        srcBufferBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        srcBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcBufferBarrier.buffer = srcBuffer.bufferHandle;
        srcBufferBarrier.offset = 0;
        srcBufferBarrier.size = VK_WHOLE_SIZE;
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
        dstBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstBufferBarrier.buffer = dstBuffer.bufferHandle;
        dstBufferBarrier.offset = 0;
        dstBufferBarrier.size = VK_WHOLE_SIZE;
        dstBufferBarrier.pNext = nullptr;

        requiredBarriers.bufferBarriers.push_back(dstBufferBarrier);
    }

    return requiredBarriers;
}

// CopyImageCommand implementations
CopyImageCommand::CopyImageCommand(ResourceManager::ImageHardwareWrap& srcImg, ResourceManager::ImageHardwareWrap& dstImg)
    : srcImage(srcImg), dstImage(dstImg) {
    executorType = ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType CopyImageCommand::getExecutorType() {
    return CommandRecordVulkan::ExecutorType::Transfer;
}

void CopyImageCommand::commitCommand(HardwareExecutorVulkan& hardwareExecutor) {
    hardwareExecutor.hardwareContext->resourceManager.copyImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstImage);
}

CommandRecordVulkan::RequiredBarriers CopyImageCommand::getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) {
    CommandRecordVulkan::RequiredBarriers requiredBarriers;

    {
        VkImageMemoryBarrier2 srcImageBarrier{};
        srcImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        srcImageBarrier.pNext = nullptr;
        srcImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        srcImageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        srcImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        srcImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        srcImageBarrier.oldLayout = srcImage.imageLayout;
        srcImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        srcImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        srcImageBarrier.image = srcImage.imageHandle;
        srcImageBarrier.subresourceRange.baseMipLevel = 0;
        srcImageBarrier.subresourceRange.levelCount = 1;
        srcImageBarrier.subresourceRange.baseArrayLayer = 0;
        srcImageBarrier.subresourceRange.layerCount = 1;

        srcImage.imageLayout = srcImageBarrier.newLayout;

        requiredBarriers.imageBarriers.push_back(srcImageBarrier);
    }

    {
        VkImageMemoryBarrier2 dstImageBarrier{};
        dstImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        dstImageBarrier.pNext = nullptr;
        dstImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        dstImageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        dstImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        dstImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        dstImageBarrier.oldLayout = dstImage.imageLayout;
        dstImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        dstImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        dstImageBarrier.image = dstImage.imageHandle;
        dstImageBarrier.subresourceRange.baseMipLevel = 0;
        dstImageBarrier.subresourceRange.levelCount = 1;
        dstImageBarrier.subresourceRange.baseArrayLayer = 0;
        dstImageBarrier.subresourceRange.layerCount = 1;

        dstImage.imageLayout = dstImageBarrier.newLayout;

        requiredBarriers.imageBarriers.push_back(dstImageBarrier);
    }

    return requiredBarriers;
}