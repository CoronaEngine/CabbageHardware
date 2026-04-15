#include "ResourceStateTracker.h"

#include <algorithm>
#include <utility>

namespace
{
uint32_t resolveCount(uint32_t base, uint32_t requested, uint32_t total)
{
    if (base >= total)
    {
        return 0;
    }
    if (requested == TextureSubresourceSet::AllMipLevels ||
        requested == TextureSubresourceSet::AllArraySlices)
    {
        return total - base;
    }
    return std::min(requested, total - base);
}

VkImageSubresourceRange makeSubresourceRange(const ResourceManager::ImageHardwareWrap &image,
                                             TextureSubresourceSet subresources)
{
    VkImageSubresourceRange range{};
    range.aspectMask = image.aspectMask;
    range.baseMipLevel = subresources.baseMipLevel;
    range.levelCount = resolveCount(subresources.baseMipLevel, subresources.numMipLevels, std::max(1u, image.mipLevels));
    range.baseArrayLayer = subresources.baseArraySlice;
    range.layerCount = resolveCount(subresources.baseArraySlice, subresources.numArraySlices, std::max(1u, image.arrayLayers));

    if (range.levelCount == 0)
    {
        range.baseMipLevel = 0;
        range.levelCount = std::max(1u, image.mipLevels);
    }
    if (range.layerCount == 0)
    {
        range.baseArrayLayer = 0;
        range.layerCount = std::max(1u, image.arrayLayers);
    }
    return range;
}

void setSubresourceStates(ResourceManager::ImageHardwareWrap &image,
                          const VkImageSubresourceRange &range,
                          ResourceState state)
{
    const uint32_t mipLevels = std::max(1u, image.mipLevels);
    const uint32_t arrayLayers = std::max(1u, image.arrayLayers);
    const size_t subresourceCount = static_cast<size_t>(mipLevels) * arrayLayers;

    if (image.subresourceStates.size() != subresourceCount)
    {
        image.subresourceStates.assign(subresourceCount, image.currentState);
    }

    for (uint32_t layer = range.baseArrayLayer; layer < range.baseArrayLayer + range.layerCount; ++layer)
    {
        for (uint32_t mip = range.baseMipLevel; mip < range.baseMipLevel + range.levelCount; ++mip)
        {
            image.subresourceStates[static_cast<size_t>(layer) * mipLevels + mip] = state;
        }
    }
}
} // namespace

VulkanResourceStateMapping convertResourceState(ResourceState state, bool isImage)
{
    if (state == ResourceState::Unknown)
    {
        return {VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED};
    }

    if (hasResourceState(state, ResourceState::CopySource))
    {
        return {VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL};
    }

    if (hasResourceState(state, ResourceState::CopyDest))
    {
        return {VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};
    }

    if (hasResourceState(state, ResourceState::RenderTarget))
    {
        return {VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    }

    if (hasResourceState(state, ResourceState::DepthWrite))
    {
        return {VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    }

    if (hasResourceState(state, ResourceState::DepthRead))
    {
        return {VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT |
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};
    }

    if (hasResourceState(state, ResourceState::Present))
    {
        return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    }

    if (!isImage && hasResourceState(state, ResourceState::VertexBuffer))
    {
        return {VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED};
    }

    if (!isImage && hasResourceState(state, ResourceState::IndexBuffer))
    {
        return {VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED};
    }

    if (!isImage && hasResourceState(state, ResourceState::ConstantBuffer))
    {
        return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};
    }

    if (hasResourceState(state, ResourceState::UnorderedAccess))
    {
        return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};
    }

    if (hasResourceState(state, ResourceState::ShaderResource))
    {
        return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};
    }

    return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
            isImage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_UNDEFINED};
}

ResourceState resourceStateFromImageLayout(VkImageLayout layout)
{
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return ResourceState::CopySource;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return ResourceState::CopyDest;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return ResourceState::RenderTarget;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return ResourceState::DepthWrite;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        return ResourceState::DepthRead;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        return ResourceState::Present;
    case VK_IMAGE_LAYOUT_GENERAL:
        return ResourceState::UnorderedAccess;
    default:
        return ResourceState::Unknown;
    }
}

void ResourceStateTracker::requireBufferState(ResourceManager::BufferHardwareWrap &buffer, ResourceState state)
{
    if (buffer.bufferHandle == VK_NULL_HANDLE || buffer.currentState == state)
    {
        return;
    }

    const VulkanResourceStateMapping before = convertResourceState(buffer.currentState, false);
    const VulkanResourceStateMapping after = convertResourceState(state, false);

    VkBufferMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.srcStageMask = before.stageMask;
    barrier.srcAccessMask = before.accessMask;
    barrier.dstStageMask = after.stageMask;
    barrier.dstAccessMask = after.accessMask;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = buffer.bufferHandle;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;

    barriers_.bufferBarriers.push_back(barrier);
    buffer.currentState = state;
}

void ResourceStateTracker::requireImageState(ResourceManager::ImageHardwareWrap &image,
                                             ResourceState state,
                                             TextureSubresourceSet subresources)
{
    const VulkanResourceStateMapping after = convertResourceState(state, true);
    requireImageLayout(image, after.imageLayout, after.stageMask, after.accessMask, subresources);
    image.currentState = state;
    setSubresourceStates(image, makeSubresourceRange(image, subresources), state);
}

void ResourceStateTracker::requireImageLayout(ResourceManager::ImageHardwareWrap &image,
                                              VkImageLayout layout,
                                              VkPipelineStageFlags2 dstStageMask,
                                              VkAccessFlags2 dstAccessMask,
                                              TextureSubresourceSet subresources)
{
    if (image.imageHandle == VK_NULL_HANDLE || image.imageLayout == layout)
    {
        return;
    }

    const VulkanResourceStateMapping before = convertResourceState(image.currentState, true);
    const VkImageSubresourceRange range = makeSubresourceRange(image, subresources);

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.srcStageMask = before.stageMask;
    barrier.srcAccessMask = before.accessMask;
    barrier.dstStageMask = dstStageMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.oldLayout = image.imageLayout;
    barrier.newLayout = layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image.imageHandle;
    barrier.subresourceRange = range;

    barriers_.imageBarriers.push_back(barrier);
    image.imageLayout = layout;
    image.currentState = resourceStateFromImageLayout(layout);
    setSubresourceStates(image, range, image.currentState);
}

void ResourceStateTracker::addMemoryBarrier(VkPipelineStageFlags2 srcStage,
                                            VkAccessFlags2 srcAccess,
                                            VkPipelineStageFlags2 dstStage,
                                            VkAccessFlags2 dstAccess)
{
    VkMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    barrier.srcStageMask = srcStage;
    barrier.srcAccessMask = srcAccess;
    barrier.dstStageMask = dstStage;
    barrier.dstAccessMask = dstAccess;
    barriers_.memoryBarriers.push_back(barrier);
}

VulkanBarrierBatch ResourceStateTracker::takeBarriers()
{
    VulkanBarrierBatch result = std::move(barriers_);
    barriers_ = {};
    return result;
}
