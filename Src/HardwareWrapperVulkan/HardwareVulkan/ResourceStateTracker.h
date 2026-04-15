#pragma once

#include <vector>

#include "HardwareWrapperVulkan/HardwareVulkan/ResourceManager.h"

struct VulkanBarrierBatch
{
    std::vector<VkMemoryBarrier2> memoryBarriers;
    std::vector<VkBufferMemoryBarrier2> bufferBarriers;
    std::vector<VkImageMemoryBarrier2> imageBarriers;

    [[nodiscard]] bool empty() const
    {
        return memoryBarriers.empty() && bufferBarriers.empty() && imageBarriers.empty();
    }
};

struct VulkanResourceStateMapping
{
    VkPipelineStageFlags2 stageMask{VK_PIPELINE_STAGE_2_NONE};
    VkAccessFlags2 accessMask{VK_ACCESS_2_NONE};
    VkImageLayout imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
};

[[nodiscard]] VulkanResourceStateMapping convertResourceState(ResourceState state, bool isImage);
[[nodiscard]] ResourceState resourceStateFromImageLayout(VkImageLayout layout);

class ResourceStateTracker
{
  public:
    void requireBufferState(ResourceManager::BufferHardwareWrap &buffer, ResourceState state);
    void requireImageState(ResourceManager::ImageHardwareWrap &image,
                           ResourceState state,
                           TextureSubresourceSet subresources = AllSubresources);
    void requireImageLayout(ResourceManager::ImageHardwareWrap &image,
                            VkImageLayout layout,
                            VkPipelineStageFlags2 dstStageMask,
                            VkAccessFlags2 dstAccessMask,
                            TextureSubresourceSet subresources = AllSubresources);
    void addMemoryBarrier(VkPipelineStageFlags2 srcStage,
                          VkAccessFlags2 srcAccess,
                          VkPipelineStageFlags2 dstStage,
                          VkAccessFlags2 dstAccess);

    [[nodiscard]] const VulkanBarrierBatch &getBarriers() const
    {
        return barriers_;
    }

    VulkanBarrierBatch takeBarriers();

  private:
    VulkanBarrierBatch barriers_{};
};
