#include "ResourceCommand.h"

#include <algorithm>
#include <utility>

namespace
{
CommandRecordVulkan::RequiredBarriers makeRequiredBarriers(VulkanBarrierBatch &&batch)
{
    CommandRecordVulkan::RequiredBarriers result;
    result.memoryBarriers = std::move(batch.memoryBarriers);
    result.bufferBarriers = std::move(batch.bufferBarriers);
    result.imageBarriers = std::move(batch.imageBarriers);
    return result;
}
} // namespace

CopyBufferCommand::CopyBufferCommand(ResourceManager::BufferHardwareWrap &src,
                                     ResourceManager::BufferHardwareWrap &dst,
                                     uint64_t srcOffsetValue,
                                     uint64_t dstOffsetValue,
                                     uint64_t sizeValue)
    : srcBuffer(src),
      dstBuffer(dst),
      srcOffset(srcOffsetValue),
      dstOffset(dstOffsetValue),
      size(sizeValue)
{
    executorType = ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType CopyBufferCommand::getExecutorType()
{
    return ExecutorType::Transfer;
}

void CopyBufferCommand::commitCommand(HardwareExecutorVulkan &hardwareExecutor)
{
    hardwareExecutor.hardwareContext->resourceManager.copyBuffer(hardwareExecutor.currentRecordQueue->commandBuffer,
                                                                  srcBuffer,
                                                                  dstBuffer,
                                                                  srcOffset,
                                                                  dstOffset,
                                                                  size);
}

CommandRecordVulkan::RequiredBarriers CopyBufferCommand::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    ResourceStateTracker tracker;
    collectResourceStates(hardwareExecutor, tracker);
    return makeRequiredBarriers(tracker.takeBarriers());
}

void CopyBufferCommand::collectResourceStates(HardwareExecutorVulkan &, ResourceStateTracker &tracker)
{
    tracker.requireBufferState(srcBuffer, ResourceState::CopySource);
    tracker.requireBufferState(dstBuffer, ResourceState::CopyDest);
}

CopyImageCommand::CopyImageCommand(ResourceManager::ImageHardwareWrap &srcImg,
                                   ResourceManager::ImageHardwareWrap &dstImg,
                                   uint32_t srcLayerValue,
                                   uint32_t dstLayerValue,
                                   uint32_t srcMipValue,
                                   uint32_t dstMipValue)
    : srcImage(srcImg),
      dstImage(dstImg),
      srcLayer(srcLayerValue),
      dstLayer(dstLayerValue),
      srcMip(srcMipValue),
      dstMip(dstMipValue)
{
    executorType = ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType CopyImageCommand::getExecutorType()
{
    return ExecutorType::Transfer;
}

void CopyImageCommand::commitCommand(HardwareExecutorVulkan &hardwareExecutor)
{
    if (srcImage.imageFormat != dstImage.imageFormat ||
        srcLayer >= std::max(1u, srcImage.arrayLayers) ||
        dstLayer >= std::max(1u, dstImage.arrayLayers) ||
        srcMip >= std::max(1u, srcImage.mipLevels) ||
        dstMip >= std::max(1u, dstImage.mipLevels))
    {
        return;
    }

    hardwareExecutor.hardwareContext->resourceManager.copyImage(hardwareExecutor.currentRecordQueue->commandBuffer,
                                                                srcImage,
                                                                dstImage,
                                                                srcLayer,
                                                                dstLayer,
                                                                srcMip,
                                                                dstMip);

    if (!hardwareExecutor.automaticBarriersEnabled())
    {
        if ((srcImage.imageUsage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)) != 0 &&
            srcImage.imageLayout != VK_IMAGE_LAYOUT_GENERAL)
        {
            hardwareExecutor.hardwareContext->resourceManager.transitionImageLayout(
                hardwareExecutor.currentRecordQueue->commandBuffer,
                srcImage,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
        }

        if ((dstImage.imageUsage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)) != 0 &&
            dstImage.imageLayout != VK_IMAGE_LAYOUT_GENERAL)
        {
            hardwareExecutor.hardwareContext->resourceManager.transitionImageLayout(
                hardwareExecutor.currentRecordQueue->commandBuffer,
                dstImage,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
        }
    }
}

CommandRecordVulkan::RequiredBarriers CopyImageCommand::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    ResourceStateTracker tracker;
    collectResourceStates(hardwareExecutor, tracker);
    return makeRequiredBarriers(tracker.takeBarriers());
}

void CopyImageCommand::collectResourceStates(HardwareExecutorVulkan &, ResourceStateTracker &tracker)
{
    if (srcImage.imageFormat != dstImage.imageFormat ||
        srcLayer >= std::max(1u, srcImage.arrayLayers) ||
        dstLayer >= std::max(1u, dstImage.arrayLayers) ||
        srcMip >= std::max(1u, srcImage.mipLevels) ||
        dstMip >= std::max(1u, dstImage.mipLevels))
    {
        return;
    }

    tracker.requireImageState(srcImage, ResourceState::CopySource,
                              TextureSubresourceSet{srcMip, 1, srcLayer, 1});
    tracker.requireImageState(dstImage, ResourceState::CopyDest,
                              TextureSubresourceSet{dstMip, 1, dstLayer, 1});
}

CopyBufferToImageCommand::CopyBufferToImageCommand(ResourceManager::BufferHardwareWrap &srcBuf,
                                                   ResourceManager::ImageHardwareWrap &dstImg,
                                                   uint64_t bufferOffsetValue,
                                                   uint32_t imageLayerValue,
                                                   uint32_t mip)
    : srcBuffer(srcBuf),
      dstImage(dstImg),
      bufferOffset(bufferOffsetValue),
      imageLayer(imageLayerValue),
      mipLevel(mip)
{
    executorType = ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType CopyBufferToImageCommand::getExecutorType()
{
    return ExecutorType::Transfer;
}

void CopyBufferToImageCommand::commitCommand(HardwareExecutorVulkan &hardwareExecutor)
{
    hardwareExecutor.hardwareContext->resourceManager.copyBufferToImage(
        hardwareExecutor.currentRecordQueue->commandBuffer,
        srcBuffer,
        dstImage,
        bufferOffset,
        imageLayer,
        mipLevel,
        1);

    if (!hardwareExecutor.automaticBarriersEnabled() &&
        (dstImage.imageUsage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)) != 0 &&
        dstImage.imageLayout != VK_IMAGE_LAYOUT_GENERAL)
    {
        hardwareExecutor.hardwareContext->resourceManager.transitionImageLayout(
            hardwareExecutor.currentRecordQueue->commandBuffer,
            dstImage,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
    }
}

CommandRecordVulkan::RequiredBarriers CopyBufferToImageCommand::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    ResourceStateTracker tracker;
    collectResourceStates(hardwareExecutor, tracker);
    return makeRequiredBarriers(tracker.takeBarriers());
}

void CopyBufferToImageCommand::collectResourceStates(HardwareExecutorVulkan &, ResourceStateTracker &tracker)
{
    tracker.requireBufferState(srcBuffer, ResourceState::CopySource);
    tracker.requireImageState(dstImage, ResourceState::CopyDest,
                              TextureSubresourceSet{mipLevel, 1, imageLayer, 1});
}

CopyImageToBufferCommand::CopyImageToBufferCommand(ResourceManager::ImageHardwareWrap &srcImg,
                                                   ResourceManager::BufferHardwareWrap &dstBuf,
                                                   uint32_t imageLayerValue,
                                                   uint32_t imageMipValue,
                                                   uint64_t bufferOffsetValue)
    : srcImage(srcImg),
      dstBuffer(dstBuf),
      imageLayer(imageLayerValue),
      imageMip(imageMipValue),
      bufferOffset(bufferOffsetValue)
{
    executorType = ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType CopyImageToBufferCommand::getExecutorType()
{
    return ExecutorType::Transfer;
}

void CopyImageToBufferCommand::commitCommand(HardwareExecutorVulkan &hardwareExecutor)
{
    hardwareExecutor.hardwareContext->resourceManager.copyImageToBuffer(hardwareExecutor.currentRecordQueue->commandBuffer,
                                                                        srcImage,
                                                                        dstBuffer,
                                                                        imageLayer,
                                                                        imageMip,
                                                                        bufferOffset,
                                                                        1);

    if (!hardwareExecutor.automaticBarriersEnabled() &&
        (srcImage.imageUsage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)) != 0 &&
        srcImage.imageLayout != VK_IMAGE_LAYOUT_GENERAL)
    {
        hardwareExecutor.hardwareContext->resourceManager.transitionImageLayout(
            hardwareExecutor.currentRecordQueue->commandBuffer,
            srcImage,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
    }
}

CommandRecordVulkan::RequiredBarriers CopyImageToBufferCommand::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    ResourceStateTracker tracker;
    collectResourceStates(hardwareExecutor, tracker);
    return makeRequiredBarriers(tracker.takeBarriers());
}

void CopyImageToBufferCommand::collectResourceStates(HardwareExecutorVulkan &, ResourceStateTracker &tracker)
{
    tracker.requireImageState(srcImage, ResourceState::CopySource,
                              TextureSubresourceSet{imageMip, 1, imageLayer, 1});
    tracker.requireBufferState(dstBuffer, ResourceState::CopyDest);
}

BlitImageCommand::BlitImageCommand(ResourceManager::ImageHardwareWrap &srcImg, ResourceManager::ImageHardwareWrap &dstImg)
    : srcImage(srcImg), dstImage(dstImg)
{
    executorType = ExecutorType::Graphics;
}

CommandRecordVulkan::ExecutorType BlitImageCommand::getExecutorType()
{
    return ExecutorType::Graphics;
}

void BlitImageCommand::commitCommand(HardwareExecutorVulkan &hardwareExecutor)
{
    hardwareExecutor.hardwareContext->resourceManager.blitImage(hardwareExecutor.currentRecordQueue->commandBuffer, srcImage, dstImage);

    if (!hardwareExecutor.automaticBarriersEnabled())
    {
        if ((srcImage.imageUsage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)) != 0 &&
            srcImage.imageLayout != VK_IMAGE_LAYOUT_GENERAL)
        {
            hardwareExecutor.hardwareContext->resourceManager.transitionImageLayout(
                hardwareExecutor.currentRecordQueue->commandBuffer,
                srcImage,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
        }

        if ((dstImage.imageUsage & (VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT)) != 0 &&
            dstImage.imageLayout != VK_IMAGE_LAYOUT_GENERAL)
        {
            hardwareExecutor.hardwareContext->resourceManager.transitionImageLayout(
                hardwareExecutor.currentRecordQueue->commandBuffer,
                dstImage,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
        }
    }
}

CommandRecordVulkan::RequiredBarriers BlitImageCommand::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    ResourceStateTracker tracker;
    collectResourceStates(hardwareExecutor, tracker);
    return makeRequiredBarriers(tracker.takeBarriers());
}

void BlitImageCommand::collectResourceStates(HardwareExecutorVulkan &, ResourceStateTracker &tracker)
{
    tracker.requireImageState(srcImage, ResourceState::CopySource);
    tracker.requireImageState(dstImage, ResourceState::CopyDest);
}

TransitionImageLayoutCommand::TransitionImageLayoutCommand(ResourceManager::ImageHardwareWrap &imageValue,
                                                           VkImageLayout imageLayoutValue,
                                                           VkPipelineStageFlags2 dstStageMaskValue,
                                                           VkAccessFlags2 dstAccessMaskValue)
    : image(imageValue),
      imageLayout(imageLayoutValue),
      dstStageMask(dstStageMaskValue),
      dstAccessMask(dstAccessMaskValue)
{
    executorType = (imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
                    imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
                    imageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                       ? ExecutorType::Graphics
                       : ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType TransitionImageLayoutCommand::getExecutorType()
{
    return executorType;
}

void TransitionImageLayoutCommand::commitCommand(HardwareExecutorVulkan &hardwareExecutor)
{
    if (!hardwareExecutor.automaticBarriersEnabled())
    {
        hardwareExecutor.hardwareContext->resourceManager.transitionImageLayout(
            hardwareExecutor.currentRecordQueue->commandBuffer,
            image,
            imageLayout,
            dstStageMask,
            dstAccessMask);
    }
}

CommandRecordVulkan::RequiredBarriers TransitionImageLayoutCommand::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    ResourceStateTracker tracker;
    collectResourceStates(hardwareExecutor, tracker);
    return makeRequiredBarriers(tracker.takeBarriers());
}

void TransitionImageLayoutCommand::collectResourceStates(HardwareExecutorVulkan &, ResourceStateTracker &tracker)
{
    tracker.requireImageLayout(image, imageLayout, dstStageMask, dstAccessMask);
}

TransitionImageStateCommand::TransitionImageStateCommand(ResourceManager::ImageHardwareWrap &imageValue,
                                                         ResourceState stateValue,
                                                         TextureSubresourceSet subresourcesValue)
    : image(imageValue), state(stateValue), subresources(subresourcesValue)
{
    executorType = (state == ResourceState::RenderTarget ||
                    state == ResourceState::DepthRead ||
                    state == ResourceState::DepthWrite ||
                    state == ResourceState::Present)
                       ? ExecutorType::Graphics
                       : ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType TransitionImageStateCommand::getExecutorType()
{
    return executorType;
}

void TransitionImageStateCommand::commitCommand(HardwareExecutorVulkan &)
{
}

CommandRecordVulkan::RequiredBarriers TransitionImageStateCommand::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    ResourceStateTracker tracker;
    collectResourceStates(hardwareExecutor, tracker);
    return makeRequiredBarriers(tracker.takeBarriers());
}

void TransitionImageStateCommand::collectResourceStates(HardwareExecutorVulkan &, ResourceStateTracker &tracker)
{
    tracker.requireImageState(image, state, subresources);
}

TransitionBufferStateCommand::TransitionBufferStateCommand(ResourceManager::BufferHardwareWrap &bufferValue,
                                                           ResourceState stateValue)
    : buffer(bufferValue), state(stateValue)
{
    executorType = ExecutorType::Transfer;
}

CommandRecordVulkan::ExecutorType TransitionBufferStateCommand::getExecutorType()
{
    return executorType;
}

void TransitionBufferStateCommand::commitCommand(HardwareExecutorVulkan &)
{
}

CommandRecordVulkan::RequiredBarriers TransitionBufferStateCommand::getRequiredBarriers(HardwareExecutorVulkan &hardwareExecutor)
{
    ResourceStateTracker tracker;
    collectResourceStates(hardwareExecutor, tracker);
    return makeRequiredBarriers(tracker.takeBarriers());
}

void TransitionBufferStateCommand::collectResourceStates(HardwareExecutorVulkan &, ResourceStateTracker &tracker)
{
    tracker.requireBufferState(buffer, state);
}
