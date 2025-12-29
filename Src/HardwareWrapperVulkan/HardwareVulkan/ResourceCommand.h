#pragma once
#include "HardwareExecutorVulkan.h"

struct CopyBufferCommand : public CommandRecordVulkan {
    ResourceManager::BufferHardwareWrap& srcBuffer;
    ResourceManager::BufferHardwareWrap& dstBuffer;

    CopyBufferCommand(ResourceManager::BufferHardwareWrap& src, ResourceManager::BufferHardwareWrap& dst);

    ExecutorType getExecutorType() override;
    void commitCommand(HardwareExecutorVulkan& hardwareExecutor) override;
    CommandRecordVulkan::RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) override;
};

struct CopyImageCommand : public CommandRecordVulkan {
    ResourceManager::ImageHardwareWrap& srcImage;
    ResourceManager::ImageHardwareWrap& dstImage;

    CopyImageCommand(ResourceManager::ImageHardwareWrap& srcImg, ResourceManager::ImageHardwareWrap& dstImg);

    ExecutorType getExecutorType() override;
    void commitCommand(HardwareExecutorVulkan& hardwareExecutor) override;
    CommandRecordVulkan::RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) override;
};

//struct CopyBufferToImageCommand : public CommandRecordVulkan {
//    ResourceManager::BufferHardwareWrap& srcBuffer;
//    ResourceManager::ImageHardwareWrap& dstImage;
//    uint32_t mipLevel;
//
//    CopyBufferToImageCommand(ResourceManager::BufferHardwareWrap& srcBuf,
//                             ResourceManager::ImageHardwareWrap& dstImg,
//                             uint32_t mip = 0)
//        : srcBuffer(srcBuf), dstImage(dstImg), mipLevel(mip) {
//        executorType = ExecutorType::Transfer;
//    }
//
//    ExecutorType getExecutorType() override {
//        return CommandRecordVulkan::ExecutorType::Transfer;
//    }
//
//    void commitCommand(HardwareExecutorVulkan& hardwareExecutor) override {
//        // 使用带 mipLevel 参数的重载版本
//        hardwareExecutor.hardwareContext->resourceManager.copyBufferToImage(
//            hardwareExecutor.currentRecordQueue->commandBuffer,
//            srcBuffer,
//            dstImage,
//            mipLevel);
//    }
//
//    CommandRecordVulkan::RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) override {
//        CommandRecordVulkan::RequiredBarriers requiredBarriers;
//        {
//            VkBufferMemoryBarrier2 srcBufferBarrier{};
//            srcBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
//            srcBufferBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
//            srcBufferBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
//            srcBufferBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
//            srcBufferBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
//            srcBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            srcBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            srcBufferBarrier.buffer = srcBuffer.bufferHandle;
//            srcBufferBarrier.offset = 0;
//            srcBufferBarrier.size = VK_WHOLE_SIZE;
//            srcBufferBarrier.pNext = nullptr;
//
//            requiredBarriers.bufferBarriers.push_back(srcBufferBarrier);
//        }
//        {
//            VkImageMemoryBarrier2 dstImageBarrier{};
//            dstImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
//            dstImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
//            dstImageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
//            dstImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
//            dstImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
//            dstImageBarrier.oldLayout = dstImage.imageLayout;
//            dstImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//            dstImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            dstImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            dstImageBarrier.image = dstImage.imageHandle;
//            dstImageBarrier.subresourceRange.aspectMask = dstImage.aspectMask;
//            dstImageBarrier.subresourceRange.baseMipLevel = mipLevel;  // 使用指定的 mipLevel
//            dstImageBarrier.subresourceRange.levelCount = 1;
//            dstImageBarrier.subresourceRange.baseArrayLayer = 0;
//            dstImageBarrier.subresourceRange.layerCount = 1;
//            dstImageBarrier.pNext = nullptr;
//
//            dstImage.imageLayout = dstImageBarrier.newLayout;
//
//            requiredBarriers.imageBarriers.push_back(dstImageBarrier);
//        }
//        return requiredBarriers;
//    }
//};

struct CopyBufferToImageCommand : public CommandRecordVulkan {
    ResourceManager::BufferHardwareWrap& srcBuffer;
    ResourceManager::ImageHardwareWrap& dstImage;
    uint32_t mipLevel;

    CopyBufferToImageCommand(ResourceManager::BufferHardwareWrap& srcBuf,
                             ResourceManager::ImageHardwareWrap& dstImg,
                             uint32_t mip = 0)
        : srcBuffer(srcBuf), dstImage(dstImg), mipLevel(mip) {
        executorType = ExecutorType::Transfer;
    }

    ExecutorType getExecutorType() override {
        return CommandRecordVulkan::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutorVulkan& hardwareExecutor) override {
        // 使用带 mipLevel 参数的重载版本
        hardwareExecutor.hardwareContext->resourceManager.copyBufferToImage(
            hardwareExecutor.currentRecordQueue->commandBuffer,
            srcBuffer,
            dstImage,
            mipLevel,
            dstImage.arrayLayers);
    }

    CommandRecordVulkan::RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) override {
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
            VkImageMemoryBarrier2 dstImageBarrier{};
            dstImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            dstImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            dstImageBarrier.srcAccessMask = 0;  // 初始转换，没有源访问
            dstImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            dstImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            dstImageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // 从未定义布局开始
            dstImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            dstImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            dstImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            dstImageBarrier.image = dstImage.imageHandle;
            dstImageBarrier.subresourceRange.aspectMask = dstImage.aspectMask;
            dstImageBarrier.subresourceRange.baseMipLevel = mipLevel;
            dstImageBarrier.subresourceRange.levelCount = 1;
            dstImageBarrier.subresourceRange.baseArrayLayer = 0;
            dstImageBarrier.subresourceRange.layerCount = dstImage.arrayLayers;  // 转换所有图层
            dstImageBarrier.pNext = nullptr;

            dstImage.imageLayout = dstImageBarrier.newLayout;

            requiredBarriers.imageBarriers.push_back(dstImageBarrier);
        }
        return requiredBarriers;
    }
};

struct CopyImageToBufferCommand : public CommandRecordVulkan {
    ResourceManager::ImageHardwareWrap& srcImage;
    ResourceManager::BufferHardwareWrap& dstBuffer;

    CopyImageToBufferCommand(ResourceManager::ImageHardwareWrap& srcImg, ResourceManager::BufferHardwareWrap& dstBuf)
        : srcImage(srcImg), dstBuffer(dstBuf) {
        executorType = ExecutorType::Transfer;
    }

    ExecutorType getExecutorType() override {
        return CommandRecordVulkan::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutorVulkan& hardwareExecutor) override {
        hardwareExecutor.hardwareContext->resourceManager.copyImageToBuffer(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstBuffer);
    }

    CommandRecordVulkan::RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) override {
        CommandRecordVulkan::RequiredBarriers requiredBarriers;
        {
            VkImageMemoryBarrier2 srcImageBarrier{};
            srcImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            srcImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            srcImageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
            srcImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            srcImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            srcImageBarrier.oldLayout = srcImage.imageLayout;
            srcImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            srcImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            srcImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            srcImageBarrier.image = srcImage.imageHandle;
            srcImageBarrier.subresourceRange.aspectMask = srcImage.aspectMask;
            srcImageBarrier.subresourceRange.baseMipLevel = 0;
            srcImageBarrier.subresourceRange.levelCount = 1;
            srcImageBarrier.subresourceRange.baseArrayLayer = 0;
            srcImageBarrier.subresourceRange.layerCount = 1;
            srcImageBarrier.pNext = nullptr;

            srcImage.imageLayout = srcImageBarrier.newLayout;

            requiredBarriers.imageBarriers.push_back(srcImageBarrier);
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
};

struct BlitImageCommand : public CommandRecordVulkan {
    ResourceManager::ImageHardwareWrap& srcImage;
    ResourceManager::ImageHardwareWrap& dstImage;

    BlitImageCommand(ResourceManager::ImageHardwareWrap& srcImg, ResourceManager::ImageHardwareWrap& dstImg)
        : srcImage(srcImg), dstImage(dstImg) {
        executorType = ExecutorType::Graphics;
    }

    ExecutorType getExecutorType() override {
        return CommandRecordVulkan::ExecutorType::Graphics;
    }

    void commitCommand(HardwareExecutorVulkan& hardwareExecutor) override {
        hardwareExecutor.hardwareContext->resourceManager.blitImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstImage);
    }

    CommandRecordVulkan::RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) override {
        CommandRecordVulkan::RequiredBarriers requiredBarriers;

        {
            VkImageMemoryBarrier2 srcImageBarrier{};
            srcImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            srcImageBarrier.pNext = nullptr;
            srcImageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            srcImageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
            srcImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
            srcImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            srcImageBarrier.oldLayout = srcImage.imageLayout;
            srcImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            srcImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            srcImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            srcImageBarrier.image = srcImage.imageHandle;
            srcImageBarrier.subresourceRange.aspectMask = srcImage.aspectMask;
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
            dstImageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BLIT_BIT;
            dstImageBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            dstImageBarrier.oldLayout = dstImage.imageLayout;
            dstImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            dstImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            dstImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            dstImageBarrier.image = dstImage.imageHandle;
            dstImageBarrier.subresourceRange.aspectMask = dstImage.aspectMask;
            dstImageBarrier.subresourceRange.baseMipLevel = 0;
            dstImageBarrier.subresourceRange.levelCount = 1;
            dstImageBarrier.subresourceRange.baseArrayLayer = 0;
            dstImageBarrier.subresourceRange.layerCount = 1;

            dstImage.imageLayout = dstImageBarrier.newLayout;

            requiredBarriers.imageBarriers.push_back(dstImageBarrier);
        }
        return requiredBarriers;
    }
};

struct TransitionImageLayoutCommand : public CommandRecordVulkan {
    ResourceManager::ImageHardwareWrap& image;
    VkImageLayout imageLayout;
    VkPipelineStageFlags2 dstStageMask;
    VkAccessFlags2 dstAccessMask;

    TransitionImageLayoutCommand(ResourceManager::ImageHardwareWrap& image, VkImageLayout imageLayout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask)
        : image(image), imageLayout(imageLayout), dstStageMask(dstStageMask), dstAccessMask(dstAccessMask) {
        executorType = ExecutorType::Transfer;
    }

    ExecutorType getExecutorType() override {
        return CommandRecordVulkan::ExecutorType::Transfer;
    }

    void commitCommand(HardwareExecutorVulkan& hardwareExecutor) override {
        hardwareExecutor.hardwareContext->resourceManager.transitionImageLayout(hardwareExecutor.currentRecordQueue->commandBuffer, image, imageLayout, dstStageMask, dstAccessMask);
    }

    CommandRecordVulkan::RequiredBarriers getRequiredBarriers(HardwareExecutorVulkan& hardwareExecutor) override {
        return CommandRecordVulkan::RequiredBarriers{};
    }
};